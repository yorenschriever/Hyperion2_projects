#include "core/hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-test.hpp"

void addKeyholeChain();
void addLampshadeChain();
void addLedbarsChain();
void addLetterBoardChain();
void addFogChain();
void addPaletteColumn();

LUT *LaserLut = new LaserLUT(0.5, 4096, 3048);
LUT *IncandescentLut8 = new IncandescentLUT(2.5, 255, 20);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);
LUT *GammaLut8 = new GammaLUT(2.5, 255);
LUT *ledLut = new ColorCorrectionLUT(2.7, 255, 255, 255, 255);

enum Columns
{
    PALETTE,
    KEYHOLE,
    LAMPSHADE,
    LEDBARS,
    LETTERBOARD,
    FOG,
    FOG_LED,
    DEBUG,
};

Hyperion hyp;
Combine dmxCombine;

int main()
{
    hyp.createChain(&dmxCombine,new UDPOutput("hypernode1.local",9619));

    addKeyholeChain();
    addLampshadeChain();
    addLedbarsChain();
    addLetterBoardChain();
    addFogChain();
    addPaletteColumn();

    hyp.hub.setColumnName(Columns::PALETTE, "Kleur");
    hyp.hub.setColumnName(Columns::KEYHOLE, "Keyhole");
    hyp.hub.setColumnName(Columns::LAMPSHADE, "Lampshade");
    hyp.hub.setColumnName(Columns::LEDBARS, "Ledbars");
    hyp.hub.setColumnName(Columns::LETTERBOARD, "Letterboard");
    hyp.hub.setColumnName(Columns::FOG, "Fog"); 
    hyp.hub.setColumnName(Columns::FOG_LED, "Fog LED");

    Tempo::AddSource(new ConstantTempo(120));

    // for (int i = 1; i < 10; i++)
    //     hyp->hub.setFlashColumn(i, false, true);
    // hyp->hub.setFlashRow(5);
    // hyp->hub.setFlashRow(6);
    // hyp->hub.setFlashRow(7);

    hyp.hub.buttonPressed(Columns::PALETTE, 0);
    hyp.hub.setForcedSelection(Columns::PALETTE);
    hyp.hub.setFlashColumn(Columns::PALETTE, false, true);

    hyp.start();

    while (1)
        Thread::sleep(1000);
}

void DMXAndMonitor(ControlHubInput<Monochrome> *input, int size, int startChannel, PixelMap *pixelMap, float monitorDotSize = 0.01)
{
    auto clone = new Slicer(
        {
            {0, size, true},
            {0, size, false},
        });
    
    hyp.createChain(input, clone);

    hyp.createChain(
        clone->getSlice(0),
        //TODO configurable lut
        new ColorConverter<Monochrome, Monochrome>(IncandescentLut8),
        dmxCombine.atDmxChannel(startChannel)
    );

    hyp.createChain(
        clone->getSlice(1),
        new ColorConverter<Monochrome, RGB>(),
        new MonitorOutput(&hyp.webServer, pixelMap, nullptr,  60, monitorDotSize)
    );
}

PixelMap createKeyholeMap()
{
    float dist = 0.07;
    Turtle turtle;
    turtle.setPosition(-2.5*dist, 0);
    turtle.setDirection(-90);

    turtle.turn(15);
    turtle.addPositionToTrail();
    for (int i = 0; i < 6; i++)
        turtle.move(dist);

    turtle.turn(-62);
    for (int i = 0; i < 13; i++)
    {
        turtle.move(dist);
        turtle.turn(360/16);
    }

    turtle.turn(-62);
    for (int i = 0; i < 6; i++)
        turtle.move(dist);

    turtle.turn(90);
    for (int i = 0; i < 4; i++)
        turtle.move(dist);

    // return applyIndexMap(turtle, new ReverseMapper(turtle.size()));
    return turtle;
}

void addKeyholeChain()
{
    int dmxStartChannel = 1;
    int size = 30;
    auto input = new ControlHubInput<Monochrome>(
        size,
        &hyp.hub,
        {
            {.column = Columns::KEYHOLE, .slot = 0, .pattern = new MonochromePatterns::OnPattern()},
            {.column = Columns::KEYHOLE, .slot = 1, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = Columns::KEYHOLE, .slot = 2, .pattern = new MonochromePatterns::SinPattern()},
            {.column = Columns::KEYHOLE, .slot = 3, .pattern = new MonochromePatterns::BeatSingleFadePattern()},
            {.column = Columns::KEYHOLE, .slot = 4, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = Columns::KEYHOLE, .slot = 5, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = Columns::KEYHOLE, .slot = 6, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = Columns::KEYHOLE, .slot = 7, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = Columns::KEYHOLE, .slot = 8, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = Columns::KEYHOLE, .slot = 9, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::KEYHOLE, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = Columns::KEYHOLE, .slot = 11, .pattern = new MonochromePatterns::SingleGlitchPattern()},
        });

    // auto map = new PixelMap(circleMap(size, 0.5, 0.5, 0));
    auto map = new PixelMap(createKeyholeMap());

    DMXAndMonitor(input, size, dmxStartChannel, map, 0.02); 
}

void addLampshadeChain()
{
    int dmxStartChannel = 50;
    int size = 8;
    auto input = new ControlHubInput<Monochrome>(
        size,
        &hyp.hub,
        {
            {.column = Columns::LAMPSHADE, .slot = 0, .pattern = new MonochromePatterns::OnPattern()},
            {.column = Columns::LAMPSHADE, .slot = 1, .pattern = new MonochromePatterns::GlowPattern()},
            {.column = Columns::LAMPSHADE, .slot = 2, .pattern = new MonochromePatterns::SinPattern()},
            {.column = Columns::LAMPSHADE, .slot = 3, .pattern = new MonochromePatterns::BeatSingleFadePattern()},
            {.column = Columns::LAMPSHADE, .slot = 4, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = Columns::LAMPSHADE, .slot = 5, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = Columns::LAMPSHADE, .slot = 6, .pattern = new MonochromePatterns::BlinderPattern()},
            {.column = Columns::LAMPSHADE, .slot = 7, .pattern = new MonochromePatterns::BeatAllFadePattern()},
            {.column = Columns::LAMPSHADE, .slot = 8, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = Columns::LAMPSHADE, .slot = 9, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::LAMPSHADE, .slot = 10, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = Columns::LAMPSHADE, .slot = 11, .pattern = new MonochromePatterns::SingleGlitchPattern()},
        });

    auto map = new PixelMap(circleMap(size, 0.25, 0, 0.7));

    //TODO hier geen lut toepassen, want dat doen de dimmerpacks
    DMXAndMonitor(input, size, dmxStartChannel, map, 0.02); 
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
            {.column = Columns::LEDBARS, .slot = 2, .pattern = new LedPatterns::GlowPattern()},
            {.column = Columns::LEDBARS, .slot = 3, .pattern = new LedPatterns::GlowPulsePattern()},
            {.column = Columns::LEDBARS, .slot = 4, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 5, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::LEDBARS, .slot = 6, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::LEDBARS, .slot = 7, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::LEDBARS, .slot = 8, .pattern = new LedPatterns::FadingNoisePattern()},
            {.column = Columns::LEDBARS, .slot = 9, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::LEDBARS, .slot = 10, .pattern = new LedPatterns::SinPattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 11, .pattern = new LedPatterns::GradientChasePattern(), .indexMap=zigzag},
            {.column = Columns::LEDBARS, .slot = 12, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::LEDBARS, .slot = 13, .pattern = new LedPatterns::FadeFromRandom(), .indexMap=zigzag},

            {.column = Columns::LEDBARS, .slot = 14, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    auto map = new PixelMap(
        applyIndexMap(
            combineMaps({
                resizeAndTranslateMap(rotateMap(gridMap(60, 6, 0.012, 0.15), 90), 1,  -1, -0.7, -0.4),
                resizeAndTranslateMap(rotateMap(gridMap(60, 6, 0.012, 0.15), 90), -1, -1,  0.7, -0.4)
            }), 
            zigzag
        )
    );

    distributeAndMonitor<BGR>(&hyp,input,map,distribution,ledLut, 0.02); 
}

template <class T_DMX_COLOR>
void DMXAndMonitorRGB(ControlHubInput<RGBA> *input, int size, int startChannel, PixelMap *pixelMap, float monitorDotSize = 0.01)
{
    auto clone = new Slicer(
        {
            {0, int(sizeof(RGBA))*size, true},
            {0, int(sizeof(RGBA))*size, false},
        });
    
    hyp.createChain(input, clone);

    hyp.createChain(
        clone->getSlice(0),
        //TODO configurable lut
        new ColorConverter<RGBA, T_DMX_COLOR>(),
        dmxCombine.atDmxChannel(startChannel)
    );

    hyp.createChain(
        clone->getSlice(1),
        new ColorConverter<RGBA, RGB>(),
        new MonitorOutput(&hyp.webServer, pixelMap, nullptr,  60, monitorDotSize)
    );
}

void addLetterBoardChain()
{
    int dmxStartChannel = 100;
    int size = 1;
    auto input = new ControlHubInput<RGBA>(
        size,
        &hyp.hub,
        {
            {.column = Columns::LETTERBOARD, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::LETTERBOARD, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
        });

    auto map = new PixelMap({
        {0, 0.7}
    });

    DMXAndMonitorRGB<RGBW>(input, size, dmxStartChannel, map, 0.1); 
}

void addFogChain()
{
    int dmxStartChannel = 150;

    auto map = new PixelMap({
        {0, 0.2}
    });

    //////////

    auto input = new ControlHubInput<RGBA>(
        1,
        &hyp.hub,
        {
            {.column = Columns::FOG_LED, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::FOG_LED, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
        });

    DMXAndMonitorRGB<RGBAmber>(input, 1, dmxStartChannel + 2, map, 0.1); 

    auto dimmerInput = new PatternInput<Monochrome>(
        1,
        new MonochromePatterns::StaticPattern("Dimmer", {{.channel = 0}})
    );

    hyp.createChain(dimmerInput, dmxCombine.atDmxChannel(dmxStartChannel+1));

    ///////////

    auto inputFog = new ControlHubInput<Monochrome>(
        1,
        &hyp.hub,
        {
            {.column = Columns::FOG, .slot = 0, .pattern = new MonochromePatterns::OnPattern(255, "Fog"), .noMasterDim=true},
        
            {.column = Columns::FOG, .slot = 1, .pattern = new MonochromePatterns::IntervalPattern(4, 64, "Interval 4/64 beats (minst)"), .noMasterDim=true},
            {.column = Columns::FOG, .slot = 4, .pattern = new MonochromePatterns::IntervalPattern(8, 64, "Interval 8/64 beats"), .noMasterDim=true},
            {.column = Columns::FOG, .slot = 2, .pattern = new MonochromePatterns::IntervalPattern(4, 32, "Interval 4/32 beats"), .noMasterDim=true},
            {.column = Columns::FOG, .slot = 3, .pattern = new MonochromePatterns::IntervalPattern(4, 16, "Interval 4/16 beats"), .noMasterDim=true},
            {.column = Columns::FOG, .slot = 5, .pattern = new MonochromePatterns::IntervalPattern(8, 16, "Interval 8/16 beats (meest)"), .noMasterDim=true},  
        }
    );

    DMXAndMonitor(inputFog, 1, dmxStartChannel, map, 0.05); 
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
