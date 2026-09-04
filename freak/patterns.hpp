#include "generation/patterns/pattern.hpp"

class SinChase : public Pattern<RGBA>
{
    Transition transition;
    LFOTempo<NegativeCosFast> lfo;

public:
    SinChase()
    {
        this->name = "Sin chase";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        lfo.setDutyCycle(params->getSize(0.06, 1));
        int amount = params->getAmount(1, 19.99);

        for (int i = 0; i < width; i++)
        {
            pixels[i] = params->getPrimaryColor() * lfo.getValue(amount * float(i) / width) * transition.getValue();
        }
    }
};

class RadialGradient: public Pattern<RGBA>
{
    Transition transition;
    PixelMap::PolarPtr map;   
public:
    RadialGradient(PixelMap::PolarPtr map)
    {
        this->map = map;
        this->name = "Radial gradient";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int index = 0; index < std::min(width, (int)map->size()); index++)
        {
            RGBA color = params->getGradient((1.-map->r(index)) * 255);
            pixels[index] = color * transition.getValue();
        }
    }
};

class LinearGradient: public Pattern<RGBA>
{
    Transition transition;
    PixelMapPtr map;   
public:
    LinearGradient(PixelMapPtr map)
    {
        this->map = map;
        this->name = "Linear gradient";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int index = 0; index < std::min(width, (int)map->size()); index++)
        {
            RGBA color = params->getGradient(Utils::rescale_c(map->y(index),0,255,-0.25,0.25));
            pixels[index] = color * transition.getValue();
        }
    }
};

class SawChasePattern : public Pattern<RGBA>
{
    Transition transition;
    LFO<SoftSawDown> lfo;

public:
    SawChasePattern()
    {
        this->name = "Saw chase";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        lfo.setDutyCycle(params->getSize(0.1, 0.5));
        lfo.setPeriod(params->getVelocity(4000, 500));
        int amount = int(params->getAmount(1, 10))*5;

        for (int i = 0; i < width; i++)
        {
            float phase = ((float)i / width) * amount;
            pixels[i] = params->getSecondaryColor() * lfo.getValue(phase) * transition.getValue();
        }
    }
};

class OnLetterChaseCenterPattern : public Pattern<RGBA>
{
    Transition transition;
    FadeDown fade[5];
    int currentLetter = 0;
    BeatWatcher watcher;
    PixelMapPtr map;

public:
    OnLetterChaseCenterPattern(PixelMapPtr map)
    {
        this->map = map;
        this->name = "On letter chase center";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered()){
            currentLetter = (currentLetter + 1) % 5;
            fade[currentLetter].reset();
        }

        float velocity = params->getVelocity(1000, 100);
        float size = params->getSize(0.1, 1);

        for (int i = 0; i < 5; i++){
            fade[i].setDuration(velocity * size);
        }

        for (int index = 0; index < width; index++)
        {
            int letter = index / 50;

            // pixels[index] = params->getPrimaryColor() * fade[letter].getValue(Utils::rescale(map->y(index), 0, velocity, 0.25, -0.25)) * transition.getValue();
            pixels[index] = params->getPrimaryColor() * fade[letter].getValue(abs(Utils::rescale(map->y(index), -velocity, velocity, 0.25, -0.25))) * transition.getValue();
        }
    }
};

class OnLetterChaseUpPattern : public Pattern<RGBA>
{
    Transition transition;
    FadeDown fade[5];
    int currentLetter = 0;
    BeatWatcher watcher;
    PixelMapPtr map;

public:
    OnLetterChaseUpPattern(PixelMapPtr map)
    {
        this->map = map;
        this->name = "On letter chase up";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered()){
            currentLetter = (currentLetter + 1) % 5;
            fade[currentLetter].reset();
        }

        float velocity = params->getVelocity(1000, 100);
        float size = params->getSize(0.1, 1);

        for (int i = 0; i < 5; i++){
            fade[i].setDuration(velocity * size);
        }

        for (int index = 0; index < width; index++)
        {
            int letter = index / 50;

            pixels[index] = params->getPrimaryColor() * fade[letter].getValue(Utils::rescale(map->y(index), 0, velocity, 0.25, -0.25)) * transition.getValue();
            // pixels[index] = params->getPrimaryColor() * fade[letter].getValue(abs(Utils::rescale(map->y(index), -velocity, velocity, 0.25, -0.25))) * transition.getValue();
        }
    }
};

class PulseRing : public Pattern<RGBA>
{
public:
    FadeDown fade;
    BeatWatcher watcher;
    PixelMapPtr map;

    PulseRing(PixelMapPtr map)
    {
        this->name = "Pulse Ring";
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        if (watcher.Triggered())
            fade.reset();

        float radius = 1.0f - fade.getValue();

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float x = map->x(i);
            float y = map->y(i);
            float dist = std::sqrt(x * x + y * y);
            float diff = std::abs(dist - radius);
            float brightness = std::max(0.0f, 1.0f - diff * 5.0f);
            pixels[i] = params->getGradient(dist) * (brightness * fade.getValue());
        }
    }
};

class TriangleBurst : public Pattern<RGBA>
{
    PixelMapPtr map;
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
    TriangleBurst(PixelMapPtr map)
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
    PixelMapPtr map;
    float size = 0.0f;
    int lastTimestamp = 0;

public:
    RotatingTriangle(PixelMapPtr map)
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


class ExpandingTriangle : public Pattern<RGBA>
{
    Transition transition;
    FadeDown resetFade = FadeDown(150, WaitAtEnd);
    LFO<SinFast> growLfo;
    LFO<SawUp> rotateLfo;
    BeatWatcher watcher;
    int beatCount = 0;
    float growPhase = 0.0f;

public:
    ExpandingTriangle(PixelMapPtr map)
    {
        this->name = "Expanding Triangle";
        this->map = map;
        rotateLfo.setPeriod(8000);
        growLfo.setPeriod(1);
    }

    PixelMapPtr map;

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        if (watcher.Triggered())
        {
            beatCount = (beatCount + 1) % 2;
            if (beatCount == 0)
            {
                resetFade.reset();
            }
        }

        float fadeOut = resetFade.getValue();
        bool inReset = fadeOut > 0.001f;

        float rawGrow = rotateLfo.getPhase();

        if (inReset)
            growPhase = 0.0f;

        float grow;
        if (beatCount == 1)
            grow = Utils::rescale(rotateLfo.getPhase(), 0.0f, 1.0f, 0.0f, 1.0f);
        else
            grow = growPhase;

        if (!inReset && beatCount == 0)
            grow = 1.0f;

        float rotation = rotateLfo.getValue() * 2.0f * M_PI;
        float maxRadius = 1.0f;
        float outlineWidth = 0.05f;

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
            if (inReset)
                visibility *= (1.0f - fadeOut);

            pixels[i] += params->getSecondaryColor() * (alpha * visibility);
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

        return minEdgeDist - 0.001f;
    }
};


class ExpandingTriangle2 : public Pattern<RGBA>
{
    Transition transition;
    FadeDown resetFade = FadeDown(150, WaitAtEnd);
    LFOTempo<SawUp> growLfo;
    LFO<SawUp> rotateLfo;
    float lastPhase = 0.0f;

public:
    ExpandingTriangle2(PixelMapPtr map)
    {
        this->name = "Expanding Triangle";
        this->map = map;
        growLfo.setPeriod(2);
        rotateLfo.setPeriod(8000);
    }

    PixelMapPtr map;

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float phase = growLfo.getPhase();

        if (phase < lastPhase)
            resetFade.reset();

        lastPhase = phase;

        float grow = phase;
        float fadeOut = resetFade.getValue();
        float rotation = rotateLfo.getValue() * 2.0f * M_PI;
        float maxRadius = 1.0f;
        float outlineWidth = 0.05f;

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

            float visibility = transition.getValue() * (1.0f - fadeOut);
            pixels[i] += params->getSecondaryColor() * (alpha * visibility);
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

        return minEdgeDist - 0.001f;
    }
};

class TrianglePulse : public Pattern<RGBA>
{
    PixelMapPtr map;
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
    TrianglePulse(PixelMapPtr map)
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
    PixelMap::PolarPtr map;
    Transition transition = Transition(200, 1000);
    LFO<SawUp> rotationLfo;
    BeatWatcher watcher;
    FadeUp growFade = FadeUp(1000);

public:
    TriangleOutlineGrow(PixelMap::PolarPtr map)
    {
        this->map = map;
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

class LetterStrobeFlash : public Pattern<RGBA>
{
    Transition transition;
    BeatWatcher watcher;
    FadeDown fade = FadeDown(500);
    int currentLetter = 0;
    int framecounter = 0;

public:
    LetterStrobeFlash()
    {
        this->name = "Letter Strobe Flash";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        int numLetters = 5;
        int strobeRate = params->getVelocity(6, 2);
        fade.setDuration(params->getSize(800, 150));

        if (watcher.Triggered())
        {
            currentLetter = (currentLetter + 1) % numLetters;
            fade.reset();
            framecounter = 0;
        }

        framecounter++;

        bool strobeOn = (framecounter % strobeRate) < (strobeRate / 2 + 1);

        float fadeVal = fade.getValue();

        if (fadeVal <= 0.01f)
            return;

        int letterSize = width / numLetters;

        int startIdx = currentLetter * letterSize;
        int endIdx = (currentLetter == numLetters - 1) ? width : startIdx + letterSize;

        RGBA color = params->getSecondaryColor();

        for (int i = startIdx; i < endIdx; i++)
        {
            if (strobeOn)
                pixels[i] = color * fadeVal * transition.getValue();
        }
    }
};

class CosmicJellyfish : public Pattern<RGBA>
{
    Transition transition = Transition(800, 2000);
    PixelMap::PolarPtr map;
    LFO<SinFast> breathLfo;
    LFO<SawDown> tentacleLfo;
    LFO<SinFast> wobbleLfo;
    FadeDown pulse = FadeDown(1200);
    BeatWatcher watcher;

public:
    CosmicJellyfish(PixelMap::PolarPtr map)
    {
        this->map = map;
        this->name = "Cosmic Jellyfish";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        float velocity = params->getVelocity(8000, 1500);
        float size = params->getSize(0.9, 0.3);
        float amount = params->getAmount(3, 12);
        float variant = params->getVariant();
        float offset = params->getOffset(0.0, 0.5);
        float intensity = params->getIntensity(0.3, 1.0);

        breathLfo.setPeriod(velocity);
        tentacleLfo.setPeriod(velocity * 0.7);
        wobbleLfo.setPeriod(velocity * 1.3);

        if (watcher.Triggered())
            pulse.reset();

        pulse.setDuration(velocity * 0.8);

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            float r = map->r(i);
            float th = map->th(i);

            float normalizedAngle = th / (2.0f * M_PI);

            float tentacleWave = tentacleLfo.getValue(normalizedAngle * amount + offset);
            float wobble = wobbleLfo.getValue(normalizedAngle * 2.0f + 0.3f) * 0.15f * variant;

            float bodyRadius = size * (0.6f + 0.4f * breathLfo.getValue());
            float bodyEdge = bodyRadius + wobble;

            float brightness = 0.0f;

            if (r < bodyEdge * 0.5f)
            {
                float coreDist = r / (bodyEdge * 0.5f);
                brightness = 1.0f - coreDist * 0.3f;
                brightness += pulse.getValue() * 0.5f * (1.0f - coreDist);
                RGBA color = params->getHighlightColor() * brightness * intensity * transition.getValue();
                pixels[i] = color;
                continue;
            }
            else if (r < bodyEdge)
            {
                float shellPos = (r - bodyEdge * 0.5f) / (bodyEdge * 0.5f);
                brightness = (1.0f - shellPos * shellPos) * 0.8f;
                brightness += pulse.getValue() * 0.3f * (1.0f - shellPos);
                RGBA color = params->getGradient(shellPos) * brightness * intensity * transition.getValue();
                pixels[i] = color;
                continue;
            }

            float tentacleReach = bodyEdge + (1.0f - bodyEdge) * tentacleWave;
            if (r < tentacleReach && r >= bodyEdge)
            {
                float tentacleFalloff = (r - bodyEdge) / (tentacleReach - bodyEdge + 0.001f);
                brightness = (1.0f - tentacleFalloff) * 0.6f;

                float flicker = breathLfo.getValue(normalizedAngle * amount * 2.0f + r * 3.0f);
                brightness *= 0.5f + 0.5f * flicker;

                brightness += pulse.getValue() * 0.2f * (1.0f - tentacleFalloff);

                RGBA color = params->getGradient(tentacleFalloff * 0.7f + 0.3f) * brightness * intensity * transition.getValue();
                pixels[i] = color;
            }
        }
    }
};