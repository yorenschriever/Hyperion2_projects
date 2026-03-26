#include "common/distributeAndMonitor.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-mapped-2d.hpp"
#include "common/patterns/patterns-trigger.hpp"
#include "common/setViewParams.hpp"
#include "patterns.hpp"
#include "hyperion.hpp"
#include "mapping/freak-map.hpp"

void addPaletteColumn(Hyperion *hyp);

LUT *pixelLut = new ColorCorrectionLUT(1.5, 255, 255, 255, 230);

PixelMap freakMapBig = resizeAndTranslateMap(freakMap, 1.2);
PixelMap nFreakMap = normalizeMap(freakMap);
PixelMap::Polar pFreakMap = freakMapBig.toPolarRotate90();

//timing for ws2801 500kHz
NeoPixels::Timing timing = {
    .bit0_high = 1000,
    .bit0_low = 1000,
    .bit1_high = 1000,
    .bit1_low = 1000,
    .reset = 2000000
};

enum Column
{
    PALETTE = 0,
    FULL = 1,
    LETTER = 2,
    SWEEP = 3,
    SPARSE = 4,
    MASK = 5,
    TRIGGER = 6,
    FLASH = 7,

    DEBUG = 8
};

int main()
{
    auto hyp = new Hyperion();

    int nleds = freakMapBig.size();

    std::vector<Slave> distribution =
    {
        {"hyperslave5.local", 9621, 100},
        {"hyperslave5.local", 9622, 150},
        // {"hypernode2.local", 9621, 50},
        // {"hypernode2.local", 9622, 50},
        // {"hypernode3.local", 9621, 50},
    };

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = FULL, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = FULL, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = FULL, .slot = 2, .pattern = new Mapped2dPatterns::HorizontalSaw(&pFreakMap)},
            {.column = FULL, .slot = 3, .pattern = new RadialGradient(&pFreakMap)},
            {.column = FULL, .slot = 4, .pattern = new LinearGradient(&freakMap)},

            {.column = LETTER, .slot = 0, .pattern = new LedPatterns::RibbenFlashPattern(50)},
            {.column = LETTER, .slot = 1, .pattern = new LedPatterns::RibbenClivePattern<Glow>(1000, 1, 0.25, 50)},
            {.column = LETTER, .slot = 2, .pattern = new OnLetterChaseUpPattern(&freakMap)},
            {.column = LETTER, .slot = 3, .pattern = new OnLetterChaseCenterPattern(&freakMap)},

            {.column = SWEEP, .slot = 0, .pattern = new Mapped2dPatterns::Lighthouse(&pFreakMap)},
            {.column = SWEEP, .slot = 1, .pattern = new Mapped2dPatterns::HorizontalSin(&pFreakMap)},
            {.column = SWEEP, .slot = 2, .pattern = new Mapped2dPatterns::RadialFadePattern(&pFreakMap)},
            {.column = SWEEP, .slot = 3, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(&pFreakMap)},
            {.column = SWEEP, .slot = 4, .pattern = new Mapped2dPatterns::RadialSaw(&pFreakMap)},

            {.column = SPARSE, .slot = 0, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = SPARSE, .slot = 1, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(&freakMap)},
            {.column = SPARSE, .slot = 2, .pattern = new Mapped2dPatterns::LineLaunch(&freakMap)},
            {.column = SPARSE, .slot = 3, .pattern = new Mapped2dPatterns::DotBeatPattern(&pFreakMap)},
            {.column = SPARSE, .slot = 4, .pattern = new SinChase()},
            {.column = SPARSE, .slot = 5, .pattern = new SawChasePattern()},

            {.column = TRIGGER, .slot = 0, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = TRIGGER, .slot = 1, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = TRIGGER, .slot = 2, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = TRIGGER, .slot = 3, .pattern = new TriggerPatterns::GrowingCirclePattern(&pFreakMap)},
            {.column = TRIGGER, .slot = 4, .pattern = new TriggerPatterns::LineLaunch(&nFreakMap)},

            {.column = FLASH, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = FLASH, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = FLASH, .slot = 2, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = FLASH, .slot = 3, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = FLASH, .slot = 4, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = FLASH, .slot = 5, .pattern = new LedPatterns::FadeFromRandom()},

            {.column = DEBUG, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution)},
            {.column = DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(10, 1)},
            {.column = DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
        });

    distributeAndMonitor<RGB, RGBA>(hyp,input,&freakMap,distribution,pixelLut,0.01,timing);

    addPaletteColumn(hyp);

    hyp->hub.setColumnName(PALETTE, "Kleur");
    hyp->hub.setColumnName(FULL, "Full");
    hyp->hub.setColumnName(LETTER, "Letter");
    hyp->hub.setColumnName(SWEEP, "Sweep");
    hyp->hub.setColumnName(SPARSE, "Sparse");
    hyp->hub.setColumnName(MASK, "Mask");
    hyp->hub.setColumnName(TRIGGER, "Trigger");
    hyp->hub.setColumnName(FLASH, "Flash");
    hyp->hub.setColumnName(DEBUG, "Debug");

    hyp->hub.buttonPressed(PALETTE, 0);
    hyp->hub.setForcedSelection(PALETTE);
    hyp->hub.setFlashColumn(PALETTE, false, true);
    hyp->hub.setFlashColumn(TRIGGER, true);
    hyp->hub.setFlashColumn(FLASH, true);

    // hyp->hub.buttonPressed(1, 0);
    // hyp->hub.buttonPressed(2, 0);

    hyp->start();

    Tempo::AddSource(new ConstantTempo(120));

    auto viewParams = new ViewParams(
        35,
        -0.75,
        Vector{0, 0.1, -2.5},
        Rotation{M_PI, 0, 1, 0},
        Rotation{0, 0, 1, 0});
    setViewParams(hyp, viewParams);

    while (1)
        Thread::sleep(1000);
}

void addPaletteColumn(Hyperion *hyp)
{
    auto paletteColumn = new PaletteColumn(
        &hyp->hub,
        PALETTE,
        0,
        {
            &pinkSunset,
            &heatmap,
            &campfire,
            &retro,
            &sunset4,
            &tunnel,
            &blueOrange,
            &purpleGreen,

            &sunset2,
            &sunset8,
            &heatmap2,
            &sunset3,
            &sunset6,
            &sunset7,
            &coralTeal,
            &deepBlueOcean,
            &redSalvation,
            &plumBath,
            &sunset1,
            &candy,
            &greatBarrierReef,
            &peach,
            &denseWater,
            &sunset5,
            &salmonOnIce,
        });
    hyp->hub.subscribe(paletteColumn);
}
