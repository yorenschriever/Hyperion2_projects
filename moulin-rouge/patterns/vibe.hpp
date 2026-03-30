#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include <math.h>
#include <vector>

namespace VibePatterns
{

class TriangleBurst : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition;
    BeatWatcher watcher;
    FadeDown fade = FadeDown(900);
    LFO<SawUp> rotationLfo;

    inline float cross(float px, float py, float ax, float ay, float bx, float by)
    {
        return (bx - ax) * (py - ay) - (by - ay) * (px - ax);
    }

    inline bool inTriangle(float px, float py, float x0, float y0, float x1, float y1, float x2, float y2)
    {
        float d1 = cross(px, py, x0, y0, x1, y1);
        float d2 = cross(px, py, x1, y1, x2, y2);
        float d3 = cross(px, py, x2, y2, x0, y0);
        return !((d1 < 0 || d2 < 0 || d3 < 0) && (d1 > 0 || d2 > 0 || d3 > 0));
    }

public:
    TriangleBurst(PixelMap *map)
    {
        this->name = "Triangle Burst";
        this->map = map;
        rotationLfo.setPeriod(3000);
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered())
            fade.reset();

        float growth = (1.0f - fade.getValue()) * 1.4f;
        if (growth < 0.001f)
            return;

        float angle = rotationLfo.getPhase() * 6.28318f;
        float x0 = growth * cosf(angle);
        float y0 = growth * sinf(angle);
        float x1 = growth * cosf(angle + 2.09440f);
        float y1 = growth * sinf(angle + 2.09440f);
        float x2 = growth * cosf(angle + 4.18879f);
        float y2 = growth * sinf(angle + 4.18879f);

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float px = map->x(i);
            float py = map->y(i);
            if (inTriangle(px, py, x0, y0, x1, y1, x2, y2))
            {
                float dist = sqrtf(px * px + py * py);
                pixels[i] = params->getGradient(dist / growth) * transition.getValue();
            }
        }
    }
};


class RotatingTriangle : public Pattern<RGBA>
{
    Transition transition;
    BeatWatcher watcher;
    LFO<SawUp> rotateLfo;
    PixelMap *map;
    float size = 0.0f;
    int lastTimestamp = 0;

public:
    RotatingTriangle(PixelMap *map)
    {
        this->name = "Rotating Triangle";
        this->map = map;
        rotateLfo.setPeriod(6000);
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered())
            size = 0.0f;

        int now = Utils::millis();
        float dt = (now - lastTimestamp) / 1000.0f;
        lastTimestamp = now;
        size = std::min(size + dt * 0.9f, 2.0f);

        float angle = rotateLfo.getValue() * 2.0f * M_PI;
        float outlineWidth = 0.04f;

        float cosA = cosf(angle);
        float sinA = sinf(angle);

        float v0x = cosA * 0.0f        - sinA *  size;
        float v0y = sinA * 0.0f        + cosA *  size;
        float v1x = cosA * ( size * sqrtf(3.0f) * 0.5f) - sinA * (-size * 0.5f);
        float v1y = sinA * ( size * sqrtf(3.0f) * 0.5f) + cosA * (-size * 0.5f);
        float v2x = cosA * (-size * sqrtf(3.0f) * 0.5f) - sinA * (-size * 0.5f);
        float v2y = sinA * (-size * sqrtf(3.0f) * 0.5f) + cosA * (-size * 0.5f);

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float x = map->x(i);
            float y = map->y(i);

            auto edgeDist = [](float px, float py, float ax, float ay, float bx, float by) -> float {
                float dx = bx - ax, dy = by - ay;
                float len = sqrtf(dx*dx + dy*dy);
                if (len < 1e-6f) return 1e6f;
                float nx = -dy / len, ny = dx / len;
                float d = (px - ax) * nx + (py - ay) * ny;
                float t = ((px - ax) * dx + (py - ay) * dy) / (len * len);
                t = std::max(0.0f, std::min(1.0f, t));
                float cx = ax + t * dx - px;
                float cy = ay + t * dy - py;
                return sqrtf(cx*cx + cy*cy);
            };

            float d0 = edgeDist(x, y, v0x, v0y, v1x, v1y);
            float d1 = edgeDist(x, y, v1x, v1y, v2x, v2y);
            float d2 = edgeDist(x, y, v2x, v2y, v0x, v0y);

            float minDist = std::min({d0, d1, d2});
            float onEdge = std::max(0.0f, 1.0f - minDist / outlineWidth);

            if (onEdge > 0.0f)
                pixels[i] = params->getPrimaryColor() * (onEdge * transition.getValue());
        }
    }
};


class TriangleStutter : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition;
    LFOTempo<SawDown> growLfo = LFOTempo<SawDown>(4);
    LFOTempo<SawUp> rotateLfo = LFOTempo<SawUp>(16);
    static BeatWatcher watcher; //make this static, so it only triggers once for all instances of this pattern, so the triangles on the wings and base will expand at the same time.
    static float rotationOffset; 
    static float growScale;

public:
    TriangleStutter(PixelMap *map)
    {
        this->name = "Triangle Stutter";
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered())
        {
            rotationOffset = Utils::random_f()*0.25;
            growScale = 0.75f + Utils::random_f() * .5f;
        }

        float rotation = rotateLfo.getValue(rotationOffset) * 2.0f * M_PI;
        float grow = Utils::rescale(growLfo.getValue(), 0.2f, growScale, 0.0f, 1.0f);
        float maxRadius = 1.0f;
        float outlineWidth = 0.1f;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float x = map->x(i);
            float y = map->y(i);

            float cx = x * cos(-rotation) - y * sin(-rotation);
            float cy = x * sin(-rotation) + y * cos(-rotation);

            float triangleRadius = grow * maxRadius;

            if (triangleRadius < 0.001f)
                continue;

            float dist = triangleEdgeDist(cx, cy, triangleRadius);

            float alpha = 0.0f;
            if (dist >= 0.0f && dist <= outlineWidth)
            {
                alpha = 1.0f - (dist / outlineWidth);
                alpha = alpha * alpha;
            }

            if (alpha < 0.001f)
                continue;

            float visibility = transition.getValue();

            pixels[i] += params->getPrimaryColor() * (alpha * visibility);
        }
    }

private:
    inline float triangleEdgeDist(float x, float y, float radius)
    {
        const float angleOffset = -M_PI / 2.0f;
        float minEdgeDist = 1e9f;

        for (int v = 0; v < 3; v++)
        {
            float a0 = angleOffset + v * (2.0f * M_PI / 3.0f);
            float a1 = angleOffset + (v + 1) * (2.0f * M_PI / 3.0f);

            float x0 = cos(a0) * radius;
            float y0 = sin(a0) * radius;
            float x1 = cos(a1) * radius;
            float y1 = sin(a1) * radius;

            float edx = x1 - x0;
            float edy = y1 - y0;
            float len2 = edx * edx + edy * edy;
            float t = ((x - x0) * edx + (y - y0) * edy) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            float px = x0 + t * edx;
            float py = y0 + t * edy;
            float d = sqrt((x - px) * (x - px) + (y - py) * (y - py));
            if (d < minEdgeDist)
                minEdgeDist = d;
        }

        return minEdgeDist - 0.015f;
    }
};
float TriangleStutter::rotationOffset = 0.0f;
float TriangleStutter::growScale = 1.0f;
BeatWatcher TriangleStutter::watcher;


class TrianglePulse : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition;
    LFO<SawUp> rotationLfo;
    LFOTempo<SawUp> growLfo;

    float distToTriangleEdge(float px, float py, float radius, float angle)
    {
        float cos120 = -0.5f;
        float sin120 = 0.866025f;

        float vx[3], vy[3];
        for (int j = 0; j < 3; j++)
        {
            float a = angle + j * 2.0944f;
            vx[j] = radius * cosf(a);
            vy[j] = radius * sinf(a);
        }

        float minDist = 1e9f;
        for (int j = 0; j < 3; j++)
        {
            int k = (j + 1) % 3;
            float ex = vx[k] - vx[j];
            float ey = vy[k] - vy[j];
            float dx = px - vx[j];
            float dy = py - vy[j];
            float len2 = ex * ex + ey * ey;
            float t = (dx * ex + dy * ey) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            float closestX = vx[j] + t * ex;
            float closestY = vy[j] + t * ey;
            float dist = sqrtf((px - closestX) * (px - closestX) + (py - closestY) * (py - closestY));
            if (dist < minDist)
                minDist = dist;
        }
        return minDist;
    }

public:
    TrianglePulse(PixelMap *map)
    {
        this->name = "Triangle pulse";
        this->map = map;
        rotationLfo.setPeriod(4000);
        growLfo.setPeriod(2);
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float angle = rotationLfo.getPhase() * 2.0f * M_PI;
        float growPhase = growLfo.getPhase();
        float maxRadius = 1.0f;
        float radius = growPhase * maxRadius;
        float thickness = 0.08f;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float px = map->x(i);
            float py = map->y(i);

            if (radius < 0.01f)
            {
                pixels[i] = RGBA(0, 0, 0, 0);
                continue;
            }

            float dist = distToTriangleEdge(px, py, radius, angle);
            float brightness = 1.0f - std::min(dist / thickness, 1.0f);
            brightness = brightness * brightness;

            pixels[i] = params->getSecondaryColor() * (brightness * transition.getValue());
        }
    }
};

class TriangleOutlineGrow : public Pattern<RGBA>
{
    PixelMap::Polar *map;
    Transition transition = Transition(200, 1000);
    LFO<SawUp> rotationLfo;
    BeatWatcher watcher;
    FadeUp growFade = FadeUp(1000);

public:
    TriangleOutlineGrow(PixelMap *map)
    {
        this->map = map->toPolar();
        this->name = "Triangle outline grow";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        rotationLfo.setPeriod(params->getVelocity(8000, 1000));
        float lineThickness = params->getSize(0.15f, 0.03f);

        if (watcher.Triggered(2))
            growFade.reset();

        float growPhase = growFade.getValue();
        float minRadius = 0.05f;
        float maxRadius = 1.0f;
        float currentRadius = minRadius + (maxRadius - minRadius) * growPhase;

        float rotationAngle = rotationLfo.getPhase() * 2.0f * M_PI;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float r = map->r(i);
            float a = map->th(i);

            float adjustedAngle = a - rotationAngle;
            while (adjustedAngle < 0) adjustedAngle += 2.0f * M_PI;
            while (adjustedAngle >= 2.0f * M_PI) adjustedAngle -= 2.0f * M_PI;

            float triAngle = fmod(adjustedAngle, 2.0f * M_PI / 3.0f) - M_PI / 3.0f;

            float edgeRadius = currentRadius * cos(M_PI / 3.0f) / cos(triAngle);

            float dist = fabs(r - edgeRadius);

            if (r <= edgeRadius + lineThickness && dist < lineThickness)
            {
                float brightness = 1.0f - (dist / lineThickness);
                pixels[i] = params->getSecondaryColor() * brightness * transition.getValue();
            }
        }
    }
};



class MatrixRainPattern : public Pattern<RGBA>
{
    static const int MAX_DROPS = 30;

    struct Drop
    {
        float x;
        float headY;
        float speed;
        float trailLength;
        bool alive;
    };

    static Drop drops[MAX_DROPS];
    Transition transition = Transition(500, 1500);
    PixelMap *map;
    unsigned long frameCounter = 0;

    void spawnDrop(Drop &d, float speedMin, float speedMax, float sizeParam)
    {
        d.x = Utils::random_f() * 2.0f - 1.0f;
        d.headY = -1.0f - Utils::random_f() * 0.5f;
        d.speed = Utils::random_f() * (speedMax - speedMin) + speedMin;
        d.trailLength = 0.3f + Utils::random_f() * 1.2f * sizeParam;
        d.alive = true;
    }

public:
    MatrixRainPattern(PixelMap *map)
    {
        this->name = "Notre dame";
        this->map = map;
        for (int i = 0; i < MAX_DROPS; i++)
            drops[i].alive = false;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        frameCounter++;

        float velocity = params->getVelocity(0.002f, 0.015f);
        int targetDrops = params->getAmount(3, 25);
        float sizeParam = params->getSize(0.3f, 1.5f);
        float dropWidth = 0.05f + params->getIntensity(0.02f, 0.15f);

        int aliveCount = 0;
        for (int i = 0; i < MAX_DROPS; i++)
        {
            if (drops[i].alive)
            {
                drops[i].headY += drops[i].speed * velocity;
                if (drops[i].headY - drops[i].trailLength > 1.0f)
                    drops[i].alive = false;
                else
                    aliveCount++;
            }
        }

        if (frameCounter % 3 == 0)
        {
            while (aliveCount < targetDrops)
            {
                for (int i = 0; i < MAX_DROPS; i++)
                {
                    if (!drops[i].alive)
                    {
                        spawnDrop(drops[i], 0.5f, 1.5f, sizeParam);
                        aliveCount++;
                        break;
                    }
                }
                if (aliveCount < targetDrops)
                {
                    bool found = false;
                    for (int i = 0; i < MAX_DROPS; i++)
                        if (!drops[i].alive) { found = true; break; }
                    if (!found) break;
                }
                else break;
            }
        }

        int mapSize = std::min(width, (int)map->size());

        for (int i = 0; i < mapSize; i++)
        {
            float px = map->x(i);
            float py = -map->y(i);

            for (int d = 0; d < MAX_DROPS; d++)
            {
                if (!drops[d].alive)
                    continue;

                float dx = std::abs(px - drops[d].x);
                if (dx > dropWidth)
                    continue;

                float tailY = drops[d].headY - drops[d].trailLength;

                if (py < tailY || py > drops[d].headY)
                    continue;

                float posInTrail = (drops[d].headY - py) / drops[d].trailLength;
                float brightness = 1.0f - posInTrail;
                brightness *= brightness;

                float lateralFade = 1.0f - (dx / dropWidth);
                lateralFade *= lateralFade;

                float alpha = brightness * lateralFade;

                RGBA color = params->getSecondaryColor() * alpha;

                if (posInTrail < 0.1f)
                    color += params->getHighlightColor() * (1.0f - posInTrail / 0.1f) * lateralFade * 0.5f;

                pixels[i] += color * transition.getValue();
            }
        }
    }
};
MatrixRainPattern::Drop MatrixRainPattern::drops[MAX_DROPS] = {};

class HeartbeatPattern : public Pattern<RGBA>
{
    Transition transition = Transition(200, 1000);
    PixelMap *map;
    BeatWatcher watcher;
    FadeDown pulse1 = FadeDown(250);
    FadeDown pulse2 = FadeDown(250);
    int beatCount = 0;
    bool doublePulseTriggered = false;
    unsigned long frameCounter = 0;
    int frameAtTrigger = 0;

public:
    HeartbeatPattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Heartbeat";
    }

    inline bool isInsideHeart(float x, float y)
    {
        float x2 = x * x;
        float y2 = y * y;
        float val = (x2 + y2 - 1.0f);
        return (val * val * val - x2 * y2 * y) <= 0.0f;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        frameCounter++;

        if (watcher.Triggered())
        {
            beatCount++;
            if (beatCount % 2 == 1)
            {
                pulse1.reset();
                doublePulseTriggered = true;
                frameAtTrigger = frameCounter;
            }
        }

        if (doublePulseTriggered && (frameCounter - frameAtTrigger) > 8)
        {
            pulse2.reset();
            doublePulseTriggered = false;
        }

        float pulseVal = std::max(pulse1.getValue(), pulse2.getValue());

        float size = params->getSize(0.1f, 0.3f);
        float scale = size + pulseVal * 0.3f * size;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float mx = map->x(i);
            float my = map->y(i) + 0.6f;

            float hx = mx / scale;
            float hy = (my - 0.0f) / scale;

            if (isInsideHeart(hx, hy))
            {
                float dist = sqrtf(hx * hx + hy * hy);
                float brightness = Utils::constrain_f(1.0f - dist * 0.4f, 0.3f, 1.0f);
                float glow = 0.3f + 0.7f * pulseVal;
                pixels[i] = params->getPrimaryColor() * brightness * glow * transition.getValue();
            }
        }
    }
};



#include "core/generation/patterns/pattern.hpp"
#include "core/generation/patterns/helpers/fade.h"
#include "core/generation/patterns/helpers/transition.h"
#include "platform/includes/utils.hpp"
#include <algorithm>
#include <cmath>

class FireworkBurstPattern : public Pattern<RGBA>
{
    Transition transition = Transition(200, 1500);
    PixelMap *map;
    static BeatWatcher watcher;
    static FadeDown fade;

    static float burstCenterX ;
    static float burstCenterY ;

    const float speeds[4] = {1.0f, 0.7f, 0.45f, 0.25f};
    // const float speeds[4] = {0.25, 0.45, 0.7, 1.0f};

public:
    FireworkBurstPattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Blast wave";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float size = params->getSize(1.5f, 0.7f);
        float velocity = params->getVelocity(2000, 600);
        float intensity = params->getIntensity(0.5f, 1.0f);

        fade.setDuration(velocity);

        if (watcher.Triggered(2))
        {
            fade.reset();
            burstCenterX = Utils::random_f() * 1.4f - 0.7f;
            burstCenterY = Utils::random_f() * 1.4f - 0.7f;
        }

        if (fade.isFinished())
            return;

        float phase = fade.getPhase();
        float fadeVal = fade.getValue();

        int amount = params->getAmount(2, 5);

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float dx = map->x(i) - burstCenterX;
            float dy = map->y(i) - burstCenterY;
            float dist = sqrtf(dx * dx + dy * dy);

            RGBA accum = RGBA(0, 0, 0, 0);

            for (int e = 0; e < amount; e++)
            {
                float offset = params->getOffset(0.0f, 0.3f) * e;
                float ringRadius = ( phase) * speeds[e] * size + offset;
                float ringWidth = 0.15f * size * (1.0f + 0.3f * e);

                float diff = fabsf(dist - ringRadius);
                if (diff > ringWidth)
                    continue;

                float brightness = (1.0f - diff / ringWidth);
                brightness = brightness * brightness;

                float gradPos = Utils::constrain_f((float)e / 3.0f + (1.-phase) * 0.3f * 1.5, 0.0f, 1.0f);
                RGBA color = params->getGradientf(gradPos);

                float distFade = 1; //1.0f - Utils::constrain_f(dist / (size * 1.2f), 0.0f, 1.0f);

                accum += color * (brightness * distFade * fadeVal * intensity);
            }

            pixels[i] += accum * transition.getValue();
        }
    }
};
float FireworkBurstPattern::burstCenterX = 0.0f;
float FireworkBurstPattern::burstCenterY = 0.0f;
BeatWatcher FireworkBurstPattern::watcher;
FadeDown FireworkBurstPattern::fade = FadeDown(1500);

class MeteorShowerPattern : public Pattern<RGBA>
{
    static const int MAX_METEORS = 30;
    Transition transition = Transition(300, 1000);
    PixelMap *map;
    LFO<SawUp> meteorLFOs[MAX_METEORS];
    LFO<SinFast> strobeLFO;
    static float angles[MAX_METEORS];

public:
    MeteorShowerPattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Plasma orb";

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

float MeteorShowerPattern::angles[MAX_METEORS] = {0};





}