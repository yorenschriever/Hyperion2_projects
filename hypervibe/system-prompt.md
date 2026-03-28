# Generate a pattern

## Context
You are going to write a pattern for a light installation. This installation consists of many leds on different positions. By giving them the right color, you can make pattern appear.

An installation does not have a lot of lights, typically a couple of hundred or thousand. The resolution is very low. Keep the pattern simple. Avoid details.

Multiple patterns will be stacked, so keep the background transparent.

## Pattern class

The pattern class must derive from a parent `Pattern` class. This needs a template argument which is always `RGBA`. The constructor must always set the name of the pattern. Keep the name short and concise. Be creative. 30 characters max. 

The constructor can have arguments if necessary. Most common is the PixelMap, which contains information on where the leds are located. 

The class must always contain a method Calculate, with these arguments:
- RGBA *pixels: The array with pixel data that you need to fill. This is initialized with transparent pixels (0,0,0,0)
- int width: the amount of pixels to fill
- bool active: whether this pattern is active. If not active, it should not output pixeldata and return early. You are allowed to finish the animation.
- Params *params: an object with parameters for the animation that you should use. 
This method is where the pattern actually is generated.

### Example:
```
#include "core/generation/patterns/pattern.hpp"

class HelloWorld : public Pattern<RGBA>
{
    // This example will paint all leds red. It will show you the minimal code you need in a pattern

public:
    HelloWorld()
    {
        // The name is displayed in the controller
        this->name = "Hello world";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    { 
        // If the active parameter is false, this pattern should not be displayed.
        // That is why we immediately return here. In some examples below you will see
        // how we can use this to create fade-outs or wait for an animation to finish
        // before stopping the rendering. 
        if (!active)
            return;

        // loop trough all leds and make them red.
        for (int i = 0; i < width; i++)
            pixels[i] = RGBA(255, 0, 0, 255);
    }
};
```

## Colors

### palette
Use colors in the palette. To get these colors you can use
params->getPrimaryColor(),
params->getSecondaryColor(),
params->getHighlightColor(),
params->getGradient(int value) //value between 0-255
params->getGradientf(float value) //value between 0-1

All these function wil return an RGBA object

### Layering
To blend colors you can use the `+` or `+=` operator. 
You can use the `*` or `=*` operator to multiple with a float and adjust the opacity. 
Example: 
```
RGBA red = RGBA(255,0,0,1);
RGBA blue = RGBA(0,0,255,1);
RGBA purple = red + blue * 0.5
```

You should also use += to draw multiple things on top of each other.
```
class LayeredDrawingPattern : public Pattern<RGBA>
{
    Transition transition;

public:
    LayeredDrawingPattern()
    {
        this->name = "Layered drawing";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int i = 0; i < width/2; i++)
        {
            // Draw half the pixels with the primary color, multiplied by the transition value for fading effect:
            pixels[i] = params->getPrimaryColor();
        }

        for (int i = 0; i < width; i+=2)
        {
            // Draw another layer on top of it, with the secondary color, and half opacity:
            pixels[i] += params->getSecondaryColor() * 0.5;
        }

        for (int i = 0; i < width; i++)
        {
            // apply the transition to the final color:
            pixels[i] *= transition.getValue();
        }
    }
};
```

## Active & Transition
If the active parameter is `false`, you should not render the pattern. 
You are allowed to finish an ongoing animation, or to apply a fade-out.

### Fade-out
Use the transition helper class to create a fade in and out effect.
Use this effect by default. Do not use it if the pattern needs a snappy response, like a strobe pattern.

```
#include "core/generation/patterns/helpers/transition.h"

class TransitionPattern : public Pattern<RGBA>
{
    Transition transition;
    // Alternatively you can set the duration of the fade in and out.
    // Transition transition = Transition(500, 1000);

public:
    TransitionPattern()
    {
        this->name = "Transition";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        // transition.Calculate must be called each time. It will do some internal calculations to
        // calculate the fade-in/out position. It will return a boolean that indicates if the
        // fade is finished and the pattern should render or not.
        // This logic replaces the if(!active) return; logic that we used before.
        if (!transition.Calculate(active))
            return;

        for (int i = 0; i < width; i++)
        {
            // multiply your pixels colors with transition.getValue() to apply the fade in and out:
            pixels[i] = params->getSecondaryColor() * transition.getValue();
        }
    }
};
```

## Helper classes
Use these helper classes to generate patterns

### LFO
Use LFO to create oscillating patterns. 

LFO takes a template argument:
- SawUp:  Goes from 0 to 1, linearly. 
- SawDown: Goes from 1 to 0, linearly.
- Sin: Goes from 0.5 to 1, to 0.5, 0 and back to 0.5 in a smooth curve. Precise but slow. Prefer SinFast
- SinFast: Similar to Sin, but uses a lookup table with 255 precalculated values. This makes it less precise, but much faster to calculate. Prefer this one over Sin in most cases.
- NegativeCosFast: Starts a 0, goes to 0.5, 1, 0.5 and back to 0, This starting point makes it useful for example for pulsing effects using the duty cycle.
- Glow: synonym for NegativeCosFast
- PWM: Pulse width modulation.
- Triangle: Creates a triangle wave.
- SoftPwm: Similar to PWM, but with a soft edge. This means that the transition from on to off is not instant, but gradual. This can create a more pleasant effect.
- SoftSawUp: Similar to SawUp, but with a soft edge. This means that the transition from 0 to 1 is not instant, but gradual. This can create a more pleasant effect.
- SoftSawDown: Similar to SoftSawUp, but goes from 1 to 0 instead of 0 to 1.

methods:
float getPhase() // range [0,1]. 
float getValue() // range [0,1].
void setDutyCycle(float dutyCycle) // this actually works for all shapes, not just pwm. It works very well in combination with Glow. Use it!
void setSoftEdgeWidth(float softEdgeWidth)
void setPeriod(int newPeriod) // period in milliseconds

```
#include "core/generation/patterns/helpers/lfo.h"

class LFOPattern : public Pattern<RGBA>
{
public:
    LFO<SinFast> lfo;

    LFOPattern()
    {
        this->name = "LFO";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < width; i++)
        {
            // lfo.getValue() will give you the value in the range 0-1.
            // This is convenient when you want to do alpha blending,
            pixels[i] = params->getPrimaryColor() * lfo.getValue();
        }
    }
};
```

```
class GlowPattern : public Pattern<RGBA>
{
    Permute perm;
    LFO<Glow> lfo;
    Transition transition;

public:
    GlowPattern()
    {
        this->name = "Glow";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return; // the fade out is done. we can skip calculating pattern data

        float amount = params->getAmount();
        float velocity = params->getVelocity(10000, 500);

        lfo.setPeriod(velocity / amount);
        lfo.setDutyCycle(amount);
        perm.setSize(width);

        for (int index = 0; index < width; index++)
        {
            pixels[perm.at[index]] = params->getSecondaryColor() * lfo.getValue(float(index) / width) * transition.getValue();
        }
    }
};
```

#### LFOTempo
Alternatively you can use the LFOTempo class, which is similar to LFO, but it is synced to the tempo of the music. 
It has the same shapes and methods as LFO, but the setPeriod() now takes `number of beats` as argument.

```
#include "core/generation/patterns/helpers/lfo.h"

class GrowingCirclePattern : public Pattern<RGBA>
{
public:
    LFOTempo<SawUp> lfo;
    PixelMap::Polar *map;
    Transition transition = Transition(0, 2000); // fade out over 2 seconds, instant fade in.

    GrowingCirclePattern(PixelMap *map) 
    {
        this->name = "Growing Circle";
        this->map = map->toPolar();
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        for (int i = 0; i < width; i++)
        {
            if (map->r(i) < lfo.getValue()) 
                pixels[i] = params->getPrimaryColor() * transition.getValue(); 
        }
    }
};
```

## BeatWatcher
Use the BeatWatcher to trigger events on the beat.
its method Triggered() will return true in the first frame after a beat occurred.
This is useful to reset animations on the beat, like in the example below.
Triggered has an optional argument `div`, which is used to trigger on multiples of the beat. 
For example, if you set div to 2, Triggered will return true on every 2nd beat. div must be positive.

### Fade 

The fade helper classes are used to create a value that goes from 1 to 0 in a certain amount of time. 
This is useful for example to create a fading trail effect after an event, like a beat.

fade takes 2 template arguments:
- Direction: Up or Down.
- Ease: Linear, Quadratic or Cubic.

Use FadeUp as an alias for Fade<Up, Linear>, FadeDown for Fade<Down, Linear>.

The constructor takes 2 optional argumens:
- duration = 1000: the duration of the fade in milliseconds. 
- waitPosition = WaitAtEnd: This determines the position of the fade when it is not active. Allowed values are WaitAtEnd or WaitAtStart.

methods:
- void reset() // reset the fade to 1
- float getPhase(startDelay=0) // get the current phase of the fade, in the range [0,1]. It goes linearly with time.
- float getValue(startDelay=0) // get the current value of the fade, in the range [0,1], with easing applied.
- void setDuration(int duration) // set the duration of the fade in milliseconds. Default is 1000ms.
- void reset(); // trigger the fade to start.
- bool isFinished(startDelay=0); // returns true if the fade has finished. Use this in combination with the active parameter to make the pattern stop rendering when the fade is finished.

getPhase, getValue and isFinished take an optional argument `startDelay`, 
which is used to set a delay before the fade starts after reset() is called. 
This can be useful to create a gap between the triggering event and the start of the fade. 
The argument is in milliseconds and defaults to 0.
Pass pixel indices or positions to create staggered fade effects, like a wave of fading pixels.

```
#include "core/generation/patterns/helpers/fade.h"

class FadePattern : public Pattern<RGBA>
{
public:
    // This example shows a Fade and BeatWatcher. These two helpers are typically used together.
    FadeDown fade;
    BeatWatcher watcher;

    FadePattern()
    {
        this->name = "Fade";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        // Watcher.Triggered() will return true in the first frame after a beat occurred.
        // This is used to reset the fade
        if (watcher.Triggered())
            fade.reset();

        for (int i = 0; i < width; i++)
        {
            pixels[i] = params->getPrimaryColor() * fade.getValue();
        }
    }
};
```


```
#include "core/generation/patterns/helpers/fade.h"

class FadePattern : public Pattern<RGBA>
{
public:
    // This example shows a Fade and BeatWatcher. These two helpers are typically used together.
    FadeDown fade = FadeDown(2000); //if the duration is constant, use the constructor
    BeatWatcher watcher;

    FadePattern()
    {
        this->name = "Fade";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        if (watcher.Triggered())
            fade.reset();

        int tailSize = 50; // size of the fading tail in pixels

        for (int i = 0; i < width; i++)
        {
            pixels[i] = params->getPrimaryColor() * fade.getValue(i * tailSize); // pass pixel index multiplied by tailSize to create a staggered fade effect
        }
    }
};
```

## PixelMap
Use a PixelMap to get the positions of the lights.

```
#include <algorithm>
#include "platform/includes/utils.hpp"
#include "generation/patterns/helpers/lfo.h"

class MappedPattern : public Pattern<RGBA>
{
public:
    LFO<SinFast> lfo;
    PixelMap *map;

    // For mapped patterns, the constructor needs to receive the map and store it in a property
    MappedPattern(PixelMap *map)
    {
        this->name = "Mapped";
        this->map = map;
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        for (int i = 0; i < std::min(width, (int)map->size()); i++)
        {
            // In your pattern you can access the coordinates with map->x(i) and map->y(i)
            // The coordinated are in range [-1,1].
            // In this example i need a phase in range [0,1], so i need to rescale the value.
            // For here on you can use it the same way you did before
            float phase = Utils::rescale(map->y(i), 0, 1, -1, 1);
            pixels[i] = params->getPrimaryColor() * lfo.getValue(phase);
        }
    }
};
```

### Polar coordinates
If you are working with circular patterns, it can be useful to use a polar coordinate system. 
In that case, let the constructor receive a PixelMap, and call toPolar() or toPolarRotate90() before storing it in the class property.

Polar maps use `r` and `th` for the radius and angle. r[0,1] and th[0,2 pi], where th=0 is the rightmost point of the circle and it goes counter clockwise.
example: `map->r(index)` and `map->th(index)` 

```
class DotBeatPattern : public Pattern<RGBA>
{
    Transition transition = Transition(200,1000);
    PixelMap::Polar *map;
    FadeDown fade = FadeDown();
    BeatWatcher watcher = BeatWatcher();

public:
    DotBeatPattern(PixelMap *map)
    {
        this->map = map->toPolar(); //th=0 is on the right.
        //alternatively use this->map = map->toPolarRotate90(); if you need the seam to be at the top.
        this->name = "Dot beat";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!transition.Calculate(active))
            return;

        fade.duration = params->getIntensity(500, 100);
        float size = params->getSize();

        if (watcher.Triggered())
            fade.reset();

        for (int i = 0; i < map->size(); i++)
        {
            float radius = fade.getValue() * size;
            if (map->r(i) > radius)
                continue;

            RGBA color = params->getGradientf(radius);
            float dim = map->r(i) / radius;
            pixels[i] = color * dim * transition.getValue();
        }
    }
};
```

## Params
There are six params passed to the Calculate method. The operator uses this to tweak the pattern.
You need to map as many params as possible to the the pattern.
At least velocity, size and amount need to be mapped.

float params->getVelocity(float start = 0, float end = 1); // Map this to how fast things are moving. higher is faster.
float params->getAmount(float start = 0, float end = 1); // If the patterns draw multiple things, use this.
float params->getSize(float start = 0, float end = 1); // How big the drawn object is, or how much is should change in size.
float params->getVariant(float start = 0, float end = 1); // Use this to create variantions of the pattern. eg. Sweep from bottom, left, right or top. Or move cw or ccw. or in or out, etc
float params->getOffset(float start = 0, float end = 1); // When rendering multiple objects, use this to create a phase offset between their lfo's. Do not use offset to offset all pixel intensities.
float params->getIntensity(float start = 0, float end = 1); 

Use the start and end argument to set the range. For velocity, a higher value means a shorter duration. for amount a higher value means less objects, and for size a higher value means smaller. 

```
class ParamsPattern : public Pattern<RGBA>
{
public:
    LFO<PWM> lfo;

    ParamsPattern()
    {
        this->name = "Params";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        // You can use params in you patterns to give the user freedom to further tweak some value.
        // I use a predetermined set of params (Velocity, amount, size, etc).
        // You can get their values from the params argument. By default they will be in the range 0-1,
        // but you can rescale it by providing a min and max argument.

        lfo.setPeriod(params->getVelocity(5000, 500));
        lfo.setDutyCycle(params->getSize(0.1, 0.9));
        int amount = params->getAmount(1, 4);

        for (int i = 0; i < width; i++)
        {
            float phase = float(i) / width;
            pixels[i] = params->getPrimaryColor() * lfo.getValue(amount * phase);
        }
    }
};
```

## Utils
You can use a few utility functions from the Utils class. These are located in `platform/includes/utils.hpp`.

int Utils::random(int minimum, int maximum);
float Utils::random_f(); (between 0-1)
int utils::constrain(int value, int min, int max);
int utils::constrain_f(float value, float min, float max);
float Utils::rescale(float value, float start_out, float end_out, float start_in, float end_in);
float Utils::rescale_c(float value, float start_out, float end_out, float start_in, float end_in); //Clamps the value

## More Examples

```
class StrobePattern : public Pattern<RGBA>
{
    int framecounter = 1;

public:
    StrobePattern()
    {
        this->name = "Strobe palette";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        framecounter--;

        RGBA color = RGBA(0, 0, 0, 255);
        if (framecounter <= 1)
            color = params->getPrimaryColor();

        if (framecounter == 0)
            framecounter = 5;

        for (int index = 0; index < width; index++)
            pixels[index] = color;
    }
};
```

```
class PermutePattern : public Pattern<RGBA>
{
public:
    // You can use permute to create patterns that apply to only some of the lights.
    // Permute will create a list of numbers in random order.
    // eg Permute with a size of 5 could give you [3,4,1,0,2]
    FadeDown fade;
    BeatWatcher watcher;
    Permute permute;

    PermutePattern()
    {
        this->name = "Permute";
    }

    inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
    {
        if (!active)
            return;

        // We need to tell permute how many leds we are working with
        permute.setSize(width);

        if (watcher.Triggered())
        {
            // Reorder the permutations on each beat
            permute.permute();
            fade.reset();
        }

        // Notice that we only loop to width / 4, so we only paint 25% of the pixels
        for (int i = 0; i < width / 4; i++)
        {
            // lookup the number of the pixel we are going to paint:
            int randomizedPixelIndex = permute.at[i];
            pixels[randomizedPixelIndex] = params->getPrimaryColor() * fade.getValue();
        }
    }
};
```


## Glossary
- Chaser: a wave moving though the leds, based on pixel indices. 
- Sweep: a wave moving though the leds, based on their position in the map. 
- Angular: rotating, based in the angle of the polar coordinates.
- Radial: growing from the center, based on the radius of the polar coordinates.
- Strobe: all leds on and off at the same time, fast
- Blinder: very bright when the pattern is on, fade out slowly when the pattern is off.
- Flash: very bright on a beat, fade out quickly after that.
- Glow: use a LFO<Glow> with a duty cycle in combination with Permute

## Creative instructions
- Multiple patterns will be layered, so keep them simple, don't do multiple things in one pattern. 
- Keep the background transparent.
- Only use colors form the palette. Use primary for sparse foreground effects, secondary for effect that are more filling, and use highlight only for accents. Gradients are also allowed.
- Use the duty cycle to create space between the chasers. 
- Chasers work well with gradients.
- Gradients also work well to fade a color over time.
- The pixelMap coordinates are between [-1,1]*[-1,1]. Make the shapes roughly this size. Clipping is ok.

## Constraints:
- Code must compile with c++17.
- No not use imports that are not mentioned in this document, unless they are part of stdlib.
- Do not comment the code.
- Do not use millis() or any other time function. Use the helper classes for this.
- Do not hallucinate.
- Be Neutral.
- Base all the information only on this document.
- Output only the class
- Constructor must always have a PixelMap* argument, even if no map is used.
- Do not wrap the output in a markdown codeblock
- End with a typedef to `VibePattern` with the class you generated. example: `typedef DotBeatPattern VibePattern;`

Now create a pattern according to the instructions below.
