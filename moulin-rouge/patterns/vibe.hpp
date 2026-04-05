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


class InfernoParticles : public Pattern<RGBA>
{
    static const int MAX_PARTICLES = 5000;

    struct Particle
    {
        float x;
        float y;
        float vx;
        float vy;
        float life;
        float maxLife;
        float size;
        bool alive;
    };

    static Particle particles[MAX_PARTICLES];
    PixelMap *map;
    SpatialGrid grid;
    std::vector<int> candidates;
    Transition transition = Transition(300, 2000);
    Timeline time;

    void spawnParticle(int i, float spread)
    {
        particles[i].x = (Utils::random_f() - 0.5f) * spread * 1.4f;
        particles[i].y = -1.0f * (1.0f + Utils::random_f() * 0.3f);
        particles[i].vx = (Utils::random_f() - 0.5f) * 0.3f;
        particles[i].vy = (Utils::random_f() * 0.8f + 0.4f);
        particles[i].life = 0.f;
        particles[i].maxLife = Utils::random_f() * 1.5f + 0.5f;
        particles[i].size = Utils::random_f() * 0.06f + 0.03f;
        particles[i].alive = true;
    }

public:
    InfernoParticles(PixelMap *map)
    {
        this->name = "Inferno";
        this->map = map;
        grid.build(map);
        for (int i = 0; i < MAX_PARTICLES; i++)
            particles[i].alive = false;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        time.FrameStart();
        float dt = time.GetDelta() / 1000.0f;
        dt = std::min(dt, 0.05f);

        float velocity = params->getVelocity(0.5f, 3.0f);
        int amount = params->getAmount(500, MAX_PARTICLES);
        float sizeParam = params->getSize(1.8f, 0.4f);
        float spread = params->getVariant(0.3f, 2.0f);
        float intensity = params->getIntensity(0.5f, 1.5f);

        int aliveCount = 0;
        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (particles[i].alive)
                aliveCount++;
        }

        int toSpawn = amount - aliveCount;
        for (int i = 0; i < MAX_PARTICLES && toSpawn > 0; i++)
        {
            if (!particles[i].alive)
            {
                spawnParticle(i, spread);
                toSpawn--;
            }
        }

        for (int i = 0; i < MAX_PARTICLES; i++)
        {
            if (!particles[i].alive)
                continue;

            particles[i].life += dt;
            if (particles[i].life >= particles[i].maxLife)
            {
                particles[i].alive = false;
                continue;
            }

            particles[i].vy += 0.1f * dt;
            particles[i].vx += (Utils::random_f() - 0.5f) * 2.0f * dt;

            particles[i].x += particles[i].vx * velocity * dt;
            particles[i].y += particles[i].vy * velocity * dt;
        }

        float transVal = transition.getValue();

        for (int p = 0; p < MAX_PARTICLES; p++)
        {
            if (!particles[p].alive)
                continue;

            float lifeRatio = particles[p].life / particles[p].maxLife;
            float brightness = (1.0f - lifeRatio);
            brightness *= brightness;
            brightness *= intensity;

            if (brightness < 0.01f)
                continue;

            float radius = particles[p].size * sizeParam * (1.0f + lifeRatio * 0.5f);

            float px = particles[p].x;
            float py = particles[p].y;

            RGBA color = params->getGradientf(1-lifeRatio);

            grid.getPixelsInRange(px, py, radius, candidates);

            for (int idx : candidates)
            {
                float dx = map->x(idx) - px;
                float dy = map->y(idx) - py;
                float distSq = dx * dx + dy * dy;
                float radiusSq = radius * radius;

                if (distSq < radiusSq)
                {
                    float dist = sqrtf(distSq);
                    float falloff = 1.0f - (dist / radius);
                    falloff *= falloff;
                    pixels[idx] += color * (falloff * brightness * transVal);
                }
            }
        }
    }
};

InfernoParticles::Particle InfernoParticles::particles[MAX_PARTICLES] ;


class BeerBubblesPattern : public Pattern<RGBA>
{
    static const int MAX_BUBBLES = 100;

    struct Bubble
    {
        float x;
        float y;
        float speed;
        float size;
        float wobblePhase;
        float wobbleAmount;
        bool alive;
    };

    Bubble bubbles[MAX_BUBBLES];
    PixelMap *map;
    Transition transition = Transition(500, 1500);
    Timeline time;
    SpatialGrid grid;
    std::vector<int> candidates;

    void spawnBubble(int index)
    {
        bubbles[index].x = Utils::random_f() * 2.f - 1.f;
        bubbles[index].y = -1.2f;
        bubbles[index].speed = Utils::random_f() * 0.3f + 0.2f;
        bubbles[index].size = Utils::random_f() * 0.08f + 0.03f;
        bubbles[index].wobblePhase = Utils::random_f() * 6.28f;
        bubbles[index].wobbleAmount = Utils::random_f() * 0.05f + 0.02f;
        bubbles[index].alive = true;
    }

public:
    BeerBubblesPattern(PixelMap *map)
    {
        this->name = "Beer bubbles";
        this->map = map;
        grid.build(map);
        for (int i = 0; i < MAX_BUBBLES; i++)
            bubbles[i].alive = false;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        time.FrameStart();

        float velocity = params->getVelocity(0.3f, 2.0f);
        int amount = params->getAmount(2, MAX_BUBBLES);
        float bubbleSize = params->getSize(1.5f, 0.3f);
        float timeStep = time.GetDelta() / 1000.0f;

        int activeBubbles = 
            std::count_if(std::begin(bubbles), std::end(bubbles), [](const Bubble &b)
                         { return b.alive; });
        int toSpawn = amount - activeBubbles;
        for (int i = 0; i < MAX_BUBBLES && toSpawn > 0; i++)
        {
            if (!bubbles[i].alive)
            {
                spawnBubble(i);
                toSpawn--;
            }
        }

        for (int i = 0; i < MAX_BUBBLES; i++)
        {
            if (!bubbles[i].alive) continue;
            bubbles[i].y += bubbles[i].speed * velocity * timeStep;
            bubbles[i].wobblePhase += timeStep * 4.0f;
            if (bubbles[i].y > 1.3f)
                bubbles[i].alive = false;
        }

        float transVal = transition.getValue();

        for (int b = 0; b < MAX_BUBBLES; b++)
        {
            if (!bubbles[b].alive) continue;

            float bx = bubbles[b].x + sinf(bubbles[b].wobblePhase) * bubbles[b].wobbleAmount;
            float by = bubbles[b].y;
            float radius = bubbles[b].size * bubbleSize;

            float gradientVal = Utils::rescale_c(by, 0.0f, 1.0f, -1.0f, 1.0f);
            RGBA color = params->getGradientf(gradientVal);

            grid.getPixelsInRange(bx, by, radius, candidates);

            for (int p : candidates)
            {
                float dx = map->x(p) - bx;
                float dy = map->y(p) - by;
                float distSq = dx * dx + dy * dy;
                float radiusSq = radius * radius;

                if (distSq < radiusSq)
                {
                    float dist = sqrtf(distSq);
                    float brightness = 1.0f - (dist / radius);
                    brightness = brightness * brightness;
                    pixels[p] += color * brightness * transVal;
                }
            }
        }
    }
};


class FloatingOrbsPattern : public Pattern<RGBA>
{
    static const int MAX_ORBS = 64;

    struct Orb
    {
        float x, y;
        float vx, vy;
        float baseSize;
        float glowPhase;
        float glowSpeed;
        bool alive;
    };

    Orb orbs[MAX_ORBS];
    PixelMap *map;
    SpatialGrid grid;
    std::vector<int> candidates;
    Transition transition = Transition(800, 1500);
    Timeline time;

    void spawnOrb(int i)
    {
        orbs[i].x = Utils::random_f() * 2.f - 1.f;
        orbs[i].y = Utils::random_f() * 2.f - 1.f;
        orbs[i].vx = (Utils::random_f() - 0.5f) * 0.4f;
        orbs[i].vy = (Utils::random_f() - 0.5f) * 0.4f;
        orbs[i].baseSize = Utils::random_f() * 0.06f + 0.04f;
        orbs[i].glowPhase = Utils::random_f() * 6.2832f;
        orbs[i].glowSpeed = Utils::random_f() * 2.f + 1.5f;
        orbs[i].alive = true;
    }

    void bounceOrb(Orb &o)
    {
        if (o.x < -1.3f) { o.x = -1.3f; o.vx = fabsf(o.vx); }
        if (o.x > 1.3f)  { o.x = 1.3f;  o.vx = -fabsf(o.vx); }
        if (o.y < -1.3f) { o.y = -1.3f; o.vy = fabsf(o.vy); }
        if (o.y > 1.3f)  { o.y = 1.3f;  o.vy = -fabsf(o.vy); }
    }

public:
    FloatingOrbsPattern(PixelMap *map)
    {
        this->name = "Floating orbs";
        this->map = map;
        grid.build(map);
        for (int i = 0; i < MAX_ORBS; i++)
            orbs[i].alive = false;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        time.FrameStart();
        float dt = time.GetDelta() / 1000.f;

        float velocity = params->getVelocity(0.2f, 2.5f);
        int amount = params->getAmount(3, MAX_ORBS);
        float sizeParam = params->getSize(1.8f, 0.4f);
        float intensity = params->getIntensity(0.5f, 1.0f);
        float offset = params->getOffset(0.f, 3.f);

        int aliveCount = 0;
        for (int i = 0; i < MAX_ORBS; i++)
            if (orbs[i].alive) aliveCount++;

        int toSpawn = amount - aliveCount;
        for (int i = 0; i < MAX_ORBS && toSpawn > 0; i++)
        {
            if (!orbs[i].alive)
            {
                spawnOrb(i);
                toSpawn--;
            }
        }

        int toKill = aliveCount - amount;
        for (int i = MAX_ORBS - 1; i >= 0 && toKill > 0; i--)
        {
            if (orbs[i].alive)
            {
                orbs[i].alive = false;
                toKill--;
            }
        }

        for (int i = 0; i < MAX_ORBS; i++)
        {
            if (!orbs[i].alive) continue;
            orbs[i].x += orbs[i].vx * velocity * dt;
            orbs[i].y += orbs[i].vy * velocity * dt;
            orbs[i].glowPhase += orbs[i].glowSpeed * dt;
            bounceOrb(orbs[i]);
        }

        float transVal = transition.getValue();

        for (int b = 0; b < MAX_ORBS; b++)
        {
            if (!orbs[b].alive) continue;

            float glowMul = 0.5f + 0.5f * sinf(orbs[b].glowPhase + offset * (float)b);
            float radius = orbs[b].baseSize * sizeParam * (0.7f + 0.3f * glowMul);

            float gradVal = Utils::rescale_c((float)b / (float)amount, 0.f, 1.f, 0.f, 1.f);
            RGBA color = params->getGradientf(gradVal);

            grid.getPixelsInRange(orbs[b].x, orbs[b].y, radius, candidates);

            for (int p : candidates)
            {
                float dx = map->x(p) - orbs[b].x;
                float dy = map->y(p) - orbs[b].y;
                float distSq = dx * dx + dy * dy;
                float radiusSq = radius * radius;

                if (distSq < radiusSq)
                {
                    float dist = sqrtf(distSq);
                    float brightness = 1.f - (dist / radius);
                    brightness = brightness * brightness * brightness;
                    brightness *= glowMul * intensity;
                    brightness = std::min(brightness * 1.5f, 1.f);
                    pixels[p] += color * brightness * transVal;
                }
            }
        }
    }
};


class MandalaZoom : public Pattern<RGBA>
{
    PixelMap::Polar *map;
    Transition transition = Transition(400, 1500);
    LFO<SawDown> zoomLfo;

public:
    MandalaZoom(PixelMap *map)
    {
        this->map = map->toPolar();
        this->name = "Mandala Zoom";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float velocity = params->getVelocity(8000, 1000);
        zoomLfo.setPeriod(velocity);

        int petals = params->getAmount(3, 12);
        float size = params->getSize(1.5f, 0.3f);
        float variant = params->getVariant();
        float offset = params->getOffset(0.0f, 3.14159f);

        float zoom = zoomLfo.getValue();
        float expanding = zoom * size * 3.0f;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float r = map->r(i);
            float th = map->th(i);

            float petalWave = (sinf(petals * th + offset) + 1.0f) * 0.5f;

            float secondaryWave = (sinf(petals * 2.0f * th + offset * 2.0f) + 1.0f) * 0.5f;
            float combined = petalWave * 0.7f + secondaryWave * 0.3f * variant;

            float zoomedR = r + expanding;
            float mandalaValue = sinf(zoomedR * 6.28318f * 2.0f) * 0.5f + 0.5f;

            float ring1 = fmodf(zoomedR * 3.0f, 1.0f);
            float ringPattern = (sinf(ring1 * 6.28318f) + 1.0f) * 0.5f;

            float pattern = combined * ringPattern * mandalaValue;

            float distanceFade = 1.0f - r * 0.4f;
            distanceFade = Utils::constrain_f(distanceFade, 0.0f, 1.0f);

            float gradientPos = fmodf(zoomedR * 2.0f + combined * 0.5f, 1.0f);
            RGBA color = params->getGradientf(gradientPos);

            float brightness = pattern * distanceFade;
            brightness = brightness * brightness;
            brightness = Utils::constrain_f(brightness * 1.5f, 0.0f, 1.0f);

            pixels[i] = color * brightness * transition.getValue();
        }
    }
};

class HeartStackPattern : public Pattern<RGBA>
{
    PixelMap *map;
    Transition transition = Transition(400, 800);
    LFO<SinFast> lfoY;

    float heartDistance(float px, float py, float cx, float cy, float scale)
    {
        float x = (px - cx) / scale;
        float y = (py - cy) / scale;
        y = -y;
        y -= 0.4f;

        float x2 = x * x;
        float y2 = y * y;
        float val = x2 + y2 - 1.0f;
        float heart = val * val * val - x2 * y2 * y;
        
        float dx = 0.01f;
        float xp = x + dx;
        float xp2 = xp * xp;
        float valp = xp2 + y2 - 1.0f;
        float heartp = valp * valp * valp - xp2 * y2 * y;

        float dy = 0.01f;
        float yp = y + dy;
        float yp2 = yp * yp;
        float valpy = x2 + yp2 - 1.0f;
        float heartpy = valpy * valpy * valpy - x2 * yp2 * yp;

        float gradX = (heartp - heart) / dx;
        float gradY = (heartpy - heart) / dy;
        float gradLen = sqrtf(gradX * gradX + gradY * gradY);
        if (gradLen < 0.0001f) gradLen = 0.0001f;

        return heart / gradLen;
    }

public:
    HeartStackPattern(PixelMap *map)
    {
        this->name = "Heart stack";
        this->map = map;
        lfoY.setPeriod(3000);
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        int numHearts = params->getAmount(2, 6);
        float velocity = params->getVelocity(8000, 1500);
        float size = params->getSize(0.4f, 0.1f);
        float lineWidth = 0.08f + params->getIntensity(0.02f, 0.12f);
        float offset = params->getOffset(0.0f, 1.0f);

        lfoY.setPeriod(velocity);

        int mapSize = std::min(width, (int)map->size());

        for (int h = 0; h < numHearts; h++)
        {
            float heartPhase = (float)h / numHearts;
            float phaseOffset = heartPhase * offset;

            float yOff = lfoY.getValue(heartPhase * 0.3f + phaseOffset) * 0.6f - 0.3f;
            float xOff = lfoY.getValue(heartPhase * 0.3f + phaseOffset + 0.25f) * 0.4f - 0.2f;

            float heartScale = size * (0.7f + 0.3f * heartPhase);

            RGBA color = params->getGradientf(heartPhase);

            for (int i = 0; i < mapSize; i++)
            {
                float dist = heartDistance(map->x(i), -map->y(i)-0.6, xOff, yOff, heartScale);

                float absDist = fabsf(dist);
                float brightness = 1.0f - Utils::constrain_f(absDist / lineWidth, 0.0f, 1.0f);
                brightness = brightness * brightness;

                if (brightness > 0.01f)
                {
                    pixels[i] += color * brightness * transition.getValue();
                }
            }
        }
    }
};

class BurningTornadoPattern : public Pattern<RGBA>
{
    static const int NUM_EMBERS = 60;
    
    struct Ember {
        float angle;
        float height;
        float radius;
        float speed;
        float riseSpeed;
        float brightness;
    };
    
    Ember embers[NUM_EMBERS];
    PixelMap *map;
    Transition transition = Transition(400, 1500);
    LFO<SawUp> timeLfo;
    unsigned long frameCount = 0;

    void initEmber(Ember &e, bool randomHeight)
    {
        e.angle = Utils::random_f() * 2.0f * M_PI;
        e.height = randomHeight ? Utils::random_f() * 2.0f - 1.0f : -1.0f + Utils::random_f() * 0.3f;
        e.radius = 0.1f + Utils::random_f() * 0.3f;
        e.speed = 0.5f + Utils::random_f() * 1.5f;
        e.riseSpeed = 0.003f + Utils::random_f() * 0.008f;
        e.brightness = 0.5f + Utils::random_f() * 0.5f;
    }

public:
    BurningTornadoPattern(PixelMap *map)
    {
        this->map = map;
        this->name = "Burning Tornado";
        timeLfo.setPeriod(10000);
        
        for (int i = 0; i < NUM_EMBERS; i++)
            initEmber(embers[i], true);
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float velocity = params->getVelocity(0.3f, 3.0f);
        float size = params->getSize(0.15f, 0.04f);
        int amount = params->getAmount(20, NUM_EMBERS);
        float intensity = params->getIntensity(0.5f, 1.0f);

        frameCount++;

        for (int i = 0; i < amount; i++)
        {
            embers[i].angle += embers[i].speed * velocity * 0.05f;
            if (embers[i].angle > 2.0f * M_PI)
                embers[i].angle -= 2.0f * M_PI;

            embers[i].height += embers[i].riseSpeed * velocity;

            if (embers[i].height > 1.2f)
                initEmber(embers[i], false);
        }

        int mapSize = std::min(width, (int)map->size());

        for (int pi = 0; pi < mapSize; pi++)
        {
            float px = map->x(pi);
            float py = map->y(pi);

            RGBA accumulated = RGBA(0, 0, 0, 0);

            for (int i = 0; i < amount; i++)
            {
                float heightNorm = (embers[i].height + 1.0f) / 2.0f;
                float funnelWidth = 0.15f + heightNorm * 0.6f;
                float emberRadius = embers[i].radius * funnelWidth;

                float emberX = cosf(embers[i].angle) * emberRadius;
                float emberY = embers[i].height;

                float dx = px - emberX;
                float dy = py - emberY;
                float dist = sqrtf(dx * dx + dy * dy);

                if (dist < size)
                {
                    float falloff = 1.0f - dist / size;
                    falloff *= falloff;

                    float heat = 1.0f - heightNorm * 0.7f;
                    float gradientVal = Utils::constrain_f(heat, 0.0f, 1.0f);

                    RGBA color = params->getGradientf(1.0f - gradientVal);

                    float alpha = falloff * embers[i].brightness * intensity;
                    alpha = Utils::constrain_f(alpha, 0.0f, 1.0f);

                    float fadeBrightness = 1.0f - Utils::constrain_f((heightNorm - 0.7f) / 0.3f, 0.0f, 1.0f);

                    accumulated += color * (alpha * fadeBrightness);
                }
            }

            pixels[pi] += accumulated * transition.getValue();
        }
    }
};

class EmberTornado : public Pattern<RGBA>
{
    PixelMap::Polar *map;
    Transition transition = Transition(400, 1500);
    LFO<SawUp> spinLfo;
    LFO<SinFast> radiusPulse;
    Permute perm;

public:
    EmberTornado(PixelMap *map)
    {
        this->name = "Ember Tornado";
        this->map = map->toPolar();
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float velocity = params->getVelocity(8000, 800);
        float size = params->getSize(0.6, 0.08);
        int arms = params->getAmount(1, 5);
        float offset = params->getOffset(0, 1);
        float intensity = params->getIntensity(0.4, 1.0);

        spinLfo.setPeriod(velocity);
        radiusPulse.setPeriod(velocity * 1.7);
        perm.setSize(width);

        float spin = spinLfo.getValue();

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float r = map->r(i);
            float th = map->th(i) / (2.0f * M_PI);

            float spiralAngle = th - spin * arms + r * 1.5f + offset;
            spiralAngle = spiralAngle * arms;
            spiralAngle = spiralAngle - floorf(spiralAngle);

            float emberWidth = size * (0.3f + 0.7f * (1.0f - r));

            float dist = (spiralAngle < 0.5f) ? spiralAngle : (1.0f - spiralAngle);
            dist *= 2.0f;

            if (dist > emberWidth)
                continue;

            float coreBrightness = 1.0f - (dist / emberWidth);
            coreBrightness = coreBrightness * coreBrightness;

            float radialFade = 0.3f + 0.7f * r;

            float flicker = radiusPulse.getValue(th + r * 0.5f + float(perm.at[i % width]) / width * 0.3f);
            flicker = 0.6f + 0.4f * flicker;

            float alpha = coreBrightness * radialFade * flicker * intensity;
            alpha = Utils::constrain_f(alpha, 0.0f, 1.0f);

            float gradientPos = Utils::constrain_f((1.0f - r) * 0.5f + coreBrightness * 0.5f, 0.0f, 1.0f);
            RGBA color = params->getGradientf(gradientPos);

            RGBA highlight = params->getHighlightColor() * (coreBrightness * coreBrightness * coreBrightness * 0.5f);

            pixels[i] += (color * alpha + highlight * alpha) * transition.getValue();
        }
    }
};


}