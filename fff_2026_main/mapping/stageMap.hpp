#pragma once

#include "hyperion.hpp"
#include "mapConsts.hpp"

void drawTripleLedBar(Turtle3d &turtle, float x, float y, float z, int size, float mirror = 1)
{
    int startAngle = mirror == 1 ? 60 : 120;

    // turtle.setDirection(-60. + angleOffset);
    // turtle.setRotation(-startAngle, 90, 90);
    turtle.setRotation(0,startAngle,90);
    turtle.setPosition((-x - ledDistance * size / 2 - 0 * barSpacing) * mirror, y, z);

    drawLedBar(turtle, size);
    turtle.yaw(mirror * 120);
    turtle.move(barSpacing, false);
    turtle.yaw(mirror * 60);
    drawLedBar(turtle, size);
    turtle.yaw(mirror * -60);
    turtle.move(barSpacing, false);
    turtle.yaw(mirror * -120);
    drawLedBar(turtle, size);
}

PixelMap3dPtr createStageMap()
{
    Turtle3d turtle;

    // dak. kleine punt om oog
    drawTripleLedBar(turtle, 0,    domeRadius, 3000, 60, 1);
    drawTripleLedBar(turtle, 0,    domeRadius, 3000, 60, -1);

    // dak. grote punt
    drawTripleLedBar(turtle, 500,  domeRadius + 1000, 3000, 120, 1);
    drawTripleLedBar(turtle, 500,  domeRadius + 1000, 3000, 120, -1);

    // backdrop. kleine punt
    drawTripleLedBar(turtle, 0,    domeRadius + 2200 + 3000, 1000, 60, 1);
    drawTripleLedBar(turtle, 0,    domeRadius + 2200 + 3000, 1000, 60, -1);

    // backdrop. grote punt
    drawTripleLedBar(turtle, 0,    domeRadius + 2200, 1000, 120, 1);
    drawTripleLedBar(turtle, 0,    domeRadius + 2200, 1000, 120, -1);

    // voorkant. kleine punt
    drawTripleLedBar(turtle, 2000, domeRadius, 1000, 60, 1);
    drawTripleLedBar(turtle, 2000, domeRadius, 1000, 60, -1);

    turtle.scale(scale);

    return std::make_shared<Turtle3d>(turtle);
}

std::vector<int> stageMapSegmentSizes = {
    60, 60,60, 60,60, 60, // dak. kleine punt
    120, 120,120, 120,120, 120, // dak. grote punt
    60, 60, 60, 60,60, 60,// backdrop. kleine punt
    120, 120,120, 120,120, 120, // backdrop. grote punt
    60, 60,60, 60,60, 60 // voorkant. kleine punt
};