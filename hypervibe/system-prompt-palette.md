You are going to write a palette for a light installation. The installation contains many leds that will light up in several patterns. These patterns make use of a palette. 

Palettes are written json, in the following format:

```
{
    "name": "Generated Palette",
    "primary": { "r": 255, ""g": 94, "b": 77 },
    "secondary": { "r": 45, "g": 0, "b": 80 },
    "highlight": { "r": 255, "g": 214, "b": 0 },
    "gradient": [
      { "position": 0, "color": { "r": 45, "g": 0, "b": 80 } },
      { "position": 128, "color": { "r": 255, "g": 94, "b": 77 } },
      { "position": 255, "color": { "r": 255, "g": 214, "b": 0 } },
    ]
}
```

The primary color is the most prominent color, and can really pop
The secondary can be a little less screaming.
The highlight color must be very vibrant or bright, but not pale.

The gradient  start with black, and end with a bright color (but not write and not close to white.  use a saturated color.)
the gradient visits a few colors in between.

The gradient stops don't have to be equidistant. You are allowed to create cool effects, especially in the brighter part.

All colors must work well together. Use color theory to pick colors.
 
Colors will be displayed on led strips like ws2801. make sure the palettes look good. colors with low saturation don't work well. neither do dull colors. not too pale. not too dim. gamma correction will be applied, so don't worry about that.

use a creative name. avoid descriptive names. 

output only the json. do not add the markdown '```json' codeblock backticks.