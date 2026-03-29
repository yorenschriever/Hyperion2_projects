#include "core/generation/patterns/pattern.hpp"
#include "core/generation/patterns/helpers/fade.h"
#include "core/generation/patterns/helpers/lfo.h"
#include "core/generation/patterns/helpers/transition.h"
#include "platform/includes/utils.hpp"
#include <algorithm>
#include <cmath>

class MeteorShowerPattern : public Pattern<RGBA>
{
    static const int MAX_METEORS = 30;
    Transition transition = Transition(300, 1000);
    PixelMap *map;
    LFO<SawUp> meteorLFOs[MAX_METEORS];
    LFO<SinFast> strobeLFO;
    float angles[MAX_METEORS];

public:
    MeteorShowerPattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Meteor Shower";

        strobeLFO.setPeriod(80);

        for (int i = 0; i < MAX_METEORS; i++)
        {
            angles[i] = Utils::random_f() * 2.0f * M_PI;
            meteorLFOs[i].setPeriod(Utils::random(1500, 3000));
        }
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        int amount = params->getAmount(3, 25);
        float velocity = params->getVelocity(4000, 800);
        float size = params->getSize(0.15f, 0.03f);
        float trailLength = params->getIntensity(0.4f, 0.15f);
        float offset = params->getOffset(0.0f, 1.0f);

        for (int m = 0; m < amount; m++)
        {
            meteorLFOs[m].setPeriod(velocity + m * (offset * 300));
        }

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float px = map->x(i);
            float py = map->y(i);

            for (int m = 0; m < amount; m++)
            {
                float phase = meteorLFOs[m].getPhase();
                float easedPhase = phase * phase;

                float dirX = cosf(angles[m]);
                float dirY = sinf(angles[m]);

                float meteorX = dirX * easedPhase * 1.5f;
                float meteorY = dirY * easedPhase * 1.5f;

                float dx = px - meteorX;
                float dy = py - meteorY;
                float dist = sqrtf(dx * dx + dy * dy);

                float toCenter = sqrtf(px * px + py * py);
                float meteorR = sqrtf(meteorX * meteorX + meteorY * meteorY);

                float behindDot = (dx * (-dirX) + dy * (-dirY));
                float trailDist = 0.0f;
                if (behindDot > 0)
                {
                    float perpX = dx - behindDot * (-dirX);
                    float perpY = dy - behindDot * (-dirY);
                    trailDist = sqrtf(perpX * perpX + perpY * perpY);
                }

                float headBrightness = .0f;
                if (dist < size)
                {
                    headBrightness = 1.0f - (dist / size);
                    //float strobe = 1; //0.7f + 0.3f * strobeLFO.getValue(float(m) / amount);
                    //headBrightness *= strobe;
                }

                float trailBrightness = 0.0f;
                if (behindDot > 0 && behindDot < trailLength && trailDist < size * 0.6f)
                {
                    trailBrightness = (1.0f - behindDot / trailLength) * (1.0f - trailDist / (size * 0.6f));
                    trailBrightness *= 0.6f;
                }

                float brightness = std::max(headBrightness, trailBrightness);
                brightness = Utils::constrain_f(brightness, 0.0f, 1.0f);

                if (brightness > 0.01f)
                {
                    float gradientVal = easedPhase;
                    RGBA color = params->getGradientf(1.-gradientVal) * brightness * transition.getValue();
                    pixels[i] += color;
                }
            }
        }

        for (int m = 0; m < amount; m++)
        {
            if (meteorLFOs[m].getPhase() < 0.02f)
            {
                angles[m] = Utils::random_f() * 2.0f * M_PI;
            }
        }
    }
};

typedef MeteorShowerPattern VibePattern;