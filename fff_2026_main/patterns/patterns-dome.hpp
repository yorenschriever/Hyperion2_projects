#pragma once
#include <math.h>
#include <vector>
#include "hyperion.hpp"
#include "common/patterns/pattern-helpers.hpp"

namespace DomePatterns
{

    class XY : public Pattern<RGBA>
    {
        Transition transition = Transition(200,1000);
        PixelMap3d *map;
        LFOTempo<Sin> lfoX = LFOTempo<Sin>(8);
        LFOTempo<Sin> lfoZ = LFOTempo<Sin>(12);

    public:
        XY(PixelMap3d *map)
        {
            this->map = map;
            this->name="XY";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;
 
            auto col = params->getSecondaryColor() * transition.getValue();
            float size = params->getSize(0.01,0.03);
            // lfoX.setPeriod(params->getVelocity(4*20000,2000));
            // lfoZ.setPeriod(params->getVelocity(4*14000,1400));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                float x_norm = (map->x(index)+1)/2;
                float y_norm = (map->y(index)+1)/2;

                float dim = std::max(
                    softEdge(abs(x_norm - lfoX.getValue()), size),
                    softEdge(abs(y_norm - lfoZ.getValue()), size)
                );

                pixels[index] = col * dim;
            }
        }
    };


class Z : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        PixelMap3d *map;
        LFOTempo<Sin> lfoZ = LFOTempo<Sin>(16);

    public:
        Z(PixelMap3d *map)
        {
            this->map = map;
            this->name="Z";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;
 
            auto col = params->getSecondaryColor() * transition.getValue();
            float size = params->getSize(0.01,0.03);
            // lfoZ.setPeriod(params->getVelocity(4*14000,1400));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                float z_norm = Utils::rescale(map->z(index), 0, 1, 1.4, 0.5);

                float dim = softEdge(abs(z_norm - lfoZ.getValue()), size);

                pixels[index] = col * dim;
            }
        }
    };


    class DotBeatPattern : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        PixelMap3d::Cylindrical *map;
        FadeDown fade = FadeDown(200);
        BeatWatcher watcher = BeatWatcher();

    public:
        DotBeatPattern(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Dot beat";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            fade.duration = params->getIntensity(500, 100);

            if (watcher.Triggered())
                fade.reset();

            for (int i = 0; i < map->size(); i++)
            {
                if (map->z(i)<0.44)
                    continue;;

                float radius = fade.getValue()*1.2;
                if (map->r(i) > radius)
                    continue;

                RGBA color = params->getGradient(radius * 255);
                float dim = map->r(i) / radius;
                pixels[i] = color * dim * transition.getValue();   
            }
        }
    };


}