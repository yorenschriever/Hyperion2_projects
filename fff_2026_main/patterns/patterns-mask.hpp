#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace MaskPatterns
{

    class AerialTipsMaskPattern : public Pattern<RGBA>
    {
        Transition transition;

    public:
        AerialTipsMaskPattern()
        {
            this->name = "Aerial tips mask";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            for (int index = 0; index < width; index++)
            {
                int posMod = index % 60;
                float distanceFromTip = 1.-std::abs(29.5 - posMod)/30.;

                distanceFromTip = Utils::rescale_c(distanceFromTip, 0, 1, 0, 0.6);

                RGBA color = RGBA(0,0,0,255);
                pixels[index] = color * distanceFromTip * transition.getValue();
            }
        }
    };

    class DomeTipsMaskPattern : public Pattern<RGBA>
    {
        Transition transition;

    public:
        DomeTipsMaskPattern()
        {
            this->name = "Dome tips mask";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            for (int index = 0; index < width; index++)
            {
                int posMod = index % 120;
                float distanceFromTip = std::abs(59.5 - posMod)/60.;

                // distanceFromTip = Utils::rescale_c(distanceFromTip, 0, 1, 0, 0.6);

                RGBA color = RGBA(0,0,0,255);
                pixels[index] = color * distanceFromTip * transition.getValue();
            }
        }
    };

}