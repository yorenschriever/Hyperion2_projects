#pragma once
#include "common/patterns/pattern-helpers.hpp"
#include "core/generation/patterns/pattern.hpp"
#include "core/generation/patterns/helpers/triggerFade.h"
#include "core/generation/patterns/helpers/triggerTimeline.h"
#include "../patterns.hpp" //for heartZoom
#include <math.h>
#include <vector>

namespace TriggerPatterns
{

    class HeartZoom : public Pattern<RGBA>
    {
    public:
        PixelMap3d *map;
        TriggerFade<> fade;

        HeartZoom(PixelMap3d *map)
        {
            this->name = "Heart zoom";
            this->map = map;
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!fade.isActive(active))
                return;

            fade.setDuration(params->getSize(300, 700));            

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



    class DoubleFlash : public Pattern<RGBA>
    {
        PixelMap3d::Cylindrical *map;
        TriggerTimeline timeline;

    public:
        DoubleFlash(PixelMap3d::Cylindrical *map)
        {
            this->map = map;
            this->name = "Double flash";
        }

        inline void Calculate(RGBA *pixels, int width, bool active, Params *params) override
        {
            if (!timeline.isActive(active, 200))
                return;

            if (!timeline.Happened(0) && !timeline.Happened(100))
                return; //quit rendering if we are not in one of the flashes 

            for (int index = 0; index < std::min(width, (int)map->size()); index++)
            {
                if (map->r(index) > 0.5 )
                    pixels[index] = params->getHighlightColor();
            }
        }
    };


}