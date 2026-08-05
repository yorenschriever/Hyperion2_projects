#pragma once

#include "hyperion.hpp"

const int ledsPerMeter = 60;
const float ledDistance = 1000.0 / ledsPerMeter; // sizes are in mm
const float barSpacing = 150.0; // mm. horizontal spacing, so not right-angle distance.

const float scale = 1.0 / 4100; // scale to fit in the -1,1 canvas

void drawLedBar(Turtle &turtle, int size)
{
    turtle.addPositionToTrail();
    for(int i = 0; i < size-1; i++)
        turtle.move(ledDistance * scale);
}

void drawTripleLedBar(Turtle &turtle, float x, float y, int size, float mirror=1)
{
    int angleOffset = mirror == 1 ? 0 : -60;

    turtle.setDirection(-60. + angleOffset);
    turtle.setPosition((-x - ledDistance*size/2 - 0*barSpacing) * scale * mirror, y * scale);

    drawLedBar(turtle, size);
    turtle.turn(mirror *  -120);
    turtle.move(barSpacing * scale, false);
    turtle.turn(mirror *  -60);
    drawLedBar(turtle, size);
    turtle.turn(mirror *  60);
    turtle.move(barSpacing * scale, false);
    turtle.turn(mirror *  120);
    drawLedBar(turtle, size);
}

PixelMap createStageMap()
{
    Turtle turtle;

    // dak. kleine punt om oog
    drawTripleLedBar(turtle, 0, 0, 60, 1);
    drawTripleLedBar(turtle, 0, 0, 60, -1);

    // dak. grote punt
    drawTripleLedBar(turtle, 500, 0, 120, 1);
    drawTripleLedBar(turtle, 500, 0, 120, -1);


    // backdrop. kleine punt 
    drawTripleLedBar(turtle, 0, 2000, 60, 1);
    drawTripleLedBar(turtle, 0, 2000, 60, -1);

    // backdrop. grote punt
    drawTripleLedBar(turtle, 0, 2000, 120, 1);
    drawTripleLedBar(turtle, 0, 2000, 120, -1);

    // voorkant. kleine punt 
    drawTripleLedBar(turtle, 2000, 2000, 60, 1);
    drawTripleLedBar(turtle, 2000, 2000, 60, -1);

    return turtle;
}