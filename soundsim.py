#!/bin/python

from math import *

sources = {
    (0, 4): lambda t: sin(t*2*pi*500)
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
    vals = []
    for loc in miclocs:
        vals.append(get_mic_vals(loc))
        
    return vals

packet = b'\xff'

for a in get_all_vals():
    for v in a:
        print v
        v /= maxval / 2.0;
        v += maxval / 2.0;
        v *= 0xff;
        v = min(max(int(v), 0x00), 0xff)
        packet += chr(v)
       
open("test.pkt", "w").write(packet)       
print(repr(packet))