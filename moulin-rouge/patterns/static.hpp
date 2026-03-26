#pragma once
#include "core/generation/patterns/pattern.hpp"
#include "common/patterns/pattern-helpers.hpp"
#include <math.h>
#include <vector>

namespace Static {

    class HorizontalSin : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<Glow> lfo;
        PixelMap::Polar *map;

    public:
        HorizontalSin(PixelMap::Polar *map)
        {
            this->map = map;
            this->name = "Horizontal sin";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            // lfo.setPeriod(params->getVelocity(11000, 500));
            lfo.setDutyCycle(params->getSize(0.03, 0.5));
            int amount = params->getAmount(1, 7.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getGradient(fromTop(map->r(index)) * 255);
                pixels[index] = color * lfo.getValue(amount * around(map->th(index))) * transition.getValue();
            }
        }
    };

    class HorizontalSaw : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<SawDown> lfo;
        PixelMap::Polar *map;

    public:
        HorizontalSaw(PixelMap::Polar *map)
        {
            this->map = map;
            this->name = "Horzontal saw";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            // lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize(0.06, 1));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getPrimaryColor();
                float lfoArg = fromTop(map->r(index));
                pixels[index] = color * lfo.getValue(lfoArg) * transition.getValue();
            }
        }
    };

    class RadialSaw : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<SawDown> lfo;
        PixelMap::Polar *map;

    public:
        RadialSaw(PixelMap::Polar *map)
        {
            this->map = map;
            this->name = "Radial saw";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            // lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize(0.06, 1));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getPrimaryColor();
                float lfoArg = around(map->th(index));
                pixels[index] = color * lfo.getValue(lfoArg) * transition.getValue();
            }
        }
    };

}