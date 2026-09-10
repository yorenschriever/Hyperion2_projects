#pragma once
#include "colors.h"

class RGBWZoom : Color
{
public:
    RGBWZoom()
    {
        this->R = 0;
        this->G = 0;
        this->B = 0;
        this->W = 0;
        this->Zoom = 0;
    }

    RGBWZoom(uint8_t R, uint8_t G, uint8_t B, uint8_t W, u_int8_t Zoom)
    {
        this->R = R;
        this->G = G;
        this->B = B;
        this->W = W;
        this->Zoom = Zoom;
    }

    RGBWZoom(RGBA rgba, u_int8_t Zoom = 0)
    {
        RGBW rgbw = rgba;
        this->R = rgbw.R;
        this->G = rgbw.G;   
        this->B = rgbw.B;
        this->W = 0;
        // this->W = rgbw.W;
    }

    inline void ApplyLut(LUT *lut)
    {
        R = lut->luts[0 % lut->Dimension][R];
        G = lut->luts[1 % lut->Dimension][G];
        B = lut->luts[2 % lut->Dimension][B];
        W = lut->luts[3 % lut->Dimension][W];
    }

    inline void dim(uint8_t value)
    {
        R = (R * value) >> 8;
        G = (G * value) >> 8;
        B = (B * value) >> 8;
        W = (W * value) >> 8;
    }

    uint8_t R, G, B, W, Zoom;
};