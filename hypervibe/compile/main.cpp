#include <stdio.h>
#include <emscripten/emscripten.h>
#include "core/distribution/buffer.hpp"
#include "core/distribution/outputs/bufferOutput.hpp"
#include "core/distribution/inputs/patternInput.hpp"
#include "core/distribution/processors/colorConverter.hpp"
#include "core/generation/patterns/helpers/tempo/tempo.h"
#include "core/generation/patterns/helpers/tempo/abstractTempo.h"
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

EXTERN EMSCRIPTEN_KEEPALIVE void init(int size, uint8_t *outputArray) {
    // printf("init called\n");

    pattern = new VibePattern();

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