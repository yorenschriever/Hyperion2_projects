#pragma once
#include "distribution/utils/indexMap.hpp"
#include <vector>
#include <algorithm>
#include <numeric>

class BaseZigZagMapper : public IndexMap
{
    std::vector<int> indices;

    void reverseBetween(int start, int size)
    {
        std::reverse(indices.begin() + start, indices.begin() + start + size);
    }

public:
    BaseZigZagMapper(int numLeds, int segmentSize = 60)
    {
        indices.resize(numLeds);
        std::iota(indices.begin(), indices.end(), 0);

        for (int i = 0; i < numLeds; i += segmentSize)
        {
            int seg = i / segmentSize;
            if (seg % 2 == 1)
            {
                int size = std::min(segmentSize, numLeds - i);
                reverseBetween(i, size);
            }
        }
    }

    int map(int i) override
    {
        return indices[i];
    }
};

class WingsZigZagMapper : public IndexMap
{
    std::vector<int> indices;

    void reverseBetween(int start, int size)
    {
        std::reverse(indices.begin() + start, indices.begin() + start + size);
    }

public:
    WingsZigZagMapper(int numLeds, int ledsPerMeter = 60)
    {
        indices.resize(numLeds);
        std::iota(indices.begin(), indices.end(), 0);

        int L = ledsPerMeter;
        for (int w = 0; w < 4; w++)
        {
            int wstart = w * 8 * L;
            reverseBetween(wstart + 2 * L, L);
            reverseBetween(wstart + 3 * L, 2 * L);

            reverseBetween(wstart + 5 * L, 3 * L);
            reverseBetween(wstart + 6 * L, 2 * L);
        }
    }

    int map(int i) override
    {
        return indices[i];
    }
};
