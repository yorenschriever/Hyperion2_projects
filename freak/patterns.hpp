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
    PixelMap::Polar *map;   
public:
    RadialGradient(PixelMap::Polar *map)
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
    PixelMap *map;   
public:
    LinearGradient(PixelMap *map)
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
    PixelMap *map;

public:
    OnLetterChaseCenterPattern(PixelMap *map)
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
    PixelMap *map;

public:
    OnLetterChaseUpPattern(PixelMap *map)
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