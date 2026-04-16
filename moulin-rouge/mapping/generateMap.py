import math
import sys
import os
from math import pi, cos, sin
import json

dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(os.environ['HYPERION_LIB_DIR'], "common/mapping/turtle"))
from turtle3d import *

ledsPerMeter = 60
ledDistance = 1000 / ledsPerMeter #sizes are in mm

scale = 1./4100 #scale to fit in the -1,1 canvas

turtleBase = Turtle3d()
turtleWings = Turtle3d()

def line(turtle,addPositionToTrail=True):
    for i in range(ledsPerMeter):
        turtle.move(ledDistance, addPositionToTrail)

def base(turtle):
    baseRadius = 1200/2
    topRadius = 900/2
    baseAngle = math.degrees(math.acos((baseRadius-topRadius)/2000))
    print ("base angle: " + str(baseAngle) + " diff, " + str(baseRadius-topRadius) + " ")

    numBars=13
    for i in range(numBars):
        turtle.setPosition(0, 0, -3000)
        turtle.setRotation(i*180/(numBars-1),0, 0)
        #radius of the circle
        turtle.move(baseRadius, False)
        turtle.pitch(180-baseAngle)
        line(turtle)
        line(turtle)
        turtle.pitch(baseAngle-90)
        line(turtle)

def wings(turtle):
    for wing in range(4):
        for i in range(5):
            angularsize = 2*13
            localangle = i/4*angularsize-angularsize/2
            angle = 45+wing*90 + localangle
            
            turtle.setPosition(0, -1000, -250)
            turtle.setRotation(0,angle, 0)
            turtle.move(200, False)
            if i%2 == 0:
                line(turtle)
                line(turtle)
            else:
                turtle.move(1000, False)
                line(turtle)

base(turtleBase)
wings(turtleWings)

def zigZag(array, segmentSize):
    result = []
    for i in range(0, len(array), segmentSize):
        segment = array[i:i+segmentSize]
        if (i//segmentSize) % 2 == 1:
            segment.reverse()
        result.extend(segment)
    return result

def reverseBetween(array, start, size):
    result = array[:]
    end = start + size
    result[start:end] = reversed(result[start:end])
    return result

def wingsZigZag(array):
    for w in range(4):
        wstart = w*8*ledsPerMeter
        array = reverseBetween(array, wstart + 2*ledsPerMeter, ledsPerMeter)
        array = reverseBetween(array, wstart + 3*ledsPerMeter, 2*ledsPerMeter)

        array = reverseBetween(array, wstart + 5*ledsPerMeter, 3*ledsPerMeter)
        array = reverseBetween(array, wstart + 5*ledsPerMeter, 2*ledsPerMeter)
    return array

def writePoints(name, points, scale=1.):
    f.write("PixelMap3d " + name + " = {\n")
    for point in points:
        f.write("    {.x = " + str(point['x'] * scale) +
                ", .y = " + str(point['z'] * scale) +
                ", .z = " + str(point['y'] * scale) + "},\n")
    f.write("};\n\n")

print ("Base map has " + str(len(turtleBase.trail)) + " points")
print ("Wings map has " + str(len(turtleWings.trail)) + " points")

f = open(os.path.join(dir, "moulin-rouge-map.hpp"), "w")
writePoints("baseMap", zigZag(turtleBase.trail, 3*60), scale)
writePoints("wingsMap", wingsZigZag(turtleWings.trail), scale)
writePoints("totalMap", turtleBase.trail + turtleWings.trail, scale)
f.close()
