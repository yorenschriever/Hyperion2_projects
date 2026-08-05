#include "core/hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/dmxAndMonitor.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-mask.hpp"
#include "mapping/domeMap.hpp"
#include "mapping/stageMap.hpp"

void addDomeChain();
void addStageChain();
void addPaletteColumn();

LUT *ledLut = new ColorCorrectionLUT(2.7, 255, 255, 255, 255);

enum Columns
{
    PALETTE,
    LEDPAR,
    SUNSTRIP,
    LEDBARS,
    LEDBARS2,
    LEDBARS_TRIGGER,
    SPARKS,
};

Hyperion hyp;

int main()
{
    addPaletteColumn();
    addDomeChain();
    addStageChain();


    hyp.hub.setColumnName(Columns::PALETTE, "Palette");
    hyp.hub.setColumnName(Columns::LEDPAR, "Ledpar");
    hyp.hub.setColumnName(Columns::SUNSTRIP, "Sunstrip");
    hyp.hub.setColumnName(Columns::LEDBARS, "Ledbars BG");
    hyp.hub.setColumnName(Columns::LEDBARS2, "Ledbars FG");
    hyp.hub.setColumnName(Columns::LEDBARS_TRIGGER, "Ledbars Tr");
    hyp.hub.setColumnName(Columns::SPARKS, "Sparks");
    
    hyp.hub.setFlashColumn(Columns::LEDBARS_TRIGGER);
    hyp.hub.setFlashColumn(Columns::SPARKS);


    hyp.hub.buttonPressed(Columns::PALETTE, 0);
    hyp.hub.setForcedSelection(Columns::PALETTE);
    hyp.hub.setFlashColumn(Columns::PALETTE, false, true);

    hyp.hub.buttonPressed(Columns::LEDBARS, 8);

    hyp.start();

    Tempo::AddSource(new ConstantTempo(120));

    while (1)
        Thread::sleep(1000);
}

void addDomeChain()
{

    auto map = new PixelMap3d(createDomeMap());

    int nLeds = map->size();
    IndexMap *zigzag = new ZigZagMapper(60, true);

    Distribution distribution = {
        {"hypernode3.local",9611,nLeds},
    };

    
    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::LEDBARS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::LEDBARS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::LEDBARS, .slot = 2, .pattern = new LedPatterns::DuoTonePattern(6*60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 3, .pattern = new LedPatterns::DuoTonePattern(30), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 4, .pattern = new LedPatterns::DuoTonePattern(60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 5, .pattern = new LedPatterns::GradientPattern(6*60,60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 6, .pattern = new LedPatterns::GradientPattern(6*60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 7, .pattern = new LedPatterns::GradientPattern(60), .indexMap=zigzag},

            {.column = Columns::LEDBARS2, .slot = 0, .pattern = new LedPatterns::GlowPattern()},
            {.column = Columns::LEDBARS2, .slot = 1, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = Columns::LEDBARS2, .slot = 2, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 3, .pattern = new LedPatterns::SinPattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 4, .pattern = new LedPatterns::GradientChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 5, .pattern = new LedPatterns::FadeFromRandom(), .indexMap=zigzag},

            {.column = Columns::LEDBARS_TRIGGER, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 2, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 3, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 4, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 5, .pattern = new LedPatterns::SegmentGlitchPattern()},
            
            {.column = Columns::LEDBARS, .slot = 8, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp,input,map,distribution,ledLut, 0.01); 
}

void addStageChain()
{

    auto map = new PixelMap(createStageMap());

    int nLeds = map->size();
    IndexMap *zigzag = new ZigZagMapper(60, true);

    Distribution distribution = {
        //dak
        {"hypernode1.local",9611,3*60},
        {"hypernode1.local",9612,3*60},
        {"hypernode1.local",9613,6*60},
        {"hypernode1.local",9614,6*60},

        //backdrop
        {"hypernode2.local",9611,3*60},
        {"hypernode2.local",9612,3*60},
        {"hypernode2.local",9615,6*60},
        {"hypernode2.local",9616,6*60},

        //voorkant
        {"hypernode2.local",9613,3*60},
        {"hypernode2.local",9614,3*60},
    };

    
    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::LEDBARS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::LEDBARS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::LEDBARS, .slot = 2, .pattern = new LedPatterns::DuoTonePattern(6*60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 3, .pattern = new LedPatterns::DuoTonePattern(30), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 4, .pattern = new LedPatterns::DuoTonePattern(60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 5, .pattern = new LedPatterns::GradientPattern(6*60,60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 6, .pattern = new LedPatterns::GradientPattern(6*60), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 7, .pattern = new LedPatterns::GradientPattern(60), .indexMap=zigzag},

            {.column = Columns::LEDBARS2, .slot = 0, .pattern = new LedPatterns::GlowPattern()},
            {.column = Columns::LEDBARS2, .slot = 1, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = Columns::LEDBARS2, .slot = 2, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 3, .pattern = new LedPatterns::SinPattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 4, .pattern = new LedPatterns::GradientChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS2, .slot = 5, .pattern = new LedPatterns::FadeFromRandom(), .indexMap=zigzag},

            {.column = Columns::LEDBARS_TRIGGER, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 2, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 3, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 4, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::LEDBARS_TRIGGER, .slot = 5, .pattern = new LedPatterns::SegmentGlitchPattern()},
            
            {.column = Columns::LEDBARS, .slot = 8, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp,input,map,distribution,ledLut, 0.01); 
}

void addPaletteColumn()
{
    auto paletteColumn = new PaletteColumn(
        &hyp.hub,
        0,
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
    hyp.hub.subscribe(paletteColumn);
}
