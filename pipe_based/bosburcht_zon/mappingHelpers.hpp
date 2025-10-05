#pragma once
#include <math.h>
#include "generation/pixelMap.hpp"
#include "utils.hpp"

float around(float th){
    return (th + M_PI)/(2*M_PI);
}

inline float softEdge(float dist, float size, float edgeSize = 0.03)
{
    if (dist > size) return 0;
    if (dist < size - edgeSize) return 1;
    return (size-dist) / edgeSize;
}