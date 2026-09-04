#pragma once

#include "hyperion.hpp"
#include "mapConsts.hpp"

void moveToObeliskCenter(Turtle3d &turtle, float angle)
{
    turtle.setPosition(0, 0, 3000);
    turtle.setRotation(angle+90, 0, 0);

    turtle.move(obeliskRadius, false);
}

void drawTriangle(Turtle3d &turtle)
{
    turtle.pitch(90);

    // correct center point
    // move to the side along the base of the triangle
    turtle.yaw(-90);
    turtle.move(500, false);
    turtle.yaw(90);
    // move down a little
    turtle.pitch(-90);
    float triangleHeight = 0.5 * 1000 * sqrt(3);
    turtle.move(0.5 * triangleHeight, false);
    turtle.pitch(90);

    turtle.yaw(90);

    for(int i = 0; i < 3; i++)
    {
        drawLedBar(turtle,60);
        turtle.pitch(120);
    }
}

void drawObelisk(Turtle3d &turtle, float angle)
{
    float obeliskLegAngle = asin(1.0/3.0) * 360 / (2 * M_PI);
    for (int i=0;i<3;i++)
    {
        moveToObeliskCenter(turtle, angle);
        turtle.yaw( i * 120);
        turtle.pitch(-90+obeliskLegAngle);
        turtle.move(250, false);
        
        bool reverse = false; //i == 0 || i == 2; // middle leg is reversed
        if (reverse)
            drawReverseLedBar(turtle, 120);
        else
            drawLedBar(turtle, 120);
    }
}

void drawAerial(Turtle3d &turtle, float angle)
{
    moveToObeliskCenter(turtle, angle);
    drawTriangle(turtle);

    moveToObeliskCenter(turtle, angle);
    turtle.pitch(-90);
    turtle.move(barSpacing, false);
    turtle.pitch(90);
    drawTriangle(turtle);
}

PixelMap3dPtr createObeliskMap()
{
    Turtle3d turtle;

    drawObelisk(turtle, 360/5 * 0);
    drawObelisk(turtle, 360/5 * 1);
    drawObelisk(turtle, 360/5 * 2);
    drawObelisk(turtle, 360/5 * 3);
    drawObelisk(turtle, 360/5 * 4);

    turtle.scale(scale);

    return std::make_shared<Turtle3d>(turtle);
}

PixelMap3dPtr createObeliskAerialMap()
{
    Turtle3d turtle;

    drawAerial(turtle, 360/5 * 0);
    drawAerial(turtle, 360/5 * 1);
    drawAerial(turtle, 360/5 * 2);
    drawAerial(turtle, 360/5 * 3);
    drawAerial(turtle, 360/5 * 4);

    turtle.scale(scale);

    return std::make_shared<Turtle3d>(turtle);
}