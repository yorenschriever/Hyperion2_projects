#include "core/hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/dmxAndMonitor.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-mask.hpp"

void addLedparChain();
void addLedbarsChain();
void addSparksChain();
void addSunstripChain();
void addPaletteColumn();

LUT *LaserLut = new LaserLUT(0.5, 4096, 3048);
LUT *IncandescentLut8 = new IncandescentLUT(2.5, 255, 20);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);
LUT *GammaLut8 = new GammaLUT(2.5, 255);
LUT *ledLut = new ColorCorrectionLUT(2.7, 255, 255, 255, 255);

enum Columns
{
    PALETTE,
    LEDBARS,
    SPARKS,
    LEDPAR,
    SUNSTRIP,
};

int ledparStartChannel = 125;
int sunstripStartChannel = 200;

Hyperion hyp;
Combine dmxCombine;

int main()
{
    hyp.createChain(&dmxCombine,new UDPOutput("hypernode1.local",9619));

    addLedbarsChain();
    addPaletteColumn();
    addLedparChain();
    addSparksChain();
    addSunstripChain();

    hyp.hub.setColumnName(Columns::PALETTE, "Palette");
    hyp.hub.setColumnName(Columns::LEDPAR, "Ledpar");
    hyp.hub.setColumnName(Columns::LEDBARS, "Ledbars");
    hyp.hub.setColumnName(Columns::SPARKS, "Sparks");
    hyp.hub.setColumnName(Columns::SUNSTRIP, "Sunstrip");

    // for (int i = 1; i < 10; i++)
    //     hyp->hub.setFlashColumn(i, false, true);
    // hyp->hub.setFlashRow(5);
    // hyp->hub.setFlashRow(6);
    // hyp->hub.setFlashRow(7);

    hyp.hub.buttonPressed(Columns::PALETTE, 0);
    hyp.hub.setForcedSelection(Columns::PALETTE);
    hyp.hub.setFlashColumn(Columns::PALETTE, false, true);

    hyp.start();

    Tempo::AddSource(new ConstantTempo(120));

    while (1)
        Thread::sleep(1000);
}

void addLedbarsChain()
{
    int nLeds = 12*60;
    IndexMap *zigzag = new ZigZagMapper(60);
    Distribution distribution = {
        {"hypernode1.local",9611,6*60},
        {"hypernode1.local",9615,6*60},
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

            {.column = Columns::LEDBARS, .slot = 8, .pattern = new LedPatterns::GlowPattern()},
            {.column = Columns::LEDBARS, .slot = 9, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = Columns::LEDBARS, .slot = 10, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 11, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::LEDBARS, .slot = 12, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::LEDBARS, .slot = 13, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::LEDBARS, .slot = 14, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = Columns::LEDBARS, .slot = 15, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::LEDBARS, .slot = 16, .pattern = new LedPatterns::SinPattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 17, .pattern = new LedPatterns::GradientChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 18, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::LEDBARS, .slot = 19, .pattern = new LedPatterns::FadeFromRandom(), .indexMap=zigzag},

            {.column = Columns::LEDBARS, .slot = 20, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    auto map = new PixelMap(
        applyIndexMap(
            combineMaps({
                resizeAndTranslateMap(rotateMap(gridMap(60, 6, 0.012, 0.15), 00), 1,  -1, -0.45, -0.4),
                resizeAndTranslateMap(rotateMap(gridMap(60, 6, 0.012, 0.15), 00), -1, -1,  0.45, -0.4)
            }), 
            zigzag
        )
    );

    distributeAndMonitor<BGR>(&hyp,input,map,distribution,ledLut, 0.02); 
}


void addSparksChain()
{
    int size = 24;

    Distribution distribution = {
        {"hypernode1.local",9621,12},
        {"hypernode1.local",9621,12},
    };

    auto input = new ControlHubInput<Monochrome>(
        size,
        &hyp.hub,
        {
            {.column = Columns::SPARKS, .slot = 0, .pattern = new MonochromePatterns::OnPattern()},
            {.column = Columns::SPARKS, .slot = 1, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = Columns::SPARKS, .slot = 2, .pattern = new MonochromePatterns::SinPattern()},
            {.column = Columns::SPARKS, .slot = 3, .pattern = new MonochromePatterns::BeatSingleFadePattern()},
            {.column = Columns::SPARKS, .slot = 4, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = Columns::SPARKS, .slot = 5, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = Columns::SPARKS, .slot = 6, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = Columns::SPARKS, .slot = 7, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = Columns::SPARKS, .slot = 8, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = Columns::SPARKS, .slot = 9, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::SPARKS, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = Columns::SPARKS, .slot = 11, .pattern = new MonochromePatterns::SingleGlitchPattern()},
        });

    auto map = new PixelMap(gridMap(6, 4, 0.3, 0.15, 0, -0.47));

    distributeAndMonitor<Monochrome12>(&hyp,input,map,distribution,GammaLut12, 0.02);
}

void addLedparChain()
{
    int size = 4;
    auto input = new ControlHubInput<RGBA>(
        size,
        &hyp.hub,
        {
            {.column = Columns::LEDPAR, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::LEDPAR, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::LEDPAR, .slot = 2, .pattern = new LedPatterns::SinChasePattern()},
            {.column = Columns::LEDPAR, .slot = 3, .pattern = new LedPatterns::DuoTonePattern()},
            {.column = Columns::LEDPAR, .slot = 4, .pattern = new MaskPatterns::SinChaseMaskPattern()},
            {.column = Columns::LEDPAR, .slot = 5, .pattern = new MaskPatterns::GlowPulseMaskPattern()},
        });

    auto map = new PixelMap(gridMap(size, 1, 0.5, 0.5, 0, 0.7));

    DMXAndMonitor<RGBWAmberUV,RGBA>(&hyp, input, size, &dmxCombine, ledparStartChannel, map, 0.04);
}

void addSunstripChain()
{
    int size = 20;
    auto input = new ControlHubInput<Monochrome>(
        size,
        &hyp.hub,
        {
            {.column = Columns::SUNSTRIP, .slot = 0, .pattern = new MonochromePatterns::OnPattern()},
            {.column = Columns::SUNSTRIP, .slot = 1, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = Columns::SUNSTRIP, .slot = 2, .pattern = new MonochromePatterns::SinPattern()},
            {.column = Columns::SUNSTRIP, .slot = 3, .pattern = new MonochromePatterns::BeatSingleFadePattern()},
            {.column = Columns::SUNSTRIP, .slot = 4, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = Columns::SUNSTRIP, .slot = 5, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = Columns::SUNSTRIP, .slot = 6, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = Columns::SUNSTRIP, .slot = 7, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = Columns::SUNSTRIP, .slot = 8, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = Columns::SUNSTRIP, .slot = 9, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::SUNSTRIP, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = Columns::SUNSTRIP, .slot = 11, .pattern = new MonochromePatterns::SingleGlitchPattern()},
        });

    auto map = new PixelMap(gridMap(10, 2, 0.1, 0.1, 0, 0.4));

    DMXAndMonitor<Monochrome12>(&hyp, input, size, &dmxCombine, sunstripStartChannel, map, 0.02, GammaLut12);
}

// void addFogChain()
// {
//     auto map = new PixelMap({
//         {-0.5, 0.5}
//     });

//     //////////

//     auto input = new ControlHubInput<RGBA>(
//         1,
//         &hyp.hub,
//         {
//             {.column = Columns::FOG_LED, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
//             {.column = Columns::FOG_LED, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
//         });

//     DMXAndMonitorRGB<RGBAmber>(input, 1, fogStartChannel + 2, map, 0.1); 

//     auto dimmerInput = new PatternInput<Monochrome>(
//         1,
//         new MonochromePatterns::StaticPattern("Dimmer", {{.channel = 0}})
//     );

//     hyp.createChain(dimmerInput, dmxCombine.atDmxChannel(fogStartChannel+1));

//     ///////////

//     auto inputFog = new ControlHubInput<Monochrome>(
//         1,
//         &hyp.hub,
//         {
//             {.column = Columns::FOG, .slot = 0, .pattern = new MonochromePatterns::OnPattern(255, "Fog"), .noMasterDim=true},
        
//             {.column = Columns::FOG, .slot = 1, .pattern = new MonochromePatterns::IntervalPattern(4, 64, "Interval 4/64 beats (minst)"), .noMasterDim=true},
//             {.column = Columns::FOG, .slot = 4, .pattern = new MonochromePatterns::IntervalPattern(8, 64, "Interval 8/64 beats"), .noMasterDim=true},
//             {.column = Columns::FOG, .slot = 2, .pattern = new MonochromePatterns::IntervalPattern(4, 32, "Interval 4/32 beats"), .noMasterDim=true},
//             {.column = Columns::FOG, .slot = 3, .pattern = new MonochromePatterns::IntervalPattern(4, 16, "Interval 4/16 beats"), .noMasterDim=true},
//             {.column = Columns::FOG, .slot = 5, .pattern = new MonochromePatterns::IntervalPattern(8, 16, "Interval 8/16 beats (meest)"), .noMasterDim=true},  
//         }
//     );

//     DMXAndMonitor(inputFog, 1, fogStartChannel, map, 0.05); 
// }

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
