#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include <math.h>
#include <vector>

namespace BasePatterns
{

    class SegmentChasePattern : public Pattern<RGBA>
    {
        Transition transition;
        Permute perm;
        int segmentSize;
        LFO<SawDown> lfo = LFO<SawDown>(5000);

    public:
        SegmentChasePattern(int segmentSize = 60)
        {
            this->name = "Segment chase";
            this->segmentSize = segmentSize;
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            int numSegments = width / segmentSize;

            perm.setSize(numSegments);

            float pulseWidth = params->getSize(0.25, 1);
            float factor = params->getAmount(4, 0.5); // 10; // 1-50;
            lfo.setPeriod(params->getVelocity(2000, 200) * factor);
            lfo.setDutyCycle(pulseWidth / factor);
            float lfoWidth = segmentSize * factor;

            for (int segment = 0; segment < numSegments; segment++)
            {
                int randomSegment = perm.at[segment];

                for (int j = 0; j < segmentSize; j++)
                {
                    float lfoVal = lfo.getValue(float(j) / lfoWidth + float(randomSegment) / numSegments + float(segment));
                    RGBA col = params->getGradient(lfoVal * 255) * lfoVal * transition.getValue();
                    pixels[segment * segmentSize + j] = col;
                }
            }
        }
    };

    class PaletteGradient : public Pattern<RGBA>
    {
        Transition transition;
        PixelMap *map;

    public:
        PaletteGradient(PixelMap *map)
        {
            this->name = "Palette gradient";
            this->map = map;
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getGradient(Utils::rescale(map->y(index), 0, 255, 0.1, -1));
                pixels[index] = color * transition.getValue();
            }
        }
    };

    class AngularWave : public Pattern<RGBA>
    {
        Transition transition;
        LFO<Glow> lfo;
        PixelMap3d::Cylindrical *map;

    public:
        AngularWave(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Angular wave";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize(0.2, 1));
            int amount = params->getAmount(1, 7.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getPrimaryColor();
                float lfoArg = around(map->th(index)) * amount;
                pixels[index] = color * lfo.getValue(lfoArg) * transition.getValue();
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


    class BarberPole : public Pattern<RGBA>
    {
        Transition transition;
        LFO<SoftSawDown> lfo;
        PixelMap3d::Cylindrical *map;

    public:
        BarberPole(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Barber pole";
            lfo.setSoftEdgeWidth(0.1);
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize());
            int amount = params->getAmount(1, 7.99);

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getPrimaryColor();
                float lfoArg = (around(map->th(index)+M_PI) + map->z(index)) * amount;
                pixels[index] = color * lfo.getValue(lfoArg) * transition.getValue();
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

        float fromTop2(float z)
        {
            return z+1; //1 - abs(1 - 2 * z);
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return; // the fade out is done. we can skip calculating pattern data

            auto col1 = params->getPrimaryColor() * transition.getValue();
            auto col2 = params->getSecondaryColor() * transition.getValue();
            float size = params->getSize(1.2,0.2);
            float zoffset = params->getVariant(0,-0.2);
            ring1.setPeriod(params->getVelocity(20000,2000));
            ring2.setPeriod(params->getVelocity(14000,1400));

            for (int index = 0; index < width; index++)
            {
                float z_norm = zoffset+fromTop2(map->z(index));
                float offset = around(map->th(index)) * params->getOffset();

                // pixels[index] = col1 * softEdge(abs(z_norm - ring1.getValue(offset)), size);
                // pixels[index] += col2 * softEdge(abs(z_norm - ring2.getValue(offset)), size);

                pixels[index] = col1 * Cos::getValue(Utils::constrain_f(abs(z_norm - ring1.getValue(offset))/size,0,0.5),0);
                pixels[index] += col2 * Cos::getValue(Utils::constrain_f(abs(z_norm - ring2.getValue(offset))/size,0,0.5),0);
            }
        }
    };



    class MeshPattern : public Pattern<RGBA>
    {
        Transition transition;
        PixelMap3d::Cylindrical *map;
        LFO<Sin> ring1;
        LFO<Sin> ring2;

    public:
        MeshPattern(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Moire Mesh";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params* params) override
        {
            if (!transition.Calculate(active))
                return; // the fade out is done. we can skip calculating pattern data

            auto col1 = params->getPrimaryColor() * transition.getValue();
            auto col2 = params->getSecondaryColor() * transition.getValue();
            float size = params->getSize(0.3,0.1);
            float zoffset = params->getVariant(1,-0.8);
            ring1.setPeriod(params->getVelocity(2*20000,2000));
            ring2.setPeriod(params->getVelocity(2*14000,1400));

            for (int index = 0; index < width; index++)
            {
                float z_norm = map->z(index) + zoffset;
                float offset = around(map->th(index)) * params->getOffset(0,0.5);

                pixels[index] = col1 * Cos::getValue(abs(z_norm - ring1.getValue(offset))/size,0);
                pixels[index] += col2 * Cos::getValue(abs(z_norm - ring2.getValue(offset))/size,0);
            }
        }
    };

    class BaseBeamsPattern : public Pattern<RGBA>
    {
        Transition transition;
        PixelMap3d::Cylindrical *map;

    public:
        BaseBeamsPattern()
        {
            this->name = "Base beams";
            // this->map = map;
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            for (int index = 0; index < width; index++)
            {
                int beamIndex = index / (3*60);

                bool isVerticalBeam = 
                    beamIndex == 0 || 
                    beamIndex == 17 ||
                    beamIndex == 6 || 
                    beamIndex == 11;

                int ledIndexInBeam = index % (3*60);
                bool isHorizontalBeam = 
                    (ledIndexInBeam >= 120 && ledIndexInBeam < 125)
                    || ledIndexInBeam < 5
                    || ledIndexInBeam >= 175;

                if (!isVerticalBeam && !isHorizontalBeam)    
                    continue;

                pixels[index] += params->getSecondaryColor() * transition.getValue();

            }
        }
    };

}