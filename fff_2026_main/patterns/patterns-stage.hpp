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

    template <class T> 
    class LFOPattern : public Pattern<RGBA>
    {
        Transition transition;
        std::vector<float> phases;
        std::vector<float> offsets;
        LFOTempo<T> lfo;

    public:
        LFOPattern(std::vector<int> segmentSizes, float duty = 1)
        {
            this->name = "LFO pattern";

            for (size_t i = 0; i < segmentSizes.size(); i++)
            {
                for (int j = 0; j < segmentSizes[i]; j++)
                {
                    phases.push_back((float)j / (segmentSizes[i]-1));
                    offsets.push_back(i%3);
                }
            }
            lfo.setDutyCycle(duty);
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            float offsetSize = params->getOffset(-0.5, 0.5);

            for (int index = 0; index < std::min(width, (int)phases.size()); index++)
            {
                float h = phases[index] + offsets[index] * offsetSize;
                auto color = params->getSecondaryColor();
                pixels[index] = color * lfo.getValue(h) * transition.getValue();
            }
        }
    };
}