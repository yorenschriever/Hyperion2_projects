#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace AerialPatterns
{
    class DoubleFlash : public Pattern<RGBA>
    {
        Timeline timeline = Timeline();
        BeatWatcher watcher;

    public:
        DoubleFlash()
        {
            this->name = "Double flash";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!active) 
                return;

            timeline.FrameStart();

            int beatDiv = params->getAmount(0,4);
            if (beatDiv >3) beatDiv =3;
            int divs[] = {8,4,2,1};

            if (watcher.Triggered() && Tempo::GetBeatNumber() % divs[beatDiv] == 0)
                timeline.reset();

            if (!timeline.Happened(0) && !timeline.Happened(100))
                return;

            for (int i = 0; i < width; i++)
            {
                pixels[i] = params->getHighlightColor();
            }
        }
    };

class GradientChasePattern : public Pattern<RGBA>
    {
        Transition transition;
        LFO<SawDown> lfo = LFO<SawDown>(5000);

    public:
        GradientChasePattern()
        {
            this->name = "Gradient chase";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            float velocity = params->getVelocity(6000, 500);
            int amount = 10 * int(params->getAmount(1, 2.99));
            float size = params->getSize();

            lfo.setPeriod(velocity / size * amount / 30);
            lfo.setDutyCycle(size);

            for (int i = 0; i < width; i++)
            {
                float lfoVal = lfo.getValue(amount * float(i) / width);
                pixels[i] = params->getGradient(lfoVal * 255) * lfoVal * transition.getValue();
            }
        }
    };


    class SinChasePattern : public Pattern<RGBA>
    {
        Transition transition;
        LFO<Glow> lfo;
        int segmentSize = 60;

    public:
        SinChasePattern(int segmentSize = 60)
        {
            this->segmentSize = segmentSize;
            this->name = "Sin chase";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setDutyCycle(params->getSize(0.1, 1));
            lfo.setPeriod(params->getVelocity(4000, 500));
            int amount = 10 * int(params->getAmount(1, 5));

            for (int i = 0; i < width; i++)
            {
                float phase = ((float)i / width) * amount;
                pixels[i] = params->getSecondaryColor() * lfo.getValue(phase) * transition.getValue();
            }
        }
    };

}