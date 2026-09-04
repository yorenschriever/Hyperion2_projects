#pragma once

#include "hyperion.hpp"
#include "mapConsts.hpp"

std::vector<std::array<int, 2>> triangles = {

    // {25,  0*360/7},
    {20,  1*360/6},
    {25,  2*360/6},
    {25,  3*360/6},
    {25,  4*360/6},
    {25,  5*360/6},
    // {25,  6*360/6},

    {50,  1*360/5},
    {50,  2*360/5},
    {50,  3*360/5},
    {50,  4*360/5},
    // {50,  5*360/5},

    {90,0}

    // {25,  -1*55},
    // // {40,  -1*60},
    // {25,  -1*-25},
    // {40,  -1*-20},
    // // {32,  -1*30},
    // {45,  -1*5},

    // {25,  -1*100},
    // {40,  -1*95},
    // // {32,  -1*125},
    // {5,   -1*180},
    // {15,  -1*-55},
    // {35,  -1*-50},

    // {25,  -1*-75},
    // // {65,  -1*-85},
    // {45,  -1*-120},

    // {90, 0}
};


void drawDoubleArrow(Turtle3d &turtle, float pitch, float yaw)
{
    turtle.setPosition(0, 0, domeCenterZ);
    turtle.setRotation(yaw-90, pitch, 0);
    turtle.move(domeRadius, false);

    // correct center point
    // move to the side along the base of the triangle
    turtle.yaw(-90);
    turtle.move(-500, false);
    turtle.yaw(90);
    // move down a little
    turtle.pitch(-90);
    float triangleHeight = 0.5 * 1000 * sqrt(3);
    turtle.move(0.5 * triangleHeight, false);
    turtle.pitch(90);

    turtle.yaw(90);
    turtle.pitch(120);

    //draw the double arrow
    drawLedBar(turtle,60);
    turtle.pitch(120);
    drawLedBar(turtle,60);
    turtle.pitch(-60);
    turtle.move(barSpacing, false);
    turtle.pitch(-120);
    drawLedBar(turtle,60);
    turtle.pitch(-60);
    turtle.move(2*barSpacing, false);
    turtle.pitch(-60);
    drawLedBar(turtle,60);
}

PixelMap3dPtr createDomeMap()
{
    Turtle3d turtle;

    for(auto triangle : triangles)
    {
        drawDoubleArrow(turtle, triangle[0], triangle[1]);
    }

    turtle.scale(scale);

    return std::make_shared<Turtle3d>(turtle);
}