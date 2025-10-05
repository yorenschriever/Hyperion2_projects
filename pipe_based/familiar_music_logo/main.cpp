#include "hyperion.hpp"
#include "mapping/bulbMap.hpp"
#include "mapping/ledMap.hpp"
#include "patterns-chevrons.hpp"
#include "patterns-led.hpp"
#include "patterns-monochrome-mapped.hpp"
#include "patterns-monochrome.hpp"

LUT *incandescentLut8 = new IncandescentLUT(2.5, 255, 24);
LUT *trianglesLut = new ColourCorrectionLUT(1.8, 255, 200, 200, 200);

// void addLedPipe(Hyperion *hyp);
// void addBulbPipe(Hyperion *hyp);
void addLaserBarsPipe(Hyperion *hyp);
void addChevronsPipe(Hyperion *hyp);
void addPaletteColumn(Hyperion *hyp);

#define COL_PALETTE 0
#define COL_BULBS 1
#define COL_BULBS_MAPPED 2
#define COL_LEDS 3
#define COL_LASER_BARS 4
#define COL_CHEVRONS 5

PixelMap::Polar pBulbMap = bulbMap.toPolar();

int main()
{
    auto hyp = new Hyperion();

    addPaletteColumn(hyp);
    // addBulbPipe(hyp);
    // addLedPipe(hyp);
    addChevronsPipe(hyp);
    addLaserBarsPipe(hyp);

    hyp->hub.setColumnName(COL_PALETTE, "Palette");
    hyp->hub.setColumnName(COL_BULBS, "Bulbs");
    hyp->hub.setColumnName(COL_BULBS_MAPPED, "Bulbs mapped");
    hyp->hub.setColumnName(COL_LEDS, "Leds");
    hyp->hub.setColumnName(COL_LASER_BARS, "Lasers");
    hyp->hub.setColumnName(COL_CHEVRONS, "Chevrons");

    hyp->hub.setFlashColumn(COL_PALETTE, false, true);
    hyp->hub.setForcedSelection(COL_PALETTE);
    hyp->hub.buttonPressed(COL_PALETTE, 0);

    hyp->start();

    while (1)
        Thread::sleep(1000);
}

void addBulbPipe(Hyperion *hyp)
{
    auto input = new ControlHubInput<Monochrome>(
        bulbMap.size(),
        &hyp->hub,
        {
            {.column = COL_BULBS, .slot = 0, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = COL_BULBS, .slot = 1, .pattern = new MonochromePatterns::SinPattern()},
            {.column = COL_BULBS, .slot = 2, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = COL_BULBS, .slot = 3, .pattern = new MonochromePatterns::LFOPattern<SoftSawDown>("Soft Saw Down")},
            {.column = COL_BULBS, .slot = 4, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = COL_BULBS, .slot = 5, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = COL_BULBS, .slot = 6, .pattern = new MonochromePatterns::LFOPattern<Glow>("Neg Cos")},
            {.column = COL_BULBS, .slot = 7, .pattern = new MonochromePatterns::BeatSingleFadePattern()},

            {.column = COL_BULBS, .slot = 8, .pattern = new MonochromePatterns::LFOPattern<Sin>("Sin")},
            {.column = COL_BULBS, .slot = 9, .pattern = new MonochromePatterns::LFOPattern<PWM>("PWM")},
            {.column = COL_BULBS, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = COL_BULBS, .slot = 11, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = COL_BULBS, .slot = 12, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = COL_BULBS, .slot = 13, .pattern = new MonochromePatterns::OnPattern()},

            {.column = COL_BULBS, .slot = 14, .pattern = new MonochromePatterns::OnPattern()},
            {.column = COL_BULBS, .slot = 15, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = COL_BULBS, .slot = 16, .pattern = new MonochromePatterns::BeatAllFadePattern()},

            {.column = COL_BULBS_MAPPED, .slot = 0, .pattern = new MonochromeMappedPatterns::SinPattern(&bulbMap)},
            {.column = COL_BULBS_MAPPED, .slot = 1, .pattern = new MonochromeMappedPatterns::RadialFadePattern(&pBulbMap)},

        });

    auto pipe = new ConvertPipe<Monochrome, RGB>(
        input,
        new MonitorOutput(&hyp->webServer, &bulbMap, 60, .03));

    //   auto pipe = new ConvertPipe<Monochrome, RGB>(
    //       input,
    //       new UDPOutput("hyperslave5.local", 9618, 60));

    hyp->addPipe(pipe);
}

void addLedPipe(Hyperion *hyp)
{
    auto input = new ControlHubInput<RGBA>(
        ledMap.size(),
        &hyp->hub,
        {
            //   {.column = COL_LEDS, .slot = 0, .pattern = new LedPatterns::OnPattern(RGB(255,0,0), "red")},
            //   {.column = COL_LEDS, .slot = 1, .pattern = new LedPatterns::OnPattern(RGB(0,255,0), "green")},
            //   {.column = COL_LEDS, .slot = 2, .pattern = new LedPatterns::OnPattern(RGB(0,0,255), "blue")},

            {.column = COL_LEDS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "primary")},
            {.column = COL_LEDS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "secondary")},
            {.column = COL_LEDS, .slot = 2, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = COL_LEDS, .slot = 3, .pattern = new LedPatterns::SegmentChasePattern()},
            {.column = COL_LEDS, .slot = 4, .pattern = new LedPatterns::FlashesPattern()},
            {.column = COL_LEDS, .slot = 5, .pattern = new LedPatterns::StrobePattern()},
            {.column = COL_LEDS, .slot = 6, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = COL_LEDS, .slot = 7, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = COL_LEDS, .slot = 8, .pattern = new LedPatterns::StrobeHighlightPattern()},

        });

    auto pipe = new ConvertPipe<RGBA, RGB>(
        input,
        new MonitorOutput(&hyp->webServer, &ledMap));

    //   auto pipe = new ConvertPipe<RGBA, RGB>(
    //       input,
    //       new UDPOutput("hyperslave5.local", 9611, 60));

    hyp->addPipe(pipe);
}

void addLaserBarsPipe(Hyperion *hyp)
{
    const int numLasers = 60;
    auto input = new ControlHubInput<Monochrome>(
        numLasers,
        &hyp->hub,
        {
            {.column = COL_LASER_BARS, .slot = 0, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = COL_LASER_BARS, .slot = 1, .pattern = new MonochromePatterns::SinPattern()},
            {.column = COL_LASER_BARS, .slot = 2, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = COL_LASER_BARS, .slot = 3, .pattern = new MonochromePatterns::LFOPattern<SoftSawDown>("Soft Saw Down")},
            {.column = COL_LASER_BARS, .slot = 4, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = COL_LASER_BARS, .slot = 5, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = COL_LASER_BARS, .slot = 6, .pattern = new MonochromePatterns::LFOPattern<Glow>("Neg Cos")},
            {.column = COL_LASER_BARS, .slot = 7, .pattern = new MonochromePatterns::BeatSingleFadePattern()},

            {.column = COL_LASER_BARS, .slot = 8, .pattern = new MonochromePatterns::LFOPattern<Sin>("Sin")},
            {.column = COL_LASER_BARS, .slot = 9, .pattern = new MonochromePatterns::LFOPattern<PWM>("PWM")},
            {.column = COL_LASER_BARS, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = COL_LASER_BARS, .slot = 11, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = COL_LASER_BARS, .slot = 12, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = COL_LASER_BARS, .slot = 13, .pattern = new MonochromePatterns::OnPattern()},

            {.column = COL_LASER_BARS, .slot = 14, .pattern = new MonochromePatterns::OnPattern()},
            {.column = COL_LASER_BARS, .slot = 15, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = COL_LASER_BARS, .slot = 16, .pattern = new MonochromePatterns::BeatAllFadePattern()},

        });

    // auto pipe = new ConvertPipe<Monochrome, Monochrome>(
    //     input,
    //     new UDPOutput("hyperslave2.local", 9611, 60));

    // hyp->addPipe(pipe);

    int totalBytes = numLasers * sizeof(Monochrome);

    auto splitInput = new InputSlicer(
        input, {
        {0 * totalBytes / 2, totalBytes / 2, true}, 
        {1 * totalBytes / 2, totalBytes / 2, true}, 
        });

    for (int i = 0; i < 2; i++)
    {
        auto pipe = new Pipe(
            splitInput->getInput(i),
            new UDPOutput("hyperslave3.local", 9611 + i*4, 60)
        );
        hyp->addPipe(pipe);
    }
}

void addChevronsPipe(Hyperion *hyp)
{
    const int numLeds = 60 * 2 * 10;
    auto input = new ControlHubInput<RGBA>(
        numLeds,
        &hyp->hub,
        {

            {.column = COL_CHEVRONS, .slot = 9, .pattern = new Chevrons::RibbenClivePattern<Glow>(10000, 1, 0.15)},
            {.column = COL_CHEVRONS, .slot = 10, .pattern = new Chevrons::RibbenFlashPattern()},

            {.column = COL_CHEVRONS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "primary")},
            {.column = COL_CHEVRONS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "secondary")},
            {.column = COL_CHEVRONS, .slot = 2, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = COL_CHEVRONS, .slot = 3, .pattern = new LedPatterns::SegmentChasePattern()},
            {.column = COL_CHEVRONS, .slot = 4, .pattern = new LedPatterns::FlashesPattern()},
            {.column = COL_CHEVRONS, .slot = 5, .pattern = new LedPatterns::StrobePattern()},
            {.column = COL_CHEVRONS, .slot = 6, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = COL_CHEVRONS, .slot = 7, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = COL_CHEVRONS, .slot = 8, .pattern = new LedPatterns::StrobeHighlightPattern()},

        });

    int totalBytes = numLeds * sizeof(RGBA);

    auto splitInput = new InputSlicer(
        input, {
        {0 * totalBytes / 2, totalBytes / 2, true}, 
         {1 * totalBytes / 2, totalBytes / 2, true}, 

        });

    for (int i = 0; i < 2; i++)
    {
        auto pipe = new ConvertPipe<RGBA, GBR>(
            splitInput->getInput(i),
            new UDPOutput("hyperslave2.local", 9611 + i*4, 60),
            trianglesLut);
        hyp->addPipe(pipe);
    }

    // auto splitInput = new InputSlicer(
    //     input, {
    //     {0 * totalBytes / 4, totalBytes / 4, true}, 
    //      {1 * totalBytes / 4, totalBytes / 4, true}, 
    //      {2 * totalBytes / 4, totalBytes / 4, true}, 
    //      {3 * totalBytes / 4, totalBytes / 4, true}
    //     });

    // for (int i = 0; i < 4; i++)
    // {
    //     auto pipe = new ConvertPipe<RGBA, GBR>(
    //         splitInput->getInput(i),
    //         new UDPOutput("hyperslave5.local", 9611 + i, 60),
    //         trianglesLut);
    //     hyp->addPipe(pipe);
    // }

    // auto pipe = new ConvertPipe<RGBA, BGR>(
    //     input,
    //     new UDPOutput("hyperslave5.local", 9611, 60));
    // hyp->addPipe(pipe);
}

void addPaletteColumn(Hyperion *hyp)
{
    auto paletteColumn = new PaletteColumn(
        &hyp->hub,
        COL_PALETTE,
        0,
        {

            &pinkSunset,
            &sunset8,
            &campfire,
            &heatmap2,
            &sunset2,
            &sunset7,
            &tunnel,
            &redSalvation,
            &plumBath,

            &sunset6,

            &sunset1,
            &coralTeal,
            &deepBlueOcean,

            &heatmap,
            &sunset4,
            &candy,
            &sunset3,
            &greatBarrierReef,
            &blueOrange,
            &peach,
            &denseWater,
            &purpleGreen,
            &sunset5,
            &salmonOnIce,
            &sunset2,
            &retro,
        });
    hyp->hub.subscribe(paletteColumn);
}
