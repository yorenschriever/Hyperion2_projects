#include "core/hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-led.hpp"

void addKeyholeChain();
void addLampshadeChain();
void addLedbarsChain();
void addLetterBoardChain();
void addFogChain();
void addPaletteColumn();

LUT *LaserLut = new LaserLUT(0.5, 4096, 3048);
LUT *IncandescentLut = new IncandescentLUT(2.5, 4096, 200);
LUT *IncandescentLut8 = new IncandescentLUT(2.5, 255, 24);
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



    // hyp->hub.setColumnName(COL_PALETTE, "Kleur");
    // hyp->hub.setColumnName(COL_MOVINGHEAD, "Moving head");
    // hyp->hub.setColumnName(COL_COLANDER, "Vergiet");
    // hyp->hub.setColumnName(COL_FAIRYLIGHT_PINSPOT, "Fairylight / spiegelbol");
    // hyp->hub.setColumnName(COL_LASERS, "Lasers");
    // hyp->hub.setColumnName(COL_BULBS, "Peertjes");
    // hyp->hub.setColumnName(COL_LEDSTER, "Ledster");
    // hyp->hub.setColumnName(COL_STROBES, "Strobes");
    // hyp->hub.setColumnName(COL_LED1, "LED 1");
    // hyp->hub.setColumnName(COL_LED2, "LED 2");

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
    turtle.setPosition(-2*dist, 0);
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
        });

    auto map = new PixelMap(circleMap(size, 0.25, 0, 0.7));

    //TODO hier geen lut toepassen, want dat doen de dimmerpacks
    DMXAndMonitor(input, size, dmxStartChannel, map, 0.02); 
}

void addLedbarsChain()
{
    //TODO
    // IndexMap *zigzag = new WingsZigZagMapper(nleds);
    // IndexMap *zigzagReverse = new WingsZigZagMapper(nleds, 60, true);

    int nLeds = 60*8;
    Distribution distribution = {
        {"hypernode1.local",9611,4*60},
        {"hypernode1.local",9615,4*60},
    };

    
    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::LEDBARS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::LEDBARS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
        });

    auto map = new PixelMap(
        combineMaps({
            gridMap(4, 60, 0.2, 0.01, -0.7, -0.5),
            gridMap(4, 60, 0.2, 0.01, 0.7,  -0.5)
        })
    );

    distributeAndMonitor<BGR, RGBA>(&hyp,input,map,distribution,ledLut); 
}

void DMXAndMonitorRGB(ControlHubInput<RGBA> *input, int size, int startChannel, PixelMap *pixelMap, float monitorDotSize = 0.01)
{
    auto clone = new Slicer(
        {
            {0, sizeof(RGBA), true},
            {0, sizeof(RGBA), false},
        });
    
    hyp.createChain(input, clone);

    hyp.createChain(
        clone->getSlice(0),
        //TODO configurable lut
        new ColorConverter<RGBA, RGBW>(),
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

    DMXAndMonitorRGB(input, size, dmxStartChannel, map, 0.1); 
}

void addFogChain()
{
    int dmxStartChannel = 150;
    int size = 1;
    auto input = new ControlHubInput<RGBA>(
        size,
        &hyp.hub,
        {
            {.column = Columns::FOG, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::FOG, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
        });

    auto map = new PixelMap({
        {-0.5, 0.0}
    });

    DMXAndMonitorRGB(input, size, dmxStartChannel, map, 0.1); 
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
