#include "common/distributeAndMonitor.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/setViewParams.hpp"
#include "hyperion.hpp"
#include "mapping/freak-map.hpp"

#define COL_PALETTE 0
#define COL_LED 1

void addPaletteColumn(Hyperion *hyp);

LUT *pixelLut = new ColorCorrectionLUT(1.5, 255, 255, 255, 240);

PixelMap::Polar pFreakMap = freakMap.toPolarRotate90();

int main()
{
    auto hyp = new Hyperion();

    int nleds = freakMap.size();

    std::vector<Slave> distribution =
    {
        {"hypernode1.local", 9621, 50},
        {"hypernode1.local", 9622, 50},
        {"hypernode2.local", 9621, 50},
        {"hypernode2.local", 9622, 50},
        {"hypernode3.local", 9621, 50},
    };

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = 1, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution)},
            {.column = 1, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = 1, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = 1, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = 1, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = 1, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = 1, .slot = 6, .pattern = new TestPatterns::Palette(10, 1)},
            {.column = 1, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
        });

    distributeAndMonitor<BGR, RGBA>(hyp,input,&freakMap,distribution,pixelLut);

    addPaletteColumn(hyp);

    hyp->hub.setColumnName(COL_PALETTE, "Kleur");
    hyp->hub.setColumnName(1, "Base");
    hyp->hub.setColumnName(2, "Wings");

    Tempo::AddSource(new ConstantTempo(120));

    hyp->hub.buttonPressed(COL_PALETTE, 0);
    hyp->hub.setForcedSelection(COL_PALETTE);
    hyp->hub.setFlashColumn(COL_PALETTE, false, true);

    hyp->hub.buttonPressed(1, 0);
    hyp->hub.buttonPressed(2, 0);

    hyp->start();

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
    hyp->hub.subscribe(paletteColumn);
}
