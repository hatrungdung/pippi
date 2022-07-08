from os import path
import random
import shutil
import tempfile
from unittest import TestCase

from pippi.soundbuffer import SoundBuffer
from pippi import dsp, grains, grains2

class TestCloud(TestCase):
    def setUp(self):
        self.soundfiles = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.soundfiles)

    def test_libpippi_graincloud(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')
        cloud = grains2.Cloud2(sound)

        length = 30
        framelength = int(length * sound.samplerate)

        out = cloud.play(length)

        out.write('tests/renders/graincloud_libpippi.wav')

        self.assertEqual(len(out), framelength)

    def test_unmodulated_graincloud(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')
        cloud = grains.Cloud(sound)

        length = 3 
        framelength = int(length * sound.samplerate)

        out = cloud.play(length)
        self.assertEqual(len(out), framelength)

        out.write('tests/renders/graincloud_unmodulated.wav')

    def test_pulsed_graincloud(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')
        out = sound.cloud(10, grainlength=0.06, grid=0.12)
        out.write('tests/renders/graincloud_pulsed.wav')

    def test_graincloud_with_length_lfo(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')
        grainlength = dsp.wt('hann', 0.01, 0.1)
        length = 3 
        framelength = int(length * sound.samplerate)

        out = sound.cloud(length, grainlength=grainlength)

        self.assertEqual(len(out), framelength)

        out.write('tests/renders/graincloud_with_length_lfo.wav')

    def test_graincloud_with_speed_lfo(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')
        minspeed = random.triangular(0.05, 1)
        maxspeed = minspeed + random.triangular(0.5, 10)
        speed = dsp.wt('rnd', minspeed, maxspeed)
        cloud = grains.Cloud(sound, grainlength=0.04, speed=speed)

        length = 3
        framelength = int(length * sound.samplerate)

        out = cloud.play(length)
        self.assertEqual(len(out), framelength)

        out.write('tests/renders/graincloud_with_speed_lfo.wav')

    def test_graincloud_with_extreme_speed_lfo(self):
        sound = SoundBuffer(filename='tests/sounds/guitar1s.wav')

        length = 3
        speed = dsp.wt('hann', 1, 100)
        framelength = int(length * sound.samplerate)

        out = sound.cloud(length=length, speed=speed)
        self.assertEqual(len(out), framelength)

        out.write('tests/renders/graincloud_with_extreme_speed_lfo.wav')

    def test_graincloud_with_read_lfo(self):
        sound = SoundBuffer(filename='tests/sounds/linux.wav')
        cloud = grains.Cloud(sound, position=dsp.win('hann', 0, 1))

        length = 3
        framelength = int(length * sound.samplerate)

        out = cloud.play(length)
        self.assertEqual(len(out), framelength)

        out.write('tests/renders/graincloud_with_read_lfo.wav')

    def test_graincloud_grainsize(self):
        snd = dsp.read('tests/sounds/guitar1s.wav')
        out = snd.cloud(
                length=dsp.rand(8, 16), 
                window='hann', 
                grainlength=dsp.win('sinc', 0.2, 6), 
                grid=dsp.win('hannout', 0.04, 1),
                spread=1, 
            )

        out.write('tests/renders/graincloud_grainsize.wav')

