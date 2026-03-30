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

}