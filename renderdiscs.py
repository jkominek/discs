#!/usr/bin/python

import sys
from math import *
import Image, ImageDraw

def quadbezier(a,b,c,t):
    def f(a,b,c):
        return (1-t)*((1-t)*a + t*b) + t*((1-t)*b + t*c)
    return map(f, a, b, c)

a, b, c, = [], [], []
for l in sys.stdin.readlines():
    vs = map(float, l.split())
    a.append(vs[0:2])
    b.append(vs[2:4])
    c.append(vs[4:6])
starts, mids, stops = a, b, c

radius, LB, UB, SCALING = map(float, sys.argv[1:])

for idx in range(0, 201):
    im = Image.new("RGB", map(int, (SCALING*(UB-LB), SCALING*(UB-LB))), "white")
    draw = ImageDraw.Draw(im)
    t = idx / 200.0
    for i in range(0, len(starts)):
        start = starts[i]
        mid = mids[i]
        stop = stops[i]

        (x,y) = quadbezier(start, mid, stop, t)
        draw.ellipse( map(lambda x: SCALING*x, (x-radius-LB, y-radius-LB, x+radius-LB, y+radius-LB)), 'red' )

    del draw
    im.save("%03i.png" % (idx,))
