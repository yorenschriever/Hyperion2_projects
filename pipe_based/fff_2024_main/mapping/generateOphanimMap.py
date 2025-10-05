from turtle import *
from math import pi, cos, sin
import os
import json

def drawCircle(amount, radius, xc=0, yc=0):
    result = []
    for i in range(amount):
        x = xc + radius * cos(float(i)/amount * 2 * pi)
        y = yc + radius * sin(float(i)/amount * 2 * pi)
        result.append({'x': x, 'y': y})
    return result

def drawHalfCircle(amount, radius, half, xc=0, yc=0):
    result = []
    for i in range(amount):
        x = xc + radius * cos(float(i)/amount  * pi)
        y = yc + radius * sin(float(i)/amount  * pi) * half
        result.append({'x': x, 'y': y})
    return result

# Ring 1, buitenste
ring1 = []
# ring1.extend(drawHalfCircle(152, 0.95,1))
# ring1.extend(drawHalfCircle(152, 0.95,-1))
ring1.extend(drawCircle(2*152, 0.95))
# ring1.extend(drawCircle(500, 0.85))

# Ring 2
ring2 = drawCircle(24, 0.70)

# Ring 3, binnenste
ring3 = []
# ring3.extend(drawCircle(500, 0.55))
# ring3.extend(drawHalfCircle(127, 0.45,1))
# ring3.extend(drawHalfCircle(127, 0.45,-1))
ring3.extend(drawCircle(2*127, 0.45))


def writePoints(name, points, scale=1.):
    f.write("PixelMap " + name + " = {\n")
    for point in points:
        f.write("    {.x = " + str(point['x'] * scale) +
                ", .y = " + str(point['y'] * scale) + "},\n")
    f.write("};\n\n")


dir = os.path.dirname(os.path.realpath(__file__))
f = open(os.path.join(dir, "ophanimMap.hpp"), "w")
writePoints("ring1Map", ring1)
writePoints("ring2Map", ring2)
writePoints("ring3Map", ring3)
f.close()

points = list(map(lambda p: {'x': p['x'] ,'y': p['y'] }, ring1))
with open(os.path.join(dir, "ophanimRing1Map.json"), "w") as outfile:
    json.dump(points, outfile)

points = list(map(lambda p: {'x': p['x'] ,'y': p['y'] }, ring3))
with open(os.path.join(dir, "ophanimRing3Map.json"), "w") as outfile:
    json.dump(points, outfile)

#export to image for video mask

import cv2
import numpy as np

width = 500
height = 500

# Make empty black image 
img = np.zeros((height, width, 3), np.uint8)

white = [255,255,255]
radius = 5

def writePointsToImg(points):
    for point in points:
        coord = (
            int((point['x']+1) * width/2),
            int((point['y']+1) * height/2),
        )
        if (coord[0] < 0 or coord[0] >= width):
            continue
        if (coord[1] < 0 or coord[1] >= height):
            continue
        print(coord)
        cv2.circle(img, coord, radius, white, -1)

writePointsToImg(ring1)
writePointsToImg(ring2)
writePointsToImg(ring3)

cv2.imwrite(os.path.join(dir,"ophanimMap.png"), img)
cv2.imshow('img', img)

cv2.waitKey(0)