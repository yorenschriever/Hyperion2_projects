#include <stdio.h>
#include <emscripten/emscripten.h>
#include "core/distribution/buffer.hpp"
#include "core/distribution/outputs/bufferOutput.hpp"
#include "core/distribution/inputs/patternInput.hpp"
#include "core/distribution/processors/colorConverter.hpp"
#include "core/generation/patterns/helpers/tempo/tempo.h"
#include "core/generation/patterns/helpers/tempo/abstractTempo.h"
#include "core/generation/patterns/helpers/timeline.h"
#include "core/generation/pixelMap/pixelMap.hpp"
#include "core/generation/pixelMap/spatialGrid.hpp"
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
PatternInput<RGBA> *patternInput;
TempoSource tempo;
PixelMap pixelMap;


EXTERN EMSCRIPTEN_KEEPALIVE void init(int size, uint8_t *outputArray, float *pixelMapData) {
    // printf("init called\n");

    for (int i = 0; i < size; i++)
        pixelMap.push_back({.x = pixelMapData[i * 2], .y = pixelMapData[i * 2 + 1]});

#ifdef NEEDS_MAP
    auto pixelMapPtr = pixelMap.getPtr();
    pattern = new VibePattern(pixelMapPtr);
#elifdef NEEDS_POLAR_MAP
    auto pixelMapPolarPtr = pixelMap.toPolar();
    pattern = new VibePattern(pixelMapPolarPtr);
#else
    pattern = new VibePattern();
#endif



    patternInput = new PatternInput<RGBA>(size, pattern);

    chain = new Chain(
        patternInput,
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

EXTERN EMSCRIPTEN_KEEPALIVE void setParam(int index, float value) {
    if (!patternInput) return;
    switch (index) {
        case 0: patternInput->params.velocity  = value; break;
        case 1: patternInput->params.amount    = value; break;
        case 2: patternInput->params.size      = value; break;
        case 3: patternInput->params.variant   = value; break;
        case 4: patternInput->params.offset    = value; break;
        case 5: patternInput->params.intensity = value; break;
    }
}

Palette customPalette{
    .gradient = Gradient({
        {.position = 0, .color = RGB(0, 0, 0)},        // Black
        {.position = 128, .color = RGB(255, 0, 0)},    // Red
        {.position = 224, .color = RGB(255, 255, 0)},  // Bright yellow
        {.position = 255, .color = RGB(255, 255, 255)} // Full white
    }),
    .primary = RGB(255, 0, 0),
    .secondary = RGB(255, 255, 0),
    .highlight = RGB(255, 255, 255),
    .name = "Heatmap"};
Gradient::GradientEntry gradientBuffer[32];

EXTERN EMSCRIPTEN_KEEPALIVE void setPrimary(int r, int g, int b) {
    customPalette.primary = RGB(r, g, b);
}

EXTERN EMSCRIPTEN_KEEPALIVE void setSecondary(int r, int g, int b) {
    customPalette.secondary = RGB(r, g, b);
}

EXTERN EMSCRIPTEN_KEEPALIVE void setHighlight(int r, int g, int b) {
    customPalette.highlight = RGB(r, g, b);
}

EXTERN EMSCRIPTEN_KEEPALIVE void setGradientStop(int index, int position, int r, int g, int b) {
    if (index < 0 || index >= 32) return;
    gradientBuffer[index] = {.position = (uint8_t)position, .color = RGB(r, g, b)};
}

EXTERN EMSCRIPTEN_KEEPALIVE void applyPalette(int numStops) {
    if (!patternInput) return;
    std::vector<Gradient::GradientEntry> entries(gradientBuffer, gradientBuffer + numStops);
    customPalette.gradient = Gradient(entries);
    patternInput->params.palette = &customPalette;
}