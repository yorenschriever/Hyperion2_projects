#pragma once
#include "hyperion.hpp"
#include <math.h>
#include <vector>

class GlowPulsePattern : public Pattern<RGBA>
{
    Permute perm;
    LFO<SinFast> lfo = LFO<SinFast>(10000);
    Transition transition;

public:
    GlowPulsePattern()
    {
        this->name = "Glow pulse";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return; // the fade out is done. we can skip calculating pattern data

        lfo.setPeriod(500 + 10000 * (1.0f - params->getVelocity()));
        perm.setSize(width);

        for (int index = 0; index < width; index++)
        {
            float val = 1.025 * lfo.getValue(float(Transition::fromCenter(index, width, 1000)) / -1000);
            if (val < 1.0)
                pixels[perm.at[index]] = params->getPrimaryColor() * lfo.getValue(float(Transition::fromCenter(index, width, 1000)) / -1000);
            else
                pixels[perm.at[index]] = params->getPrimaryColor() + (params->getSecondaryColor() * (val - 1) / 0.025);
        }
    }
};

class DitheredGradientPattern : public Pattern<RGBA>
{
   Transition transition = Transition(
        200, Transition::none, 0,
        1000, Transition::none, 0);
    std::vector<float> noise;

public:
    DitheredGradientPattern()
    {
        this->name="Dithered Gradient";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (noise.size() != width)
        {
            noise.clear();
            for(int i = 0; i < width; i++)
                noise.push_back((Utils::random_f() - 0.0) * 1.0);
        }

        for (int index = 0; index < width; index++)
        {
            RGBA color = (index - width*noise[index] > 0.5) ? params->getSecondaryColor() : params->getPrimaryColor();
            RGBA dimmedColor = color * transition.getValue();
            pixels[index] = dimmedColor;
        }
    }
};

class GradientPattern : public Pattern<RGBA>
{
   Transition transition = Transition(
        200, Transition::none, 0,
        1000, Transition::none, 0);

public:
    GradientPattern()
    {
        this->name="Gradient";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int index = 0; index < width; index++)
        {
            RGBA color = params->getPrimaryColor() + params->getSecondaryColor()* ((float)index / width);
            RGBA dimmedColor = color * transition.getValue();
            pixels[index] = dimmedColor;
        }
    }
};

class HorizontalDitheredGradientPattern : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition = Transition(
        200, Transition::none, 0,
        1000, Transition::none, 0);
    std::vector<float> noise;
    const size_t maxNoiseMapSize = 100;

public:
    HorizontalDitheredGradientPattern(PixelMap *map)
    {
        this->map = map;
        for (int i = 0; i < std::min(map->size(), maxNoiseMapSize); i++)
        {
            noise.push_back((Utils::random_f() - 0.5) * 1.3);
        }
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int index = 0; index < std::min(width, (int)map->size()); index++)
        {
            RGBA color = (map->operator[](index).x + noise[index % maxNoiseMapSize] < 0) ? params->getSecondaryColor() : params->getPrimaryColor();
            RGBA dimmedColor = color * transition.getValue();
            pixels[index] += dimmedColor;
        }
    }
};