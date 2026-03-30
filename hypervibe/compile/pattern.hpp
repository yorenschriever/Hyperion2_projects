#include "core/generation/patterns/pattern.hpp"
#include "core/generation/patterns/helpers/lfo.h"
#include "core/generation/patterns/helpers/transition.h"
#include "platform/includes/utils.hpp"
#include <cmath>
#include <algorithm>

class LayeredHeartsPattern : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition = Transition(800, 1500);

    static constexpr int MAX_HEARTS = 8;

    LFO<Glow> glowLFOs[MAX_HEARTS];
    LFO<SinFast> sizeLFOs[MAX_HEARTS];
    LFO<SinFast> orbitXLFOs[MAX_HEARTS];
    LFO<NegativeCosFast> orbitYLFOs[MAX_HEARTS];

    float heartSDF(float x, float y)
    {
        float ax = fabsf(x);
        float t = ax * ax + y * y - 1.0f;
        float d = t * t * t - ax * ax * y * y * y;
        return d;
    }

    float heartOutline(float x, float y, float scale, float softness)
    {
        float sx = x / scale;
        float sy = y / scale;
        float d = heartSDF(sx, sy);
        float edge = softness * 0.02f;
        float v = 1.0f - Utils::constrain_f(fabsf(d) / edge, 0.0f, 1.0f);
        return v * v;
    }

public:
    LayeredHeartsPattern(PixelMap *map)
    {
        this->name = "Layered Hearts";
        this->map = map;

        for (int h = 0; h < MAX_HEARTS; h++)
        {
            glowLFOs[h].setPeriod(2200 + h * 700);
            glowLFOs[h].setDutyCycle(0.5f + h * 0.03f);
            sizeLFOs[h].setPeriod(4000 + h * 1100);
            orbitXLFOs[h].setPeriod(5000 + h * 1300);
            orbitYLFOs[h].setPeriod(6000 + h * 1500);
        }
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        int numHearts = params->getAmount(3, 8);
        float baseSize = params->getSize(1.2f, 0.4f);
        float velocity = params->getVelocity(0.3f, 2.0f);
        float orbitRadius = params->getOffset(0.02f, 0.15f);

        for (int h = 0; h < numHearts; h++)
        {
            glowLFOs[h].setPeriod((int)((2200 + h * 700) / velocity));
            sizeLFOs[h].setPeriod((int)((4000 + h * 1100) / velocity));
            orbitXLFOs[h].setPeriod((int)((5000 + h * 1300) / velocity));
            orbitYLFOs[h].setPeriod((int)((6000 + h * 1500) / velocity));
        }

        int mapSize = std::min(width, (int)map->size());

        for (int h = 0; h < numHearts; h++)
        {
            float layerT = (float)h / (float)(numHearts - 1);

            float scale = baseSize * (1.0f - layerT * 0.6f);
            float sizeOsc = sizeLFOs[h].getValue() * 0.15f - 0.075f;
            scale += sizeOsc;
            scale = std::max(scale, 0.15f);

            float orbitR = orbitRadius * (1.0f - layerT * 0.5f);
            float cx = (orbitXLFOs[h].getValue() - 0.5f) * 2.0f * orbitR;
            float cy = (orbitYLFOs[h].getValue() - 0.5f) * 2.0f * orbitR;

            float glow = glowLFOs[h].getValue();
            glow = 0.3f + 0.7f * glow;

            float brightness = 0.25f + 0.75f * layerT;
            float softness = 1.5f - layerT * 0.7f;

            float gradientPos = layerT;

            for (int i = 0; i < mapSize; i++)
            {
                float px = map->x(i) - cx;
                float py = -(map->y(i) - cy) - 0.3f * scale;

                float v = heartOutline(px, py, scale, softness);
                if (v < 0.01f)
                    continue;

                RGBA color = params->getGradientf(gradientPos);
                float alpha = v * glow * brightness;
                alpha = std::min(alpha, 1.0f);
                pixels[i] += color * alpha * transition.getValue();
            }
        }
    }
};

typedef LayeredHeartsPattern VibePattern;