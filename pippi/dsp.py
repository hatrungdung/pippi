#!/usr/bin/env python2.7
# -*- coding: UTF-8 -*-

""" Anti-copywrite. Do whatever you would like with this software.
"""

import wave
import audioop
import math
import random
import struct
import time
import hashlib
import os
import sys
from datetime import datetime
import time
from docopt import docopt
import collections
from _pippic import amp, am, add, shift, mix, pine, synth, curve
from _pippic import env as cenv
from _pippic import cycle as ccycle

bitdepth = 16
audio_params = [2, 2, 44100, 0, "NONE", "not_compressed"]
snddir = '' 
dsp_grain = 64
env_min = 2 
cycle_count = 0
thetime = 0
seedint = 0
seedstep = 0
seedhash = ''
randconst = 1.0 
cpop = 0.444
crpop = 3.9
quiet = True 


##############
# List helpers
##############

def interleave(list_one, list_two):
    """ Combine two lists by interleaving their elements """
    # find the length of the longest list
    if len(list_one) > len(list_two):
        big_list = len(list_one)
    elif len(list_two) > len(list_one):
        big_list = len(list_two)
    else:
        if randint(0, 1) == 0:
            big_list = len(list_one)
        else:
            big_list = len(list_two)

    combined_lists = []

    # loop over it and insert alternating items
    for index in range(big_list):
        if index <= len(list_one) - 1:
            combined_lists.append(list_one[index])
        if index <= len(list_two) - 1:
            combined_lists.append(list_two[index])

    return combined_lists

def packet_shuffle(list, packet_size):
    """ Shuffle a subset of list items in place.
        Takes a list, splits it into sub-lists of size N
        and shuffles those sub-lists. Returns flattened list.
    """
    if packet_size >= 3 and packet_size <= len(list):
        lists = list_split(list, packet_size)
        shuffled_lists = []
        for sublist in lists:
            shuffled_lists.append(randshuffle(sublist))

        big_list = []
        for shuffled_list in shuffled_lists:
            big_list.extend(shuffled_list)
        return big_list

def list_split(list, packet_size):
    """ Split a list into groups of size N """
    trigs = []
    for i in range(len(list)):
        if i % int(packet_size) == 0:
            trigs.append(i)

    newlist = []

    for trig_index, trig in enumerate(trigs):
        if trig_index < len(trigs) - 1:
            packets = []
            for packet_bit in range(packet_size):
                packets.append(list[packet_bit + trig])

            newlist.append(packets)

    return newlist

def rotate(list, start=0, rand=False):
    """ Rotate a list by a given offset """

    if rand == True:
        start = randint(0, len(list) - 1)

    if start > len(list) - 1:
        start = len(list) - 1

    return list[start:] + list[:start]

def eu(length, numpulses):
    """ Euclidian pattern generator
    """
    pulses = [ 1 for pulse in range(numpulses) ]
    pauses = [ 0 for pause in range(length - numpulses) ]

    position = 0
    while len(pauses) > 0:
        try:
            index = pulses.index(1, position)
            pulses.insert(index + 1, pauses.pop(0))
            position = index + 1
        except ValueError:
            position = 0

    return pulses








#################
# Runtime helpers
#################

def timer(cmd='start'):
    """ Counts elapsed time between start and stop events. 
        Useful for tracking render time. """
    global thetime
    if cmd == 'start':
        thetime = time.time()

        if not quiet: print 'Started render at timestamp', thetime
        return thetime 
    elif cmd == 'stop':
        thetime = time.time() - thetime
        themin = int(thetime) / 60
        thesec = thetime - (themin * 60)
        if not quiet: print 'Render time:', themin, 'min', thesec, 'sec'
        return thetime




##############
# Utils
##############

def log(message, mode="a"):
    logfile = open(os.path.expanduser("~/pippi.error.log"), mode)
    logfile.write(str(message) + "\n")
    return logfile.close()

def flen(string):
    # string length in frames
    return len(string) / (audio_params[1] + audio_params[0])

def byte_string(number):
    """ Return integer encoded as bytes formatted for wave data """
    number = cap(number, 32767, -32768)
    return struct.pack("<h", number)

def pack(number):
    """ wrapper for byte_string that takes -1.0 to 1.0 as input """
    number = cap(number, 1.0, -1.0)
    number = (number + 1.0) * 0.5
    number = number * 65535 - 32768
    return byte_string(int(number))

def scale(low_target, high_target, low, high, pos):
    pos = float(pos - low) / float(high - low) 
    return pos * float(high_target - low_target) + low_target
    
def cap(num, max, min=0):
    if num < min:
        num = min
    elif num > max:
        num = max
    return num

def timestamp_filename():
    """ Generate a datetime string to add to filenames """
    current_time = str(datetime.time(datetime.now()))
    current_time = current_time.split(':')
    current_seconds = current_time[2].split('.')
    current_time = current_time[0] + '.' + current_time[1] + '.' + current_seconds[0]
    current_date = str(datetime.date(datetime.now()))

    return current_date + "_" + current_time











###############
# Random
###############

def seed(theseed=False):
    global seedint
    global seedhash

    if theseed == False:
        theseed = cycle(440)

    h = hashlib.sha1(theseed)
    seedhash = h.digest()

    seedint = int(''.join([str(ord(c)) for c in list(seedhash)]))
    return seedint

def stepseed():
    global seedint
    global seedstep

    h = hashlib.sha1(str(seedint))
    seedint = int(''.join([str(ord(c)) for c in list(h.digest())]))

    seedstep = seedstep + 1

    return seedint

def rpop(low=0.0, high=1.0):
    global cpop
    global crpop
    cpop = crpop * cpop * (1.0 - cpop)
    
    return cpop * (high - low) + low

def randint(lowbound=0, highbound=1):
    global seedint

    if seedint > 0:
        return int(rand() * (highbound - lowbound) + lowbound)
    else:
        return random.randint(lowbound, highbound)

def rand(lowbound=0, highbound=1):
    global seedint
    if seedint > 0:
        return ((stepseed() / 100.0**20) % 1.0) * (highbound - lowbound) + lowbound
    else:
        return random.random() * (highbound - lowbound) + lowbound

def randchoose(items):
    return items[randint(0, len(items)-1)]

def randshuffle(input):
    items = input[:]
    shuffled = []
    for i in range(len(items)):
        if len(items) > 0:
            item = randchoose(items)
            shuffled.append(item)
            items.remove(item)

    return shuffled 








###############
# DSP
###############

def bln(length, low=3000.0, high=7100.0, wform='sine2pi'):
    """ Time-domain band-limited noise generator
    """
    outlen = 0
    cycles = ''
    while outlen < length:
        acycle = cycle(rand(low, high), wform)
        outlen += len(acycle)
        cycles += acycle

    return cycles

def transpose(audio_string, amount):
    """ Transpose an audio fragment by a given amount.
        1.0 is unchanged, 0.5 is half speed, 2.0 is twice speed, etc """
    amount = 1.0 / float(amount)

    audio_string = audioop.ratecv(audio_string, audio_params[1], audio_params[0], audio_params[2], int(audio_params[2] * amount), None)
    
    return audio_string[0]

def tone(length=44100, freq=440.0, wavetype='sine', amp=1.0, phase=0.0, offset=0.0):
    # Quick and dirty mapping to transition to the new api
    if wavetype == 'sine2pi' or wavetype == 'sine':
        wtype = 0
    elif wavetype == 'cos2pi' or wavetype == 'cos':
        wtype = 1
    elif wavetype == 'hann':
        wtype = 2
    elif wavetype == 'tri':
        wtype = 3
    elif wavetype == 'saw' or wavetype == 'line':
        wtype = 4
    elif wavetype == 'isaw' or wavetype == 'phasor':
        wtype = 5
    elif wavetype == 'vary':
        wtype = 6
    elif wavetype == 'impulse':
        wtype = 7
    elif wavetype == 'square':
        wtype = 8
    else:
        wtype = 0

    return synth(wtype, float(freq), length, amp, phase, offset)
        
def alias(audio_string, passthru = 0, envelope = 'random', split_size = 0):
    if passthru > 0:
        return audio_string

    if envelope == 'flat':
        envelope = False

    if split_size == 0:
        split_size = dsp_grain / randint(1, dsp_grain)

    packets = split(audio_string, split_size)
    packets = [p*2 for i, p in enumerate(packets) if i % 2]

    out = ''.join(packets)

    if envelope:
        out = env(out, envelope)

    return out 

def noise(length):
    return ''.join([byte_string(randint(-32768, 32767)) for i in range(length * audio_params[0])])

def cycle(freq, wavetype='sine2pi', amp=1.0):
    #wavecycle = wavetable(wavetype, htf(freq))
    # Quick and dirty mapping to transition to the new api
    if wavetype == 'sine2pi' or wavetype == 'sine':
        wtype = 0
    elif wavetype == 'cos2pi' or wavetype == 'cos':
        wtype = 1
    elif wavetype == 'hann':
        wtype = 2
    elif wavetype == 'tri':
        wtype = 3
    elif wavetype == 'saw' or wavetype == 'line':
        wtype = 4
    elif wavetype == 'isaw' or wavetype == 'phasor':
        wtype = 5
    elif wavetype == 'vary':
        wtype = 6
    elif wavetype == 'impulse':
        wtype = 7
    elif wavetype == 'square':
        wtype = 8

    #return ''.join([pack(amp * s) * audio_params[0] for s in wavecycle])
    return ccycle(wtype, htf(freq), 1.0, amp)

def fill(string, length, chans=2, silence=False):
    if flen(string) < length:
        if silence == False:
            try:
                repeats = length / flen(string) + 1
            except ZeroDivisionError:
                return string
        else:
            return pad(string, 0, length - flen(string))

        string = string * repeats

    return cut(string, 0, length)

def pad(string, start, end):
    # start and end given in samples, as usual 
    # eg lengths of silence to pad at start and end

    zero = struct.pack('<h', 0)
    zero = zero[0:1] * audio_params[1] * audio_params[0] # will we ever have a width > 2? donno.

    return "%s%s%s" % ((start * zero), string, (end * zero))

def iscrossing(first, second):
    """ Detects zero crossing between two mono frames """

    if len(first) == 2 and len(second) == 2:
        first = struct.unpack("<h", first) 
        second = struct.unpack("<h", second) 

        if first[0] > 0 and second[0] < 0:
            return True
        elif first[0] < 0 and second[0] > 0:
            return True
        elif first[0] == 0 and second[0] != 0:
            return False 
        elif first[0] != 0 and second[0] == 0:
            return True 

    return False

def insert_into(haystack, needle, position):
    # split string at position index
    hay = cut(haystack, 0, position)
    stack = cut(haystack, position, flen(haystack) - position)
    return "%s%s%s" % (hay, needle, stack)

def replace_into(haystack, needle, position):
    hayend = position * audio_params[1] * audio_params[0]
    stackstart = hayend - (flen(needle) * audio_params[1] * audio_params[0])
    return "%s%s%s" % (haystack[:hayend], needle, haystack[stackstart:])

def cut(string, start, length):
    # start and length are both given in frames (aka samples)za

    #if start + length > flen(string):
        #log('No cut for you!')
        #log('in len: '+str(flen(string))+'start: '+str(start)+' length: '+str(length))

    length = int(length) * audio_params[1] * audio_params[0]
    start = int(start) * audio_params[1] * audio_params[0]

    return string[start : start + length]

def splitmono(string):
    """ split a stereo sound into a list of mono sounds """
    left = audioop.tomono(string, audio_params[1], 1, 0)
    right = audioop.tomono(string, audio_params[1], 0, 1)

    return [left, right] 

def split(string, size, chans=2):
    """ split a sound into chunks of size N in frames, or by zero crossings """
    if size == 0:
        if chans == 2:
            # split into mono files
            tracks = splitmono(string)

            for i, track in enumerate(tracks):
                tracks[i] = split(track, 0, 1)

            return tracks

        elif chans == 1:
            frames = split(string, 1, 1)
            chunk, chunks = [], []

            for i, frame in enumerate(frames):
                try:
                    if chunk == []:
                        chunk += [ frame ]
                    elif iscrossing(frame, frames[i+1]) == False and chunk != []:
                        chunk += [ frame ]
                    elif iscrossing(frame, frames[i+1]) == True and chunk != []:
                        chunk += [ frame ]
                        chunks += [ ''.join(chunk) ]
                        chunk = []
                except IndexError:
                    chunk += [ frame ]
                    chunks += [ ''.join(chunk) ]

            return chunks

    elif size > 0:
        frames = int(size) * audio_params[1] * chans 
        return [string[frames * count : (frames * count) + frames] for count in range(len(string) / frames)]

def vsplit(input, minsize, maxsize):
    # min/max size is in frames...
    output = []
    pos = 0

    for chunk in range(flen(input) / minsize):
        chunksize = randint(minsize, maxsize)
        if pos + chunksize < flen(input) - chunksize:
            output.append(cut(input, pos, chunksize))
            pos += chunksize

    return output

def write(audio_string, filename, timestamp=False):
    """ Write audio data to renders directory with the Python wave module """
    if timestamp == True:
        filename = filename + '-' + timestamp_filename() + '.wav' 
    else:
        filename = filename + '.wav'

    wavfile = wave.open(os.getcwd() + '/' + filename, "w")
    wavfile.setparams(audio_params)
    wavfile.writeframes(audio_string)
    wavfile.close()
    return filename

def cache(s='', clear=False):
    """ Simple write() wrapper to create and clear cache audio """
    if clear == True:
        files = os.listdir('renders/cache/')
        for file in files:
            os.remove('renders/cache/' + file)
    else:
        return write(s, 'cache/c', True)

    return s 

def read(filename):
    """ Read a 44.1k / 16bit WAV file from disk with the Python wave module. 
        Mono files are converted to stereo automatically. """
    filename = snddir + filename
    if not quiet: print 'loading', filename

    file = wave.open(filename, "r")
    file_frames = file.readframes(file.getnframes())

    snd = Sound()

    # check for mono files
    if file.getnchannels() == 1:
        file_frames = audioop.tostereo(file_frames, file.getsampwidth(), 0.5, 0.5)
        snd.params = file.getparams()
        snd.params = (2, snd.params[1], snd.params[2], snd.params[3], snd.params[4], snd.params[5])
    else:
        snd.params = file.getparams()

    snd.data = file_frames

    return snd

def pantamp(pan_pos):
    # Translate the pan position into a tuple size two of left amp and right amp
    if pan_pos > 0.5:
        pan_pos -= 0.5
        pan_pos *= 2.0
        pan_pos = 1.0 - pan_pos
        pan_pos = (pan_pos, 1.0)
    elif pan_pos < 0.5:
        pan_pos *= 2.0
        pan_pos = (1.0, pan_pos)
    else:
        pan_pos = (1.0, 1.0)

    return pan_pos

def pan(slice, pan_pos=0.5, amp=1.0):
    amps = pantamp(pan_pos)

    lslice = audioop.tomono(slice, audio_params[1], 1, 0)
    lslice = audioop.tostereo(lslice, audio_params[1], amps[0], 0)

    rslice = audioop.tomono(slice, audio_params[1], 0, 1)
    rslice = audioop.tostereo(rslice, audio_params[1], 0, amps[1])

    slice = audioop.add(lslice, rslice, audio_params[1])
    return audioop.mul(slice, audio_params[1], amp)

def env(audio_string, wavetype="sine", fullres=False, highval=1.0, lowval=0.0, wtype=0, amp=1.0, phase=0.0, offset=0.0, mult=1.0):
    """ Temp wrapper for new env function """

    # Quick and dirty mapping to transition to the new api
    if wavetype == 'sine2pi' or wavetype == 'sine':
        wtype = 0
    elif wavetype == 'cos2pi' or wavetype == 'cos':
        wtype = 1
    elif wavetype == 'hann':
        wtype = 2
    elif wavetype == 'tri':
        wtype = 3
    elif wavetype == 'saw' or wavetype == 'line':
        wtype = 4
    elif wavetype == 'isaw' or wavetype == 'phasor':
        wtype = 5
    elif wavetype == 'vary':
        wtype = 6
    elif wavetype == 'impulse':
        wtype = 7
    elif wavetype == 'square':
        wtype = 8
    elif wavetype == 'random':
        wtype = randint(0, 8)

    return cenv(audio_string, wtype, amp, phase, offset, mult)

def benv(sound, points):
    chunksize = flen(sound) / (len(points) - 1)
    sounds = split(sound, chunksize, audio_params[0])

    return ''.join([env(s, 'line', points[i + 1], points[i]) for i, s in enumerate(sounds)])
 
def panenv(sound, ptype='line', etype='sine', panlow=0.0, panhigh=1.0, envlow=0.0, envhigh=1.0):
    packets = split(sound, dsp_grain)

    ptable = wavetable(ptype, len(packets), panlow, panhigh)
    etable = wavetable(etype, len(packets), envlow, envhigh)

    packets = [pan(p, ptable[i], etable[i]) for i, p in enumerate(packets)]

    return ''.join(packets)

def drift(sound, amount):
    high = (amount * 0.5) + 1.0
    low = 1.0 - (amount * 0.5)
    sound = split(sound, 441)
    freqs = wavetable('sine', len(sound), high, low)
    sound = [ transpose(sound[i], freqs[i]) for i in range(len(sound)) ]

    return ''.join(sound)

def fnoise(sound, coverage):
    target_frames = int(flen(sound) * coverage)

    for i in range(target_frames):
        p = randint(0, flen(sound) - 1)
        f = cut(sound, p, 1)
        sound = replace_into(sound, f, randint(0, flen(sound) - 1))

    return sound











####################
# Periodic functions
####################

def breakpoint(values, size=512):
    """ Takes a list of values, or a pair of wavetable types and values, 
    and builds an interpolated list of points between each value using 
    the wavetable type. Default table type is linear. """

    # we need at least a start and end point
    if len(values) < 2:
        values = [ 0.0, ['line', 1.0] ] 

    # Handle some small size cases
    if size == 0:
        #log('WARNING: breakpoint size 0')
        #log('values: '+str(values))
        #log('')
        return []
    elif size < 4 and size > 0:
        #log('WARNING: small breakpoint, size ' + str(size))
        #log('values: '+str(values))
        #log('')
        return [values[0] for i in range(size)]

    # Need at least one destination value per point computed
    if size < len(values):
        values = values[:size]

    # Each value produces a group of intermediate points
    groups = []

    # The size of each group of intermediate points is divded evenly into the target 
    # size, ignoring the first value and accounting for uneven divisions.
    gsize = size / (len(values)-1) 
    gsizespill = size % (len(values)-1)

    # Pretend the first loop shifts the last endval to the startval
    try:
        if len(values[0]) > 1:
            endval = values[0][1]
    except TypeError:
        endval = values[0]
    values.pop(0)

    # To build the list of points, loop through each value
    for i, v in enumerate(values):
        try:
            if len(v) > 1:
                wtype = v[0]
                startval = endval 
                endval = v[1]

            if len(v) == 3:
                gsize = gsize * v[2]
        except TypeError:
            wtype = 'line'
            startval = endval
            endval = v

        # Pad last group with leftover points
        if v == values[-1]:
            gsize += gsizespill 

        groups.extend(wavetable(wtype, gsize, endval, startval))

    return groups

def wavetable(wtype="sine", size=512, highval=1.0, lowval=0.0, rf = rand):
    """ The end is near. That'll do, wavetable()
    """
    wtable = []
    wave_types = ["sine", "gauss", "cos", "line", "saw", "impulse", "phasor", "sine2pi", "cos2pi", "vary", "flat"]

    if wtype == "random":
        wtype = wave_types[int(rf(0, len(wave_types) - 1))]

    if wtype == "sine":
        wtable = [math.sin(i * math.pi) * (highval - lowval) + lowval for i in frange(size, 1.0, 0.0)]

    elif wtype == "hann" or wtype == "hanning":
        wtable = [ 0.5 * ( 1 - math.cos((2 * math.pi * i) / (size - 1))) for i in range(size) ]

    elif wtype == "gauss":
        def gauss(x):
            # From: http://johndcook.com/python_phi.html
            # Prolly doing it wrong!
            a1 =  0.254829592
            a2 = -0.284496736
            a3 =  1.421413741
            a4 = -1.453152027
            a5 =  1.061405429
            p  =  0.3275911

            sign = 1
            if x < 0:
                sign = -1
            x = abs(x)/math.sqrt(2.0)

            t = 1.0/(1.0 + p * x)
            y = 1.0 - (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * math.exp(-x * x)

            return abs(abs(sign * y) - 1.0)

        wtable = [gauss(i) * (highval - lowval) + lowval for i in frange(size, 2.0, -2.0)] 
    elif wtype == "sine2pi":
        wtable = [math.sin(i * math.pi * 2) * (highval - lowval) + lowval for i in frange(size, 1.0, 0.0)]

    elif wtype == "cos2pi":
        wtable = [math.cos(i * math.pi * 2) * (highval - lowval) + lowval for i in frange(size, 1.0, 0.0)]

    elif wtype == "cos":
        wtable = [math.cos(i * math.pi) * (highval - lowval) + lowval for i in frange(size, 1.0, 0.0)]

    elif wtype == "itri":
        # Inverted triangle
        wtable = [math.fabs(i) for i in frange(size, highval, lowval - highval)] # Only really a triangle wave when centered on zero 

    elif wtype == "tri":
        wtable = [ (2.0 / (size + 1)) * ((size + 1) / 2.0 - math.fabs(i - ((size - 1) / 2.0))) for i in range(size) ]

    elif wtype == "saw" or wtype == "line":
        wtable = [i for i in frange(size, highval, lowval)]

    elif wtype == "phasor":
        wtable = wavetable("line", size, highval, lowval)
        list.reverse(wtable)

    elif wtype == "impulse":
        wtable = [float(randint(-1, 1)) for i in range(size / randint(2, 12))]
        wtable.extend([0.0 for i in range(size - len(wtable))])

    elif wtype == "vary":
        if size < 32:
            bsize = size
        else:
            bsize = size / int(rf(2, 16))

        btable = [ [wave_types[int(rf(0, len(wave_types)-1))], rf(lowval, highval)] for i in range(bsize) ]

        if len(btable) > 0:
            btable[0] = lowval
        else:
            btable = [lowval]

        wtable = breakpoint(btable, size) 
    elif wtype == "flat":
        wtable = [highval for i in range(size)]
    
    return wtable

def frange(steps, highval=1.0, lowval=0.0):
    if steps == 1:
        return [lowval]

    return  [ (i / float(steps-1)) * (highval - lowval) + lowval for i in range(steps)]

def prob(item_dictionary):
    weighted_list = []
    for item, weight in item_dictionary.iteritems():
        for i in range(weight):
            weighted_list.append(item)

    return randchoose(weighted_list)





#################
# Unit conversion
#################

def stf(s):
    ms = s * 1000.0
    return mstf(ms)

def mstf(ms):
    frames_in_ms = audio_params[2] / 1000.0
    frames = ms * frames_in_ms

    return int(frames)

def ftms(frames):
    ms = frames / float(audio_params[2]) 
    return ms * 1000

def fts(frames):
    s = frames / float(audio_params[2])
    return s

def ftc(frames):
    frames = int(frames)
    frames *= audio_params[1] # byte width
    frames *= audio_params[0] # num chans

    return frames

def fth(frames):
    """ frames to hz """
    if frames > 0:
        return (1.0 / frames) * audio_params[2]

    return 0

def htf(hz):
    """ hz to frames """
    if hz > 0:
        frames = audio_params[2] / float(hz)
    else:
        frames = 1 # 0hz is okay...

    return int(frames)

def bpm2ms(bpm):
    return 60000.0 / float(bpm)

def bpm2frames(bpm):
    return int((bpm2ms(bpm) / 1000.0) * audio_params[2]) 




###########
# Misc
###########

def delay(frames):
    target = (frames / 44100.0) + time.time()

    time.sleep(fts(frames))

    while target > time.time():
        time.sleep(target - time.time())

    return True

def pipe(f):
    if sys.argv is not '':
        args = docopt(f.__doc__)
        # TODO parse args and stream to stdout
        f(args)







class Sound(collections.MutableSequence):
    def __init__(self, data='', channels=2, depth=16, rate=44100):
        super(Sound, self).__init__()

        self.data = data
        self.channels = channels
        self.depth = depth
        self.rate = rate

    def __len__(self):
        return flen(self.data)

    def __str__(self):
        return self.data

    def __contains__(self, key):
        """ TODO: search by frame or group of frames """
        return False

    def __getitem__(self, index):
        return cut(self.data, index, 1)

    def __setitem__(self, index, value):
        replace_into(self.data, value, index)

    def __delitem__(self, value):
        """ TODO: delete frames by value... """
        pass

    def insert(self, index, value):
        insert_into(self.data, value, index)

    def append(self, value):
        if isinstance(value, Sound) or isinstance(value, list):
            self.frames += [ value ]
        else:
            print 'foo'

    def loop(self, loops):
        self.data = self.data * loops

    def __iadd(self, value):
        return self.__add__(value)

    def __add__(self, value):
        if isinstance(value, int):
            return value
        elif isinstance(value, Sound):
            return Sound(mix([self.data, value.data]))

    def __radd__(self, value):
        return self.__add__(value)

    def __sub__(self, value):
        if isinstance(value, int):
            return Sound([ frame - int(value) for frame in self.frames ])
        elif isinstance(value, Sound):
            return Sound([ frame - value[i] for i, frame in enumerate(self.frames) ])
    
    def __rsub__(self, value):
        if isinstance(value, int):
            return Sound([ int(value) - frame for frame in self.frames ])
        elif isinstance(value, Sound):
            return Sound([ value[i] - frame for i, frame in enumerate(self.frames) ])

    def __mul__(self, value):
        if isinstance(value, int) or isinstance(value, float):
            return Sound([ frame * value for frame in self.frames ])
        elif isinstance(value, Sound):
            return Sound([ frame * value[i] for i, frame in enumerate(self.frames) ])

    def __rmul__(self, value):
        return self.__mul__(value)

    def unpack(self, value=None):
        value = self.data if value is None else value

        return [ struct.unpack('<h', '%s%s' % (frame, value[i]))[0]  for i, frame in enumerate(value) if i % 2 == 0 and i < len(value) - 1 ] 

