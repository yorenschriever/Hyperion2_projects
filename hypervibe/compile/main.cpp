#include <stdio.h>
#include <emscripten/emscripten.h>
#include "core/distribution/buffer.hpp"
#include "core/distribution/outputs/bufferOutput.hpp"
#include "core/distribution/inputs/patternInput.hpp"
#include "core/distribution/processors/colorConverter.hpp"
#include "core/generation/patterns/helpers/tempo/tempo.h"
#include "core/generation/patterns/helpers/tempo/abstractTempo.h"
#include "core/generation/pixelMap/pixelMap.hpp"
#include "core/distribution/chain.hpp"
#include "platform/includes/utils.hpp"

#include "pattern.hpp"

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

class TempoSource : public AbstractTempo
{
public:
    void beat(){
        validSignal = true;
        AbstractTempo::beat();
    }
};

Pattern<RGBA> *pattern;
ColorConverter<RGBA,RGB> colorConverter;
Chain *chain;
TempoSource tempo;
PixelMap pixelMap;

EXTERN EMSCRIPTEN_KEEPALIVE void init(int size, uint8_t *outputArray, float *pixelMapData) {
    // printf("init called\n");

    for (int i = 0; i < size; i++)
        pixelMap.push_back({.x = pixelMapData[i * 2], .y = pixelMapData[i * 2 + 1]});

    pattern = new VibePattern(&pixelMap);

    chain = new Chain(
        new PatternInput<RGBA>(size, pattern),
        &colorConverter, 
        new BufferOutput(outputArray, size * sizeof(RGB))
    );

    Tempo::AddSource(&tempo);
}

EXTERN EMSCRIPTEN_KEEPALIVE void beat() {
    // printf("beat called\n");
    tempo.beat();
}

EXTERN EMSCRIPTEN_KEEPALIVE void process() {
    // printf("calculateFrame Called\n");
    chain->process();
}