#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include <math.h>
#include <vector>

namespace MaskPatterns
{

class WingsFadePattern : public Pattern<RGBA>
{
public:
    RGBA color = RGBA(0,0,0,255);
    PixelMap::Polar *map;
    bool invert;
    WingsFadePattern(PixelMap::Polar *map, bool invert=false)
    {
        this->map = map;
        this->invert = invert;
        this->name = "Wings radial";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < width; i++)
        {
            float distance = map->r(i);
            float intensity = invert ? distance : 1 - distance;
            pixels[i] = color * intensity; //fade out based on distance to create a nice depth effect
        }
    }
};

}