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
        PixelMap3d::CylindricalPtr map;

    public:
        TopChase(PixelMap3d::CylindricalPtr map)
        {
            this->map = map;
            this->name = "Top chase";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            int amount = params->getAmount(1, 6);
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



    class Lighthouse : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        LFO<Glow> lfo;
        PixelMap3d::CylindricalPtr map;

    public:
        Lighthouse(PixelMap3d::CylindricalPtr map)
        {
            this->map = map;
            this->name = "Lighthouse";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            lfo.setPeriod(params->getVelocity(5000, 500));
            lfo.setDutyCycle(params->getSize(0.06, 0.5));

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                RGBA color = params->getSecondaryColor();
                // RGBA color = params->getGradient(fromTop(map->z(index))*255);
                pixels[index] = color * lfo.getValue(-2 * around(map->th(index))) * /*fromMid(map->operator[](index)) **/ transition.getValue();
                // pixels[index] = color * lfo.getValue(fromTop(map->z(index))) * transition.getValue();
            }
        }
    };


    class GrowingCirclesPattern : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        FadeDown fade[6] = {
            FadeDown(200),
            FadeDown(200),
            FadeDown(200),
            FadeDown(200),
            FadeDown(200),
            FadeDown(200)};
        std::vector<float> radii[6];
        BeatWatcher watcher = BeatWatcher();
        Permute perm;
        int pos = 0;

    public:
        GrowingCirclesPattern(PixelMap3dPtr map)
        {
            for (int i = 0; i < 6; i++)
            {
                float xc = cos(float(i) / 6 * 2 * M_PI);
                float yc = 0.4;
                float zc = sin(float(i) / 6 * 2 * M_PI);
                std::transform(map->begin(), map->end(), std::back_inserter(radii[i]), [xc, yc, zc](PixelPosition3d pos) -> float
                               { return sqrt(pow(pos.x - xc, 2) + pow(pos.y - yc, 2) + pow(pos.z - zc, 2)); });
            }
            this->perm = Permute(map->size());
            this->name = "Growing circles";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            if (watcher.Triggered() && Tempo::GetBeatNumber() % 4 == 0)
            {
                pos = (pos + 1) % 6;
                fade[pos].reset();
            }

            float velocity = params->getVelocity(1000, 100)*2;
            for(int i=0;i<6;i++)
                fade[i].duration = params->getSize(400,50)*velocity/1000;
            
            // float density = 481./width;

            for (int i = 0; i < width; i++)
            {
                for (int column = 0; column < 6; column++)
                {
                    float fadePosition = fade[column].getValue(radii[column][i] * velocity);
                    RGBA color = params->getPrimaryColor();
                    pixels[i] += color * fadePosition * transition.getValue();
                }
            }
        }
    };

    class RadialGlitterFadePattern : public Pattern<RGBA>
    {
        Transition transition = Transition(
            200, Transition::none, 0,
            1000, Transition::none, 0);
        PixelMap3d::CylindricalPtr map;
        FadeDown fade1 = FadeDown(200);
        FadeDown fade2 = FadeDown(200);
        BeatWatcher watcher = BeatWatcher();
        Permute perm1;
        Permute perm2;
        int fadeNr =0;

    public:
        RadialGlitterFadePattern(PixelMap3d::CylindricalPtr map)
        {
            this->map = map;
            this->perm1 = Permute(map->size());
            this->perm2 = Permute(map->size());
            this->name = "Radial glitter fade";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!transition.Calculate(active))
                return;

            fade1.duration = params->getIntensity(500, 100);
            fade2.duration = params->getIntensity(500, 100);

            // timeline.FrameStart();
            // if (timeline.Happened(0))
            if (watcher.Triggered())
            {
                fadeNr = (fadeNr+1)%2;
                if (fadeNr==0){
                    fade1.reset();
                    perm1.permute();
                } else {
                    fade2.reset();
                    perm2.permute();
                }
            }

            float velocity = params->getVelocity(2000, 750);
            // float trail = params->getIntensity(0,1) * density;
            // float trail = params->getIntensity(0, 200);
            float fromCenter = params->getVariant() >= 0.5;

            if (perm1.at ==0 || perm2.at==0)
                return;

            for (int i = 0; i < map->size(); i++)
            {
                // fade.duration = perm.at[i] * trail; // + 100;
                // fade.duration = (perm.at[i] * trail) < map->size() / 20 ? ; // + 100;

                // fade.duration = 100;
                // if (perm.at[i] < density * map->size()/ 10)
                //     fade.duration *= 4;

                float density = 481. / width;
                fade1.duration = 100; // trail + perm.at[i] / (density * map->size()/ 10);
                fade2.duration = 100; 
                if (perm1.at[i] < density * map->size() / 10)
                    fade1.duration *= perm1.at[i] * 4 / (density * map->size() / 10);
                if (perm2.at[i] < density * map->size() / 10)
                    fade2.duration *= perm2.at[i] * 4 / (density * map->size() / 10);

                float conePos = fromCenter ? 
                    // vanaf midden
                    (0.5 + (map->r(i) - map->z(i)) / 4) :
                    // vanaf de knik beide kanten op
                    (1-(map->r(i) + map->z(i))/4);

                float fadePosition1 = fade1.getValue(conePos * velocity);
                RGBA color1 = params->getGradient(fadePosition1 * 255);
                pixels[i] = color1 * fadePosition1 * (1.5 - map->r(i)) * transition.getValue();

                float fadePosition2 = fade2.getValue(conePos * velocity);
                RGBA color2 = params->getGradient(fadePosition2 * 255);
                pixels[i] += color2 * fadePosition2 * (1.5 - map->r(i)) * transition.getValue();
            }
        }
    };

}