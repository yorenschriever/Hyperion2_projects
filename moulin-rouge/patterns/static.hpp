#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include <math.h>
#include <vector>

namespace Static
{

    class AngularSweep : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<Glow> lfo;
        PixelMap::Polar *map;

    public:
        AngularSweep(PixelMap::Polar *map)
        {
            this->map = map;
            this->name = "Angular sweep";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            // lfo.setPeriod(params->getVelocity(11000, 500));
            lfo.setDutyCycle(params->getSize(0.1, 0.8));
            int amount = params->getAmount(1, 7.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getGradient(fromTop(map->r(index)) * 255);
                pixels[index] = color * lfo.getValue(amount * around(map->th(index))) * transition.getValue();
            }
        }
    };

    class RadialSaw : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<SawUp> lfo;
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
                float lfoArg = fromTop(map->r(index));
                pixels[index] = color * lfo.getValue(lfoArg) * transition.getValue();
            }
        }
    };

    class HorizontalWave : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFOTempo<Glow> lfo;
        PixelMap *map;

    public:
        HorizontalWave(PixelMap *map)
        {
            this->map = map;
            this->name = "Horizontal wave";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setDutyCycle(params->getSize(0.06, 1));
            int amount = params->getAmount(1, 7.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getPrimaryColor();
                float lfoArg = map->y(index) * amount;
                pixels[index] = color * (lfo.getValue(lfoArg)) * transition.getValue();
            }
        }
    };

}