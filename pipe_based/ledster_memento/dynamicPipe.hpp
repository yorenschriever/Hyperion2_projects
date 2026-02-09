#pragma once

#include "core/distribution/pipes/pipe.hpp"

//This pipe is still work in progress
template <class T_SOURCE_COLOR1, class T_SOURCE_COLOR2, class T_TARGET_COLOR>
class DynamicPipe : public Pipe
{
protected:
    virtual int transfer(uint8_t *data, int length, Output *out)
    {
        if (length === sizeMatch * sizeof(T_SOURCE_COLOR1))
            return transfer_<T_SOURCE_COLOR1>(data,length,out);
        else
            return transfer_<T_SOURCE_COLOR2>(data,length,out);
    }

    template <class T_SOURCE_COLOR_>
    virtual int transfer_(uint8_t *data, int length, Output *out)
    {
        out->setLength(length / sizeof(T_SOURCE_COLOR_) * sizeof(T_TARGET_COLOR));

        int numPixels = length / sizeof(T_SOURCE_COLOR_);
        for (int i = 0; i < numPixels; i++)
        {
            // store the i-th pixel in an object of type T (source datatype)
            auto col = ((T_SOURCE_COLOR_ *)data)[i];

            T_TARGET_COLOR outCol = static_cast<T_TARGET_COLOR>(col);

            if (lut)
                outCol.ApplyLut(lut);

            out->setData((uint8_t *)&outCol, sizeof(T_TARGET_COLOR), i * sizeof(T_TARGET_COLOR));
        }
        return numPixels;
    }

public:
    DynamicPipe(Input *in, Output *out, LUT *lut = NULL, int sizeMatch) : Pipe(in, out)
    {
        this->sizeMatch = sizeMatch;
        this->lut = lut;
    }
    LUT *lut;
    int sizeMatch;

};
