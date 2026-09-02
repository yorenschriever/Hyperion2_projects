#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace ObeliskPatterns
{

class HorizontalSin : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFO<Glow> lfo;
        PixelMap3d::Cylindrical *map;

    public:
        HorizontalSin(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Horizontal sin";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(11000,500));
            lfo.setDutyCycle(params->getSize(0.03,0.5));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                // RGBA color = params->getPrimaryColor(); 
                RGBA color = params->getGradient(fromTop(map->z(index))*255); 
                pixels[index] = color * lfo.getValue(around(map->th(index))) * transition.getValue();
                // pixels[index] = color * lfo.getValue(fromTop(map->z(index))) * transition.getValue();
            }
        }

    };

     class VerticallyIsolated : public Pattern<RGBA>
    {
        Transition transition = Transition(
                200, Transition::fromStart, 1200,
                500, Transition::fromEnd, 1500);
        PixelMap3d::Cylindrical *map;

    public:
        VerticallyIsolated(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Vertically isolated";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return; // the fade out is done. we can skip calculating pattern data

            for (int index = 0; index < width; index++)
            {
                int z255 = Utils::rescale(map->z(index), 255, 0, 0.25, 0.6);
                pixels[index] = params->getGradient(z255) * transition.getValue(z255,255);
            }
        }
    };

    class StaticGradientPattern : public Pattern<RGBA>
    {
        PixelMap3d *map;
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);

    public:
        StaticGradientPattern(PixelMap3d *map)
        {
            this->map = map;
            this->name = "Static gradient";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            float height = params->getSize(0.2,1);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                float h = Utils::rescale(map->z(index), height, 0, 0.25, 0.6);
                RGBA color = params->getGradient(h * 255);
                pixels[index] = color * h * transition.getValue();
            }
        }
    };

    class OnBeatColumnChaseUpPattern : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        FadeDown fade[5] = {
            FadeDown(200),
            FadeDown(200),
            FadeDown(200),
            FadeDown(200),
            FadeDown(200)};
        BeatWatcher watcher = BeatWatcher();
        PixelMap3d *map;
        Permute perm;
        int pos = 0;

    public:
        OnBeatColumnChaseUpPattern(PixelMap3d *map)
        {
            this->map = map;
            this->perm = Permute(map->size());
            this->name = "On beat column chase up";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            if (watcher.Triggered())
            {
                pos = (pos + 1) % 5;
                fade[pos].reset();
            }

            float velocity = params->getVelocity(2000, 100);
            float tailSize = params->getSize(300,50);

            for (int column = 0; column < 5; column++)
            {
                int columnStart = column * width / 5;
                int columnEnd = columnStart + width / 5;

                fade[column].duration = tailSize;

                for (int i = columnStart; i < columnEnd; i++)
                {
                    float z = Utils::rescale(map->z(i), 0, 1, 0.25, 0.8);
                    float fadePosition = fade[column].getValue(z * velocity);
                    RGBA color = params->getPrimaryColor(); 
                    pixels[i] = color * fadePosition * (1 - z) * transition.getValue();
                }
            }
        }
    };

    class GrowShrink : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFO<SinFast> lfo;
        PixelMap3d::Cylindrical *map;

    public:
        GrowShrink(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Grow shrink";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(5000,500));
            float size = params->getSize(0.1,0.35);
            float offset = params->getOffset(0,1);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {

                float lfoSize = lfo.getValue(offset * around(map->th(index))) * size; 
                float distance = abs(map->z(index) - 0.45);
                if (distance > lfoSize)
                    continue;

                float distanceAsRatio = 1 - distance / lfoSize ;

                pixels[index] = params->getGradient(distanceAsRatio * 255) * distanceAsRatio * transition.getValue();
            }
        }
    };

    class RotatingRingsPattern : public Pattern<RGBA>
    {
        Transition transition;
        PixelMap3d::Cylindrical *map;
        LFO<Sin> ring1;
        LFO<Sin> ring2;

    public:
        RotatingRingsPattern(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Rotating rings";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return; // the fade out is done. we can skip calculating pattern data

            auto col1 = params->getPrimaryColor() * transition.getValue();
            auto col2 = params->getSecondaryColor() * transition.getValue();
            float size = params->getSize(0.01,0.1);
            float zoffset = params->getVariant(0,-0.2);
            ring1.setPeriod(params->getVelocity(20000,2000));
            ring2.setPeriod(params->getVelocity(14000,1400));

            for (int index = 0; index < width; index++)
            {
                float z_norm = zoffset + Utils::rescale(map->z(index), 0, 2, 0.25, 0.8);
                float offset = around(map->th(index)) * params->getOffset();

                pixels[index] = col1 * softEdge(abs(z_norm - ring1.getValue(offset)), size);
                pixels[index] += col2 * softEdge(abs(z_norm - ring2.getValue(offset)), size);
            }
        }
    };


        class FlyingEmbersPattern : public Pattern<RGBA>
    {
        Transition transition;
        Permute perm;
        int segmentSize;
        LFO<SawDown> lfo = LFO<SawDown>(5000);

    public:
        FlyingEmbersPattern(int segmentSize = 60)
        {
            this->name = "Flying embers";
            this->segmentSize = segmentSize;
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            int numSegments = width / segmentSize;

            perm.setSize(numSegments);

            float pulseWidth = params->getSize(0.05, 0.5);
            float factor = params->getAmount(1, 0.25); // 10; // 1-50;
            float velocity = params->getVelocity(500, 100) * factor;
            lfo.setDutyCycle(pulseWidth / factor);
            float lfoWidth = segmentSize * factor;

            for (int segment = 0; segment < numSegments; segment++)
            {
                int randomSegment = perm.at[segment];

                for (int j = 0; j < segmentSize; j++)
                {
                    float lfoVal = lfo.getValue(float(j) / lfoWidth + float(randomSegment) / numSegments + float(segment), velocity + randomSegment * 20);
                    RGBA col = params->getGradient(lfoVal * 255) * lfoVal * transition.getValue();
                    pixels[segment * segmentSize + j] = col;
                }
            }
        }
    };

}