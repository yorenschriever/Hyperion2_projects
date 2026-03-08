#include "hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "mapping/moulin-rouge-map.hpp"
#include "common/setViewParams.hpp"
#include "patterns.hpp"

#define COL_PALETTE 0
#define COL_LED 1

void addBase(Hyperion *hyp);
void addWings(Hyperion *hyp);

void addPaletteColumn(Hyperion *hyp);

LUT *pixelLut = new ColorCorrectionLUT(1.5, 255, 255, 255, 240);

PixelMap3d::Cylindrical cWingsMap = wingsMap.toCylindricalXY();

int main()
{
    auto hyp = new Hyperion();

    addBase(hyp);
    addWings(hyp);
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


void addBase(Hyperion *hyp)
{
    int nleds = baseMap.size();

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = 1, .slot = 0, .pattern = new Base(&baseMap, RGB(255,0,0),"Red")},
            {.column = 1, .slot = 1, .pattern = new OneColor(RGB(0,255,0),"Green")},
            {.column = 1, .slot = 2, .pattern = new OneColor(RGB(0,0,255),"Blue")},
        });

    distributeAndMonitor<BGR, RGBA>(
        hyp,
        input,
        &baseMap,
        {
            {"hypernode1.local",9611,nleds/4},
            {"hypernode1.local",9612,nleds/4},
            {"hypernode1.local",9613,nleds/4},
            {"hypernode1.local",9614,nleds/4},
        },
        pixelLut
    );
}

void addWings(Hyperion *hyp)
{
    int nleds = wingsMap.size();

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = 2, .slot = 0, .pattern = new Wings(&cWingsMap, RGB(255,0,0),"Red")},
            {.column = 2, .slot = 1, .pattern = new OneColor(RGB(0,255,0),"Green")},
            {.column = 2, .slot = 2, .pattern = new OneColor(RGB(0,0,255),"Blue")},
        });

    distributeAndMonitor<BGR, RGBA>(
        hyp,
        input,
        &wingsMap,
        {
            {"hypernode2.local",9611,nleds/4},
            {"hypernode2.local",9612,nleds/4},
            {"hypernode2.local",9613,nleds/4},
            {"hypernode2.local",9614,nleds/4},
        },
        pixelLut
    );
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
