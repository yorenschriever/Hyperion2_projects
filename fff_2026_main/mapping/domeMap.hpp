#pragma once

#include "hyperion.hpp"
#include "mapConsts.hpp"

std::vector<std::array<int, 2>> triangles = {
    {25,  -1*55},
    {40,  -1*60},
    {25,  -1*-25},
    {40,  -1*-20},
    {32,  -1*30},
    {45,  -1*5},

    {25,  -1*100},
    {40,  -1*95},
    {32,  -1*125},
    {5,   -1*180},
    {15,  -1*-55},
    {35,  -1*-50},

    {25,  -1*-75},
    {65,  -1*-85},
    {45,  -1*-120},

    {90, 0}
};

void drawLedBar(Turtle3d &turtle, int size)
{
    turtle.addPositionToTrail();
    for(int i = 0; i < size-1; i++)
        turtle.move(ledDistance * scale);
}

void drawTriangle(Turtle3d &turtle, float pitch, float yaw)
{
    turtle.setPosition(0, 0, 0);
    turtle.setRotation(yaw, pitch, 0);
    turtle.move(domeRadius * scale, false);

    // correct center point
    // move to the side along the base of the triangle
    turtle.yaw(-90);
    turtle.move(500 * scale, false);
    turtle.yaw(90);
    // move down a little
    turtle.pitch(-90);
    float triangleHeight = 0.5 * 1000 * sqrt(3);
    turtle.move(0.5 * triangleHeight * scale, false);
    turtle.pitch(90);

    turtle.yaw(90);

    for(int i = 0; i < 3; i++)
    {
        drawLedBar(turtle,60);
        turtle.pitch(120);
    }
}

void drawDoubleArrow(Turtle3d &turtle, float pitch, float yaw)
{
    turtle.setPosition(0, 0, 0);
    turtle.setRotation(yaw, pitch, 0);
    turtle.move(domeRadius * scale, false);

    // correct center point
    // move to the side along the base of the triangle
    turtle.yaw(-90);
    turtle.move(500 * scale, false);
    turtle.yaw(90);
    // move down a little
    turtle.pitch(-90);
    float triangleHeight = 0.5 * 1000 * sqrt(3);
    turtle.move(0.5 * triangleHeight * scale, false);
    turtle.pitch(90);

    turtle.yaw(90);
    turtle.pitch(120);

    drawLedBar(turtle,60);
    turtle.pitch(120);
    drawLedBar(turtle,60);
    turtle.pitch(-60);
    turtle.move(barSpacing * scale, false);
    turtle.pitch(-120);
    drawLedBar(turtle,60);
    turtle.pitch(-60);
    turtle.move(2*barSpacing * scale, false);
    turtle.pitch(-60);
    drawLedBar(turtle,60);

    // turtle.pitch(120);
    // drawLedBar(turtle,60);   

    // for(int i = 0; i < 3; i++)
    // {
    //     drawLedBar(turtle,60);
    //     turtle.pitch(120);
    // }
}

PixelMap3d createDomeMap()
{
    Turtle3d turtle;

    for(auto triangle : triangles)
    {
        // drawTriangle(turtle, triangle[0], triangle[1]);
        drawDoubleArrow(turtle, triangle[0], triangle[1]);
    }

    return turtle;
}