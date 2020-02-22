#cython: language_level=3

""" Some helpers for building and transforming onset lists
"""

from pippi cimport wavetables
from pippi cimport interpolation
from pippi cimport dsp
from pippi.soundbuffer cimport SoundBuffer
from pippi cimport seq as _seq
from pippi cimport rand
import pyparsing
import numpy as np

REST_SYMBOLS = set(('0', '.', ' ', '-', 0, False))

CLAVE = {
    'son':      'x..x..x...x.x...', 
    'rumba':    'x..x...x..x.x...', 
    'bell':     'x..x..xx..x.x..x', 
    'tresillo': 'x..x..x.', 
}

SON = CLAVE['son']
RUMBA = CLAVE['rumba']
BELL = CLAVE['bell']
TRESILLO = CLAVE['tresillo']

cdef class Seq:
    def __init__(Seq self, object bpm=None):
        if bpm is None:
            bpm = 120.0
        self.bpm = bpm
        self.drums = {}

    cpdef void add(Seq self, 
            str name, 
            object pattern=None, 
            object callback=None, 
            object sounds=None, 
            object barcallback=None, 
            double swing=0, 
            double div=1, 
            object lfo=None,
            double delay=0
        ):
        self.drums[name] = dict(
            name=name, 
            pattern=pattern, 
            sounds=list(sounds or []), 
            callback=callback, 
            barcallback=barcallback, 
            div=div, 
            swing=swing, 
            lfo=lfo, 
            delay=delay
        )

    def update(self, name, **kwargs):
        self.drums[name].update(kwargs)

    cpdef SoundBuffer play(Seq self, double length):
        cdef SoundBuffer out = SoundBuffer(length=length)
        cdef SoundBuffer bar
        cdef dict drum
        cdef list onsets
        cdef double onset
        cdef SoundBuffer clang
        cdef int count

        for k, drum in self.drums.items():
            bar = SoundBuffer(length=length)
            onsets = frompattern(
                        drum['pattern'], 
                        bpm=self.bpm, 
                        length=length, 
                        swing=drum['swing'], 
                        div=drum['div'], 
                        lfo=drum['lfo'], 
                        delay=drum['delay'],
                    )

            count = 0
            for onset in onsets:
                if drum.get('callback', None) is not None:
                    clang = drum['callback'](onset/length, count)
                elif len(drum['sounds']) > 0:
                    clang = SoundBuffer(filename=str(rand.choice(drum['sounds'])))
                else:
                    clang = SoundBuffer()
                bar.dub(clang, onset)
                count += 1

            if drum.get('barcallback', None) is not None:
                bar = drum['barcallback'](bar)
            out.dub(bar)

        return out

cpdef list frompattern(
    object pat,            # Pattern
    double bpm=120.0,      # Tempo in beats per minute
    double length=1,       # Output length in seconds 
    double swing=0,        # MPC swing amount 0-1
    double div=1,          # Beat subdivision
    object lfo=None,       # Apply lfo tempo modulation across pattern (string, iterable, soundbuffer, etc)
    double delay=0,        # Fixed delay in seconds added to each onset
):
    """ Onsets from ascii
    """
    bpm = bpm if bpm > 0 else np.nextafter(0, 1)
    beat = (60. / bpm) / div

    positions = topositions(pat, beat, length, lfo)
    positions = onsetswing(positions, swing, beat)

    if delay > 0:
        positions = [ pos + delay for pos in positions ]

    return positions

cpdef list topositions(object p, double beat, double length, wavetables.Wavetable lfo=None):
    cdef double pos = 0
    cdef int count = 0
    cdef double delay = 0
    cdef double div = 1
    cdef list out = []

    while pos < length:
        index = count % len(p)
        event = p[index]

        if lfo is not None:
            delay += interpolation._linear_pos(lfo.data, pos/length)
            pos += delay

        if event in REST_SYMBOLS:
            count += 1
            pos += beat
            continue

        elif event == '[':
            end = p.find(']', index) - 1
            div = len(p[index : end])
            beat /= div
            count += 1
            continue

        elif event == ']':
            beat *= div
            count += 1
            continue

        out += [ pos ]
        pos += beat
        count += 1

    return out

def render(pattern, callback, beat=0.25, length=1, truncate=False):
    out = dsp.buffer(length=length)

    positions = topositions(pattern, beat, length)

    for pos in positions:
        clang = callback(pos/length)
        if truncate and pos + clang.dur > length:
            clang = clang.cut(0, length - pos)
        out.dub(clang, pos)

    return out


def normalize_pattern(pattern, reverse=False, rotate=None):
    pattern = [ 0 if tick in REST_SYMBOLS or not tick else 1 for tick in pattern ]
    if reverse:
        pattern = [ p for p in reversed(pattern) ]

    if rotate:
        pattern = [ 0 for _ in range(rotate) ] + pattern[:-rotate]

    return pattern

def onsetswing(onsets, amount, beat):
    """ Add MPC-style swing to a list of onsets.
        Amount is a value between 0 and 1, which 
        maps to a swing amount between 0% and 75%.
        Every odd onset will be delayed by
            
            beat length * swing amount

        This will only really work like MPC swing 
        when there is a run of notes, as rests 
        are not represented in onset lists (and 
        MPC swing just delays the upbeats).
    """
    if amount <= 0:
        return onsets

    delay_amount = beat * amount * 0.75
    for i, onset in enumerate(onsets):
        if i % 2 == 1:
            onsets[i] += delay_amount

    return onsets


def pgen(numbeats, div=1, offset=0, reps=None, reverse=False):
    """ Pattern creation helper
    """
    pat = [ 1 if tick == 0 else 0 for tick in range(div) ]
    pat = [ pat[i % len(pat)] for i in range(numbeats) ]

    if offset > 0:
        pat = [ 0 for _ in range(offset) ] + pat[:-offset]

    if reps is not None:
        pat *= reps

    if reverse:
        pat = [ p for p in reversed(pat) ]

    return pat

def onsets(pattern, beat=0.2, length=None, start=0):
    length = length or len(pattern)
    grid = [ beat * i + start for i in range(length) ]
    pattern = [ pattern[i % len(pattern)] for i in range(length) ]

    out = []

    for i, tick in enumerate(pattern):
        if tick not in REST_SYMBOLS:
            pos = grid[i % len(grid)]
            out += [ pos ]
        
    return out

def eu(length, numbeats, offset=0, reps=None, reverse=False):
    """ A euclidian pattern generator

        Length 6, numbeats 3
        >>> rhythm.eu(6, 3)
        [1, 0, 1, 0, 1, 0]

        Length 6, numbeats 3, offset 1
        >>> rhythm.eu(6, 3, 1)
        [0, 1, 0, 1, 0, 1]
    """
    pulses = [ 1 for pulse in range(numbeats) ]
    pauses = [ 0 for pause in range(length - numbeats) ]

    position = 0
    while len(pauses) > 0:
        try:
            index = pulses.index(1, position)
            pulses.insert(index + 1, pauses.pop(0))
            position = index + 1
        except ValueError:
            position = 0

    pattern = rotate(pulses, offset+len(pulses))

    if reps:
        pattern *= reps

    if reverse:
        pattern = [ p for p in reversed(pattern) ]

    return pattern

def curve(numbeats=16, wavetable=None, reverse=False):
    win = dsp.wt(wavetable or dsp.HANN, numbeats)

    if reverse:
        win = win.reversed()

    return win

def rotate(pattern, offset=0):
    """ Rotate a pattern list by a given offset
    """
    return pattern[-offset % len(pattern):] + pattern[:-offset % len(pattern)]

def repeat(onsets, reps):
    """ Repeat a sequence of onsets a given number of times
    """
    out = []
    total = sum(onsets)
    for rep in range(reps):
        out += [ onset + (rep * total) for onset in onsets ]

    return out

def seq(*args, **kwargs):
    return Seq(*args, **kwargs)


