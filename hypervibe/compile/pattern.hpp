#include "core/generation/patterns/pattern.hpp"
#include "core/generation/patterns/helpers/fade.h"
#include "core/generation/patterns/helpers/transition.h"
#include "platform/includes/utils.hpp"
#include <algorithm>
#include <cmath>

class LightningStrikePattern : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition = Transition(0, 800);
    BeatWatcher watcher;
    Fade<Down, Cubic> fade = Fade<Down, Cubic>(400);
    
    static const int MAX_SEGMENTS = 12;
    float boltX[MAX_SEGMENTS];
    float boltY[MAX_SEGMENTS];
    int numSegments = 0;
    bool struck = false;

    void generateBolt()
    {
        float intensity = 0.6f + Utils::random_f() * 0.4f;
        numSegments = 6 + Utils::random(0, MAX_SEGMENTS - 6);
        
        float startX = Utils::random_f() * 1.4f - 0.7f;
        boltX[0] = startX;
        boltY[0] = 1.0f;
        
        for (int i = 1; i < numSegments; i++)
        {
            float t = float(i) / (numSegments - 1);
            boltX[i] = boltX[i - 1] + (Utils::random_f() - 0.5f) * 0.4f * intensity;
            boltY[i] = 1.0f - t * 2.0f;
        }
    }

    float distToSegment(float px, float py, float ax, float ay, float bx, float by)
    {
        float dx = bx - ax;
        float dy = by - ay;
        float len2 = dx * dx + dy * dy;
        if (len2 < 0.0001f)
            return std::sqrt((px - ax) * (px - ax) + (py - ay) * (py - ay));
        float t = ((px - ax) * dx + (py - ay) * dy) / len2;
        t = Utils::constrain_f(t, 0.0f, 1.0f);
        float projX = ax + t * dx;
        float projY = ay + t * dy;
        return std::sqrt((px - projX) * (px - projX) + (py - projY) * (py - projY));
    }

    float distToBolt(float px, float py)
    {
        float minDist = 999.0f;
        for (int i = 0; i < numSegments - 1; i++)
        {
            float d = distToSegment(px, py, boltX[i], boltY[i], boltX[i + 1], boltY[i + 1]);
            if (d < minDist)
                minDist = d;
        }
        return minDist;
    }

public:
    LightningStrikePattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Lightning strike";
        numSegments = 0;
        struck = false;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (fade.isFinished() && !active)
            return;

        float thickness = params->getSize(0.25f, 0.05f);
        float glowSize = thickness * 3.0f;

        if (watcher.Triggered())
        {
            generateBolt();
            fade.reset();
            struck = true;
        }

        if (!struck || numSegments < 2)
            return;

        float fadeVal = fade.getValue();

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float px = map->x(i);
            float py = map->y(i);

            float dist = distToBolt(px, py);

            if (dist < glowSize)
            {
                float coreBrightness = 0.0f;
                if (dist < thickness)
                    coreBrightness = 1.0f;
                else
                    coreBrightness = 1.0f - (dist - thickness) / (glowSize - thickness);

                coreBrightness *= coreBrightness;

                RGBA core = params->getHighlightColor() * (coreBrightness > 0.8f ? coreBrightness : 0.0f);
                RGBA glow = params->getPrimaryColor() * coreBrightness;

                pixels[i] += (core + glow) * fadeVal * transition.getValue();
            }
        }
    }
};

typedef LightningStrikePattern VibePattern;