#!/usr/bin/python

import sys
from math import *

a = int(sys.argv[1]) / 360.0 * pi * 2
sources = {
    (cos(a)*sqrt(18), sin(a)*sqrt(18)): lambda t: sin(t*2*pi*800) ,
#    (0,  0): lambda t: sin(t*2*pi*60) * 0.007
}

miclocs = [(0.1, 0.1), (-0.1, 0.1), (0.1, -0.1), (-0.1, -0.1)]

samplerate   = 60000.0
samplenum    = 1024
speedofsound = 340.0
maxval       = 1.0

def dist(a, b):
    return sqrt((a[0] - b[0])**2 + (a[1] - b[1])**2)

def get_val(t, micloc):
    rtn = 0
    for sloc, s in sources.items():
        d = dist(sloc, micloc)
        t -= d / speedofsound
        rtn += s(t) / (d**2)

    return rtn

def get_mic_vals(micloc):
    vals = []
    for i in range(0, samplenum):
        t = i * (1.0/samplerate)
        vals.append(get_val(t, micloc))

    return vals

def get_all_vals():
    mics = [get_mic_vals(loc) for loc in miclocs]
    vals = []
    
    for i in range(0, samplenum):
        for mic in range(len(miclocs)):
            vals.append(mics[mic][i])

    return vals

packet = b'\xff'

for v in get_all_vals():
    v /= maxval / 2.0;
    v += maxval / 2.0;
    v *= 0xff;
    v = min(max(int(v), 0x00), 0xfe)
    packet += chr(v)

open(sys.argv[1] + "deg.pkt", "w").write(packet)
open("test.pkt", "w").write(packet)
