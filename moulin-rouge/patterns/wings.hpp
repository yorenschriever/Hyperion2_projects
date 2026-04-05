#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include <math.h>
#include <vector>

namespace Wings
{



class GrowShrink : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFO<SinFast> lfo;
        PixelMap::Polar *map;

    public:
        GrowShrink(PixelMap::Polar *map)
        {
            this->map = map;
            this->name = "Grow shrink";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(5000, 500));
            float size = params->getSize(0.1, 1.5);
            int offset = params->getOffset(0, 3.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                float lfoSize = lfo.getValue(Utils::modulus_f(offset * around(map->th(index)))) * size;
                float distance = abs(map->r(index) + 0.07);
                if (distance > lfoSize)
                    continue;

                float distanceAsRatio = 1 - distance / lfoSize;

                pixels[index] = params->getGradient(distanceAsRatio * 255) * distanceAsRatio * transition.getValue();
            }
        }
    };


class WingCycleFlashesPattern : public Pattern<RGBA>
    {
        FadeDown fade = FadeDown(2400);
        BeatWatcher watcher = BeatWatcher();
        int wingIndex=0;

    public:
        WingCycleFlashesPattern()
        {
            this->name = "Wing cycle flashes";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!active)
                return;
            fade.duration = params->getVelocity(500, 25);

            if (watcher.Triggered()){
                fade.reset();
                wingIndex = (wingIndex + 1) % 4;
            }

            RGBA color;
            float val = fade.getValue();
            if (val >= 0.5)
                color = params->getSecondaryColor() + RGBA(255, 255, 255, 255) * ((val - 0.5) * 2.);
            else
                color = params->getSecondaryColor() * (val * 2.);

            for (int index = 0; index < width/4; index++)
                pixels[index + wingIndex * width/4] = color;
        }
    };


    class SegmentGlitchPattern : public Pattern<RGBA>
    {
        // Timeline timeline = Timeline(50);
        Permute perm = Permute(1);
        Transition transition;
        int segmentSize ;
        int currentSegment = 0;

    public:
        SegmentGlitchPattern(int segmentSize= 60)
        {
            this->name = "Segment glitch";
            this->segmentSize = segmentSize;

        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            int numSegments = width / segmentSize;

            currentSegment = (currentSegment + Utils::random(1,numSegments-1)) % numSegments;

            int start = currentSegment * segmentSize;
            for (int i=start; i< std::min(width, start + segmentSize); i++){
                pixels[i] += params->getHighlightColor() * transition.getValue();
            }

            // // int numSegmentsPerBar = 30 - params->getSize(29, 15);
            // // int numSegments = (width / 60) * numSegmentsPerBar;
            // int numSegments = width / segmentSize;
            // int threshold = params->getAmount(0, numSegments / 8);
            // perm.setSize(numSegments);
            // perm.permute();

            // for (int index = 0; index < width; index++)
            // {
            //     int segmentIndex = index * numSegments / width;

            //     if (perm.at[segmentIndex] > threshold)
            //         continue;
            //     pixels[index] += params->getHighlightColor() * transition.getValue();
            // }
        }
    };

    class WingsBeamsPattern : public Pattern<RGBA>
    {
        Transition transition;
        PixelMap::Polar *map;   

    public:
        WingsBeamsPattern(PixelMap::Polar *map)
        {
            this->name = "Wings beams";
            this->map = map;    
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            // float intensity = params->getIntensity(0.4, 1.0);
            // float size = params->getSize(0.1, 0.5);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                // float angleModulus = Utils::modulus_f(map->th(index), M_PI / 2) - M_PI/4;
                float angleDeg = map->th(index) * 180. / M_PI;
                int angleModulus = Utils::mod(int(angleDeg), 0,90) - 45;

                bool isInAngularBeam = angleModulus < -10 || angleModulus > 7;
                bool isInRadialBeam = map->r(index) > 0.76 || (map->r(index) > 0.45 && map->r(index) < 0.5);

                if (!isInAngularBeam && !isInRadialBeam)
                    continue;

                // if (isInAngularBeam)
                    // pixels[index] += params->getHighlightColor() * transition.getValue();
                // if (isInRadialBeam)
                    pixels[index] += params->getSecondaryColor() * transition.getValue();

            }
        }
    };

}