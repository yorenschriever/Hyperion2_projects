#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace StagePatterns
{

    class StaticGradientPattern : public Pattern<RGBA>
    {
        Transition transition;
        std::vector<float> phases;

    public:
        StaticGradientPattern(std::vector<int> segmentSizes)
        {
            this->name = "Static gradient";

            for (size_t i = 0; i < segmentSizes.size(); i++)
            {
                for (int j = 0; j < segmentSizes[i]; j++)
                    phases.push_back((float)j / (segmentSizes[i]-1));
            }
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            float height = params->getSize(0.2,1);

            for (int index = 0; index < std::min(width, (int)phases.size()); index++)
            {
                float h = phases[index];
                RGBA color = params->getGradientf(h);
                pixels[index] = color * h * transition.getValue();
            }
        }
    };

    class StaticGradientTripletPattern : public Pattern<RGBA>
    {
        Transition transition;
        std::vector<float> phases;

    public:
        StaticGradientTripletPattern(std::vector<int> segmentSizes)
        {
            this->name = "Static gradient triplet";

            for (size_t i = 0; i < segmentSizes.size(); i++)
            {
                for (int j = 0; j < segmentSizes[i]; j++)
                    phases.push_back((float)(i%3+1) / 5);
            }
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            float height = params->getSize(0.2,1);

            for (int index = 0; index < std::min(width, (int)phases.size()); index++)
            {
                float h = phases[index];
                RGBA color = params->getGradientf(h);
                pixels[index] = color * transition.getValue();
            }
        }
    };

}