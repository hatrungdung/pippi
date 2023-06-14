#cython: language_level=3

import array
from cpython cimport array
from libc.stdlib cimport calloc, free
from libc.string cimport memcpy
import logging
from logging.handlers import SysLogHandler
import warnings
import importlib
import importlib.util
import os
from pathlib import Path
import platform
import struct
import subprocess
import threading

import redis

from pippi import dsp, midi
from pippi.soundbuffer cimport SoundBuffer

ADC_NAME = 'adc'

logger = logging.getLogger('astrid-cyrenderer')
if not logger.handlers:
    if platform.system() == 'Darwin':
        log_path = '/var/run/syslog'
    else:
        log_path = '/dev/log'

    logger.addHandler(SysLogHandler(address=log_path))
    logger.setLevel(logging.DEBUG)
    warnings.simplefilter('always')

_redis = redis.StrictRedis(host='localhost', port=6379, db=0)
bus = _redis.pubsub()

cdef bytes serialize_buffer(SoundBuffer buf, size_t onset, int is_looping, lpmsg_t * msg):
    cdef bytearray strbuf
    cdef size_t audiosize, length, msgsize
    cdef int channels, samplerate
    cdef unsigned char[:] msgview

    msgsize = sizeof(lpmsg_t)

    channels = <int>buf.channels
    samplerate = <int>buf.samplerate
    length = <size_t>len(buf)

    # size of audio data 
    audiosize = length * channels * sizeof(lpfloat_t)

    strbuf = bytearray()

    strbuf += struct.pack('N', audiosize)
    strbuf += struct.pack('N', length)
    strbuf += struct.pack('i', channels)
    strbuf += struct.pack('i', samplerate)
    strbuf += struct.pack('i', is_looping)
    strbuf += struct.pack('N', onset)

    for i in range(length):
        for c in range(channels):
            strbuf += struct.pack('d', buf.frames[i,c])

    msgview = <unsigned char[:msgsize]>(<unsigned char *>msg)
    for i in range(msgsize):
        strbuf.append(msgview[i])

    logger.debug('strbuf loop: %s' % is_looping)
    logger.debug('strbuf bytes: %d' % len(strbuf))

    return bytes(strbuf)

cdef SoundBuffer read_from_adc(double length, double offset=0, int channels=2, int samplerate=48000):
    cdef size_t i
    cdef int c
    cdef double sample
    cdef double * block

    cdef SoundBuffer snd = SoundBuffer(length=length, channels=channels, samplerate=samplerate)
    cdef size_t framelength = len(snd)
    cdef size_t frameoffset = <size_t>(offset * samplerate)

    logger.debug('ADC: framelength %s (before)' % framelength)
    logger.debug('ADC: frameoffset %s (before)' % frameoffset)

    if lpadc_read_block_of_samples(frameoffset, framelength, &block) < 0:
        logger.error('cyrenderer ADC read: failed to read %d frames at offset %d from ADC' % (framelength, frameoffset))
        return snd

    logger.debug('ADC: framelength %s' % framelength)
    logger.debug('ADC: frameoffset %s' % frameoffset)

    for i in range(framelength):
        for c in range(snd.channels):
            sample = block[i * snd.channels + c]
            snd.frames[i,c] = sample

    free(block)

    return snd

cdef class MidiEvent:
    def __cinit__(self,
            double onset,
            double length,
            char type,
            char note,
            char velocity,
            char channel
        ):

        self.event = <lpmidievent_t *>calloc(1, sizeof(lpmidievent_t))
        self.event.onset = onset
        self.event.length = length
        self.event.type = type
        self.event.note = note
        self.event.velocity = velocity
        self.event.channel = channel

    cpdef double schedule_event(MidiEvent self, int qfd):
        midi_triggerq_schedule(qfd, self.event[0])
        return self.event.onset + self.event.length

    def __dealloc__(self):
        if self.event is not NULL:
            free(self.event)

cdef class MidiEventListenerProxy:
    cpdef float cc(self, int cc, int device_id=0):
        cdef int value = lpmidi_getcc(device_id, cc)
        return float(value) / 127

    cpdef int cci(self, int cc, int device_id=0):
        return lpmidi_getcc(device_id, cc)

    cpdef float note(self, int note, int device_id=0):
        cdef int velocity = lpmidi_getnote(device_id, note)
        return float(velocity) / 127

    cpdef int notei(self, int note, int device_id=0):
        return lpmidi_getnote(device_id, note)

cdef class EventTriggerFactory:
    cpdef midi(self, double onset, double length, double freq, double amp, int channel=1, int device=-1):
        cdef char note, velocity
        cdef MidiEvent noteon
        cdef MidiEvent noteoff
        
        note = <char>midi.ftomi(freq)
        velocity = <char>max(0, min(127, (amp * 127)))

        noteon = MidiEvent(onset, length, <char>NOTE_ON, note, velocity, <char>channel)
        noteoff = MidiEvent(onset + length, 0, <char>NOTE_OFF, note, velocity, <char>channel)

        return [noteon, noteoff]
 
cdef class SessionParamBucket:
    """ params[key] to params.key

        An interface to redis to get and set shared session params.
    """
    def __getattr__(self, key):
        return self.get(key)

    def get(self, key, default=None):
        v = _redis.get(key.encode('ascii')) 
        if v is None:
            return default
        return v.decode('utf-8')

    def __setattr__(self, key, value):
        _redis.set(key.encode('ascii'), value.encode('ascii'))

cdef class ParamBucket:
    """ params[key] to params.key

        These params are passed in to the render context 
        through the play message to the renderer.
    """
    def __init__(self, str play_params=None):
        self._play_params = play_params

    def __getattr__(self, key):
        return self.get(key)

    def get(self, key, default=None):
        if self._params is None:
            self._params = self._parse_play_params()
        return self._params.get(key, default)

    def _parse_play_params(self):
        cdef dict params = {}
        cdef dict _params
        cdef str t, k, v

        if self._play_params is not None:
            _params = {}
            for t in self._play_params.split(' '):
                if '=' in t:
                    t = t.strip()
                    k, v = tuple(t.split('='))
                    _params[k] = v
            params.update(_params)
        return params

cdef class EventContext:
    def __cinit__(self, 
            str instrument_name=None, 
            str msg=None,
            object sounds=None,
            dict cache=None,
            int voice_id=-1
        ):

        self.cache = cache
        self.p = ParamBucket(str(msg))
        self.s = SessionParamBucket() 
        self.t = EventTriggerFactory()
        self.m = MidiEventListenerProxy()
        self.instrument_name = instrument_name
        self.sounds = sounds
        self.vid = voice_id

    def adc(self, length=1, offset=0, channels=2):
        return read_from_adc(length, offset=offset, channels=channels)

    def log(self, msg):
        logger.info('ctx.log[%s] %s' % (self.instrument_name, msg))

    def get_params(self):
        return self.p._params

cdef class Instrument:
    def __init__(self, str name, str path, object renderer):
        self.name = name
        self.path = path
        self.renderer = renderer
        self.sounds = self.load_sounds()
        self.cache = {}
        self.last_reload = 0

    def reload(self):
        logger.debug('Reloading instrument %s from %s' % (self.name, self.path))
        spec = importlib.util.spec_from_file_location(self.name, self.path)
        if spec is not None:
            renderer = importlib.util.module_from_spec(spec)
            try:
                spec.loader.exec_module(renderer)
            except Exception as e:
                logger.exception('Error reloading instrument module: %s' % str(e))

            if not hasattr(renderer, '_'):
                renderer._ = None

            self.renderer = renderer
            self.register_midi_triggers()
        else:
            logger.error('Error reloading instrument. Null spec at path:\n  %s' % self.path)

    def load_sounds(self):
        if hasattr(self.renderer, 'SOUNDS') and isinstance(self.renderer.SOUNDS, list):
            return [ dsp.read(snd) for snd in self.renderer.SOUNDS ]
        elif hasattr(self.renderer, 'SOUNDS') and isinstance(self.renderer.SOUNDS, dict):
            return { k: dsp.read(snd) for k, snd in self.renderer.SOUNDS.items() }
        return None

    def register_midi_triggers(self):
        if hasattr(self.renderer, 'MIDI'): 
            devices = []
            if isinstance(self.renderer.MIDI, list):
                devices += self.renderer.MIDI
            else:
                devices = [ self.renderer.MIDI ]

            try:
                for device, low, high in devices:
                    _redis.hset('%s-triggers' % device, self.name, 1)
                    _redis.set('%s-%s-triglow' % (device, self.name), str(low))
                    _redis.set('%s-%s-trighigh' % (device, self.name), str(high))
            except (TypeError, ValueError):
                logger.error('Could not unpack MIDI params from %s: %s' % (self.name, devices))


class InstrumentNotFoundError(Exception):
    def __init__(self, instrument_name, *args, **kwargs):
        self.message = 'No instrument named %s found' % instrument_name

def _load_instrument(name, path):
    """ Loads a renderer module from the script 
        at self.path 

        Failure to load the module raises an 
        InstrumentNotFoundError
    """
    try:
        logger.debug('Loading instrument %s from %s' % (name, path))
        spec = importlib.util.spec_from_file_location(name, path)
        if spec is not None:
            renderer = importlib.util.module_from_spec(spec)
            try:
                spec.loader.exec_module(renderer)
            except Exception as e:
                logger.exception('Error loading instrument module: %s' % str(e))

            if not hasattr(renderer, '_'):
                renderer._ = None

            instrument = Instrument(name, path, renderer)

        else:
            logger.error('Could not load instrument - spec is None: %s %s' % (path, name))
            raise InstrumentNotFoundError(name)
    except TypeError as e:
        logger.exception('TypeError loading instrument module: %s' % str(e))
        raise InstrumentNotFoundError(name) from e

    instrument.last_reload = os.path.getmtime(path)
    instrument.register_midi_triggers()
    return instrument

cdef tuple collect_players(object instrument):
    loop = False
    if hasattr(instrument.renderer, 'LOOP'):
        loop = instrument.renderer.LOOP

    # find all play functions
    players = set()

    # The simplest case is a single play function
    if hasattr(instrument.renderer, 'play'):
        players.add(instrument.renderer.play)

    # Play functions can also be registered via 
    # an @player.init decorator
    if hasattr(instrument.renderer, 'player') \
        and hasattr(instrument.renderer.player, 'players') \
        and isinstance(instrument.renderer.player.players, set):
        players |= instrument.renderer.player.players

    # Finally, any compatible function can be added to a set
    if hasattr(instrument.renderer, 'PLAYERS') \
        and isinstance(instrument.renderer.PLAYERS, set):
        players |= instrument.renderer.PLAYERS
    
    return players, loop

cdef int render_event(object instrument, lpmsg_t * msg):
    cdef set players
    cdef object onset_generator
    cdef bint loop
    cdef EventContext ctx 
    cdef str msgstr
    cdef bytes render_params = msg.msg
    cdef size_t onset = msg.onset_delay

    msgstr = render_params.decode('ascii')
    ctx = EventContext.__new__(EventContext,
        instrument_name=instrument.name, 
        msg=msgstr,
        sounds=instrument.sounds,
        cache=instrument.cache,
        voice_id=0,
    )

    logger.debug('rendering event %s w/params %s' % (str(instrument), render_params))

    if hasattr(instrument.renderer, 'before'):
        instrument.renderer.before(ctx)

    players, loop = collect_players(instrument)

    for player in players:
        try:
            ctx.count = 0
            ctx.tick = 0
            generator = player(ctx)

            try:
                for snd in generator:
                    bufstr = serialize_buffer(snd, onset, loop, msg)
                    _redis.publish('astridbuffers', bufstr)

            except Exception as e:
                logger.exception('Error during %s generator render: %s' % (ctx.instrument_name, e))
                return 1
        except Exception as e:
            logger.exception('Error allocating generator for %s render: %s' % (ctx.instrument_name, e))
            return 1

    if hasattr(instrument.renderer, 'done'):
        instrument.renderer.done(ctx)

    return 0

cdef set collect_trigger_planners(object instrument):
    # find all trigger planner functions
    planners = set()

    # The simplest case is a single trigger function
    if hasattr(instrument.renderer, 'trigger'):
        planners.add(instrument.renderer.trigger)

    # Trigger functions can also be registered via 
    # a @triggerer.init decorator
    if hasattr(instrument.renderer, 'triggerer') \
        and hasattr(instrument.renderer.triggerer, 'planners') \
        and isinstance(instrument.renderer.triggerer.planners, set):
        planners |= instrument.renderer.triggerer.planners

    # Finally, any compatible function can be added to a set
    if hasattr(instrument.renderer, 'TRIGGERERS') \
        and isinstance(instrument.renderer.TRIGGERERS, set):
        planners |= instrument.renderer.TRIGGERERS
    
    return planners

cdef int trigger_events(object instrument, lpmsg_t * msg):
    """ Collect the trigger functions in the instrument module
        and compute the triggers to be scheduled.
    """
    cdef set planners
    cdef bint loop
    cdef EventContext ctx 
    cdef str msgstr
    cdef int qfd
    cdef double now = 0
    cdef double loop_interval = 0
    cdef bytes trigger_params = msg.msg
    cdef list triggers = []

    msgstr = trigger_params.decode('ascii')
    ctx = EventContext.__new__(EventContext,
        instrument_name=instrument.name, 
        msg=msgstr,
        sounds=instrument.sounds,
        cache=instrument.cache,
        voice_id=0,
    )

    logger.debug('trigger generation event %s w/params %s' % (str(instrument), trigger_params))

    if hasattr(instrument.renderer, 'trigger_before'):
        instrument.renderer.trigger_before(ctx)

    planners = collect_trigger_planners(instrument)

    for p in planners:
        ctx.count = 0
        ctx.tick = 0
        try:
            # TODO allow single triggers OR lists of triggers here
            triggers += p(ctx)
        except Exception as e:
            logger.exception('Error during %s trigger generation: %s' % (ctx.instrument_name, e))
            return 1

    qfd = midi_triggerq_open()
    if qfd < 0:
        logger.exception('Error after %s trigger generation: Could not open fifo q' % ctx.instrument_name)
        return 1

    lpscheduler_get_now_seconds(&now)
    logger.info('NOW %f' % now)

    # Schedule the triggers
    for t in triggers:
        loop_interval = max(loop_interval, t.schedule_event(qfd))

    if midi_triggerq_close(qfd) < 0:
        logger.exception('Error after %s trigger generation: Could not close fifo q' % ctx.instrument_name)
        return 1

    if hasattr(instrument.renderer, 'trigger_done'):
        instrument.renderer.trigger_done(ctx)

    # If looping, schedule the retrigger callback
    # Send a new play message, cloned from the original 
    # with an onset delay of largest onset + largest onset length
    # Always loop for now.
    msg.timestamp = now + loop_interval
    logger.info('scheduling retrigger msg: loop_interval %f timestamp %f now %f (%s)' % (loop_interval, msg.timestamp, now, msg.type))
    if send_message(msg[0]) < 0:
        logger.exception('Error after %s trigger generation: Could not send retrigger loop message' % ctx.instrument_name)
        return 1

    return 0


ASTRID_INSTRUMENT = None

cdef public int astrid_reload_instrument(char * path) except -1:
    cdef size_t last_edit 
    global ASTRID_INSTRUMENT

    if ASTRID_INSTRUMENT is None:
        _path = path.decode('utf-8')
        name = Path(_path).stem
        ASTRID_INSTRUMENT = _load_instrument(name, _path)
        return 0

    last_edit = os.path.getmtime(ASTRID_INSTRUMENT.path)
    if last_edit > ASTRID_INSTRUMENT.last_reload:
        ASTRID_INSTRUMENT.reload()
        ASTRID_INSTRUMENT.last_reload = last_edit

    return 0


cdef public int astrid_load_instrument(char * path) except -1:
    global ASTRID_INSTRUMENT
    _path = path.decode('utf-8')
    name = Path(_path).stem

    ASTRID_INSTRUMENT = _load_instrument(name, _path)
    return 0

cdef public int astrid_schedule_python_render(void * msgp) except -1:
    global ASTRID_INSTRUMENT
    cdef lpmsg_t * msg = <lpmsg_t *>msgp
    cdef size_t last_edit = os.path.getmtime(ASTRID_INSTRUMENT.path)

    # Reload instrument
    if last_edit > ASTRID_INSTRUMENT.last_reload:
        ASTRID_INSTRUMENT.reload()
        ASTRID_INSTRUMENT.last_reload = last_edit

    return render_event(ASTRID_INSTRUMENT, msg)


cdef public int astrid_schedule_python_triggers(void * msgp) except -1:
    global ASTRID_INSTRUMENT
    cdef lpmsg_t * msg = <lpmsg_t *>msgp
    cdef size_t last_edit = os.path.getmtime(ASTRID_INSTRUMENT.path)

    # Reload instrument
    if last_edit > ASTRID_INSTRUMENT.last_reload:
        ASTRID_INSTRUMENT.reload()
        ASTRID_INSTRUMENT.last_reload = last_edit

    return trigger_events(ASTRID_INSTRUMENT, msg)


