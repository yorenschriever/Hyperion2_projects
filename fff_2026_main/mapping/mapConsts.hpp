#pragma once
#include "hyperion.hpp"

const int ledsPerMeter = 60;
const float ledDistance = 1000.0 / ledsPerMeter; // sizes are in mm
const float barSpacing = 150.0; // mm. horizontal spacing, so not right-angle distance.

const float domeRadius = 4000;
const float domeCenterZ = 1500;
const float obeliskRadius = 3000;

const float scale = 1.0 / 4100; // scale to fit in the -1,1 canvas

void drawLedBar(Turtle3d &turtle, int size)
{
    turtle.addPositionToTrail();
    for(int i = 0; i < size-1; i++)
        turtle.move(ledDistance);
}

void drawReverseLedBar(Turtle3d &turtle, int size)
{
    turtle.move(ledDistance * (size-1));
    for(int i = 0; i < size-1; i++)
        turtle.move(-ledDistance);
}

PixelMap3d normalizeStage(PixelMap3d map)
{
    PixelMap3d result = map;
    for(auto &led : result)
    {
        led.x *= 0.5;
        led.y *= 0.5;
        led.z *= 0.5;
    }
    return result;
}