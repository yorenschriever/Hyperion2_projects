#include "generation/patterns/pattern.hpp"

class OneColor : public Pattern<RGBA>
{
public:
    RGBA color;
    OneColor(RGBA color, const char *name)
    {
        this->color = color;
        this->name = name;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < width; i++)
        {
            pixels[i] = color;
        }
    }
};

class Base : public Pattern<RGBA>
{
public:
    RGBA color;
    PixelMap3d *map;
    Base(PixelMap3d *map, RGBA color, const char *name)
    {
        this->color = color;
        this->name = name;
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < width; i++)
        {
            pixels[i] = color * (-0.5*map->y(i)); //fade out based on distance to create a nice depth effect
        }
    }
};

class Wings : public Pattern<RGBA>
{
public:
    RGBA color;
    PixelMap3d::Cylindrical *map;
    Wings(PixelMap3d::Cylindrical *map, RGBA color, const char *name)
    {
        this->color = color;
        this->name = name;
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < width; i++)
        {
            pixels[i] = color * map->r(i); //fade out based on distance to create a nice depth effect
        }
    }
};

class RenderHeart
{
    static bool circle(float x, float y, float d)
    {
        return x * x + y * y < d * d;
    }
public:
    static bool heart(float x, float y)
    {
        return circle(x-0.5, y+0.5, 0.5) || 
               circle(x+0.5, y+0.5, 0.5) ||
               (y > -0.25 && std::abs(x*1.25) < 0.92-y) ||
               (std::abs(x) < 0.5 && y > -0.5 && y < 0.25);
    }

    static bool heartOutline(float x, float y)
    {
        return heart(x,y) && !heart(x*1.15, y*1.15);
    }
};

class Heart : public Pattern<RGBA>
{
public:
    PixelMap3d *map;
    Timeline timeline;
    BeatWatcher beat;
    FadeUp fade = FadeUp(250);

    Heart(PixelMap3d *map)
    {
        this->name = "Heart";
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        timeline.FrameStart();
        if (beat.Triggered(2))
        {
            timeline.reset();
        }

        if (timeline.Happened(0) || timeline.Happened(250))
        {
            fade.reset();
        }

        float size = 6 + fade.getValue() * 1;

        for (int i = 0; i < width; i++)
        {
            float x = map->x(i)*size;
            float y = (-1*map->y(i)-0.5)*size;
            auto color = params->getPrimaryColor();
            pixels[i] = RenderHeart::heart(x, y) ? color : RGBA(0, 0, 0, 0);
        }
    }
};


class HeartZoom : public Pattern<RGBA>
{
public:
    PixelMap3d *map;
    BeatWatcher beat;
    FadeDown fade = FadeDown(500);

    HeartZoom(PixelMap3d *map)
    {
        this->name = "Heart zoom";
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        if (beat.Triggered(1))
        {
            fade.reset();
        }

        float size = 12 * fade.getValue() ;

        for (int i = 0; i < width; i++)
        {
            float x = map->x(i)*size;
            float y = (-1*map->y(i)-0.25)*size;
            auto color = params->getPrimaryColor();
            pixels[i] = RenderHeart::heartOutline(x, y) ? color : RGBA(0, 0, 0, 0);
        }
    }
};