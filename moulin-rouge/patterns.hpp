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