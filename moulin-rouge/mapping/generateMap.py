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
    numBars=24
    for i in range(numBars):
        turtle.setPosition(0, 1000, -3000)
        turtle.setRotation(i*360/numBars,0, 0)
        #radius of the circle
        turtle.move(1000, False)
        turtle.pitch(100)
        line(turtle)
        line(turtle)
        turtle.pitch(-10)
        line(turtle)

def wings(turtle):
    for wing in range(4):
        for i in range(5):
            angularsize = 30
            localangle = i/5*angularsize-angularsize/2
            angle = 45+wing*90 + localangle
            
            turtle.setPosition(0, 0, -250)
            turtle.setRotation(0,angle, 0)
            turtle.move(250, False)
            if i%2 == 0:
                line(turtle)
                line(turtle)
            else:
                turtle.move(1000, False)
                line(turtle)

base(turtleBase)
wings(turtleWings)

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
writePoints("baseMap", turtleBase.trail, scale)
writePoints("wingsMap", turtleWings.trail, scale)
writePoints("totalMap", turtleBase.trail + turtleWings.trail, scale)
f.close()
