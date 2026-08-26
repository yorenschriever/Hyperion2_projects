#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace AllPatterns
{

 
    class TopChase : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFO<SawDown> lfo;
        PixelMap3d::Cylindrical *map;

    public:
        TopChase(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Top chase";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            int amount = params->getAmount(1, 12);
            lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize(0.06, 0.5));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                float lfoVal = lfo.getValue(-amount * around(map->th(index)));
                RGBA color = params->getGradient(lfoVal * 255);
                pixels[index] = color * lfoVal * fromTop(map->operator[](index).z) * transition.getValue();
            }
        }
    };




}