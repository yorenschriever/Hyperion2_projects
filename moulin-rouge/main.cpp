#include "hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-mapped-2d.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-mask.hpp"
#include "common/patterns/patterns-trigger.hpp"
#include "mapping/moulin-rouge-map.hpp"
#include "common/setViewParams.hpp"
// #include "distribution/utils/indexMap.hpp"
#include "patterns/static.hpp"
#include "patterns/base.hpp"
#include "patterns/wings.hpp"
#include "patterns/mask.hpp"
#include "patterns/trigger.hpp"
#include "patterns/vibe.hpp"
#include "patterns.hpp"
#include "wings-zigzag.hpp"

enum Columns
{
    PALETTE,
    STATIC,
    BASE,
    WINGS,
    TRIGGER,
    MASK,
    TRIGGER2,
    FLASH,
    DEBUG,
};

void addBase(Hyperion *hyp);
void addWings(Hyperion *hyp);

void addPaletteColumn(Hyperion *hyp);

LUT *pixelLut = new ColorCorrectionLUT(1.5, 255, 255, 255, 240);


PixelMap3d::Cylindrical cBaseMap = *(baseMap.toCylindricalXZ());
PixelMap base2d = resizeAndTranslateMap(*(baseMap.to2d()), 1.45, 0, 0.06);
PixelMap::Polar basePolar = *(base2d.toPolar());

PixelMap3d::Cylindrical cWingsMap = *(wingsMap.toCylindricalXY(0, -0.06));
PixelMap wings2d = resizeAndTranslateMap(*(wingsMap.to2d()), 1.45, 0, 0.06);
PixelMap::Polar wingsPolar = *(wings2d.toPolar());

int main()
{
    auto hyp = new Hyperion();

    addBase(hyp);
    addWings(hyp);
    addPaletteColumn(hyp);

    hyp->hub.setColumnName(Columns::PALETTE, "Kleur");
    hyp->hub.setColumnName(Columns::STATIC, "Static");
    hyp->hub.setColumnName(Columns::BASE, "Base");
    hyp->hub.setColumnName(Columns::WINGS, "Wings");
    hyp->hub.setColumnName(Columns::TRIGGER, "Trigger");
    hyp->hub.setColumnName(Columns::MASK, "Mask");
    hyp->hub.setColumnName(Columns::TRIGGER2, "Trigger 2");
    hyp->hub.setColumnName(Columns::FLASH, "Flash");
    hyp->hub.setColumnName(Columns::DEBUG, "Debug");

    hyp->hub.buttonPressed(Columns::PALETTE, 0);
    hyp->hub.setForcedSelection(Columns::PALETTE);
    hyp->hub.setFlashColumn(Columns::PALETTE, false, true);
    hyp->hub.setFlashColumn(Columns::TRIGGER, true);
    hyp->hub.setFlashColumn(Columns::TRIGGER2, true);
    hyp->hub.setFlashColumn(Columns::FLASH, true);

    // hyp->hub.buttonPressed(Columns::DEBUG, 0);
    // hyp->hub.buttonPressed(Columns::WINGS, 0);

    hyp->start();
    
    Tempo::AddSource(new ConstantTempo(120));

    auto viewParams = new ViewParams(
        35,
        -0.75,
        Vector{0, 0.15, -2.5},
        Rotation{M_PI, 0, 1, 0},
        Rotation{0, 0, 1, 0});
    setViewParams(hyp, viewParams);

    while (1)
        Thread::sleep(1000);
}


void addBase(Hyperion *hyp)
{
    int nleds = baseMap.size();
    IndexMap *zigzag = new BaseZigZagMapper(nleds,3*60);
    IndexMap *zigzagReverse = new BaseZigZagMapper(nleds,3*60, true);

    Distribution distribution = {
        {"hypernode1.local",9611,6*60},
        {"hypernode1.local",9612,6*60},
        {"hypernode1.local",9613,6*60},
        {"hypernode1.local",9614,6*60},

        {"hypernode1.local",9615,6*60},

        {"hypernode2.local",9611,6*60},
        {"hypernode2.local",9612,6*60},
        {"hypernode2.local",9613,6*60},
        {"hypernode2.local",9614,6*60},
    };

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = Columns::STATIC, .slot = 0, .pattern = new BaseFadePattern(&baseMap, RGB(255,0,0),"Base")},
            {.column = Columns::STATIC, .slot = 1, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::STATIC, .slot = 2, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::STATIC, .slot = 3, .pattern = new BasePatterns::PaletteGradient(&base2d)},
            {.column = Columns::STATIC, .slot = 4, .pattern = new LedPatterns::RibbenFlashPattern(20)},
            {.column = Columns::STATIC, .slot = 5, .pattern = new LedPatterns::FadeFromRandom(3*60), .indexMap = zigzag},


            {.column = Columns::BASE, .slot = 0, .pattern = new Static::RadialSaw(&basePolar)},
            {.column = Columns::BASE, .slot = 1, .pattern = new Static::AngularSweep(&basePolar)},
            {.column = Columns::BASE, .slot = 2, .pattern = new Heart(&baseMap)},
            {.column = Columns::BASE, .slot = 3, .pattern = new HeartZoom(&baseMap)},
            {.column = Columns::BASE, .slot = 4, .pattern = new Static::HorizontalWave(&base2d)},
            {.column = Columns::BASE, .slot = 5, .pattern = new BasePatterns::SegmentChasePattern(3*60), .indexMap = zigzag},
            {.column = Columns::BASE, .slot = 6, .pattern = new BasePatterns::AngularWave(&cBaseMap)},
            {.column = Columns::BASE, .slot = 7, .pattern = new BasePatterns::FlyingEmbersPattern(3*60), .indexMap = zigzag},
            {.column = Columns::BASE, .slot = 8, .pattern = new Mapped2dPatterns::Lighthouse(&basePolar)},
            {.column = Columns::BASE, .slot = 9, .pattern = new BasePatterns::BarberPole(&cBaseMap)},
            {.column = Columns::BASE, .slot = 10, .pattern = new BasePatterns::RotatingRingsPattern(&cBaseMap)},

            // {.column = Columns::BASE, .slot = 11, .pattern = new VibePatterns::TriangleBurst(&base2d)},
            {.column = Columns::BASE, .slot = 12, .pattern = new VibePatterns::RotatingTriangle(&base2d)},
            {.column = Columns::BASE, .slot = 13, .pattern = new VibePatterns::TriangleStutter(&base2d)},
            {.column = Columns::BASE, .slot = 14, .pattern = new VibePatterns::TrianglePulse(&base2d)},
            {.column = Columns::BASE, .slot = 15, .pattern = new VibePatterns::TriangleOutlineGrow(&base2d)},

            {.column = Columns::TRIGGER, .slot = 0, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::TRIGGER, .slot = 1, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::TRIGGER, .slot = 2, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::TRIGGER, .slot = 3, .pattern = new TriggerPatterns::GrowingCirclePattern(&basePolar)},
            {.column = Columns::TRIGGER, .slot = 4, .pattern = new TriggerPatterns::LineLaunch(&base2d)},
            {.column = Columns::TRIGGER, .slot = 5, .pattern = new TriggerPatterns::LineLaunch(&base2d, 500, 20)},
            {.column = Columns::TRIGGER, .slot = 6, .pattern = new TriggerPatterns::LineLaunch(&base2d, 1000, 20)},
            {.column = Columns::TRIGGER, .slot = 7, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base up"), .indexMap = zigzag},
            {.column = Columns::TRIGGER, .slot = 8, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base down"), .indexMap = zigzagReverse},
            {.column = Columns::TRIGGER, .slot = 9, .pattern = new TriggerPatterns::HeartZoom(&baseMap)},
            {.column = Columns::TRIGGER, .slot = 10, .pattern = new TriggerPatterns::FadeFromRandom(3*60), .indexMap = zigzag },



            {.column = Columns::MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 1, .pattern = new MaskPatterns::GlowPulseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, true), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, false), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 6, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::FLASH, .slot = 7, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},

            {.column = Columns::FLASH, .slot = 0, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},
        

            // {.column = Columns::MASK, .slot = 3, .pattern = new MaskPatterns::TriggerPalettePattern(1, "Secondary", 20)},

            // {.column = Columns::BASE, .slot = 1, .pattern = new OneColor(RGB(0,255,0),"Green")},
            // {.column = Columns::BASE, .slot = 2, .pattern = new OneColor(RGB(0,0,255),"Blue")},



            
            
            

            

            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution, 60)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(10, 1)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 9, .pattern = new TestPatterns::OrderBarsPattern(distribution, 60, 4, "Order bars zigzag"),  .indexMap = zigzag},
        });

    distributeAndMonitor<BGR, RGBA>(hyp,input,&baseMap,distribution,pixelLut);
}

void addWings(Hyperion *hyp)
{
    int nleds = wingsMap.size();
    IndexMap *zigzag = new WingsZigZagMapper(nleds);
    IndexMap *zigzagReverse = new WingsZigZagMapper(nleds, 60, true);

    Distribution distribution = {
        {"hypernode3.local",9611,5*60},
        {"hypernode3.local",9612,3*60},

        {"hypernode3.local",9613,5*60},
        {"hypernode3.local",9614,3*60},

        {"hypernode3.local",9615,5*60},
        {"hypernode3.local",9616,3*60},

        {"hypernode3.local",9617,5*60},
        {"hypernode3.local",9618,3*60},
    };

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            // {.column = Columns::WINGS, .slot = 1, .pattern = new OneColor(RGB(0,255,0),"Green")},
            // {.column = Columns::WINGS, .slot = 2, .pattern = new OneColor(RGB(0,0,255),"Blue")},

            // {.column = Columns::WINGS, .slot = 4, .pattern = new HeartZoom(&wingsMap)},

            // {.column = Columns::WINGS, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            // {.column = Columns::WINGS, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            // {.column = Columns::WINGS, .slot = 2, .pattern = new Static::HorizontalSaw(&wingsPolar)},
            // {.column = Columns::WINGS, .slot = 3, .pattern = new Static::HorizontalSin(&wingsPolar)},
            // {.column = Columns::WINGS, .slot = 4, .pattern = new Static::RadialSaw(&wingsPolar)},

            // {.column = Columns::WINGS, .slot = 5, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap = zigzag},

            {.column = Columns::WINGS, .slot = 0, .pattern = new WingsFadePattern(&cWingsMap, RGB(255,0,0),"Red")},
            {.column = Columns::WINGS, .slot = 1, .pattern = new Mapped2dPatterns::Lighthouse(&wingsPolar)},
            {.column = Columns::WINGS, .slot = 2, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(&wings2d)},
            {.column = Columns::WINGS, .slot = 3, .pattern = new Mapped2dPatterns::DotBeatPattern(&wingsPolar)},
            {.column = Columns::WINGS, .slot = 4, .pattern = new Mapped2dPatterns::RadialFadePattern(&wingsPolar)},
            {.column = Columns::WINGS, .slot = 5, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(&wingsPolar)},
            {.column = Columns::WINGS, .slot = 6, .pattern = new Wings::GrowShrink(&wingsPolar)},
            {.column = Columns::WINGS, .slot = 7, .pattern = new Wings::WingCycleFlashesPattern()},
            {.column = Columns::WINGS, .slot = 8, .pattern = new Wings::SegmentGlitchPattern(8*60)},
            {.column = Columns::WINGS, .slot = 9, .pattern = new LedPatterns::SegmentGlitchPattern()},

            {.column = Columns::TRIGGER, .slot = 11, .pattern = new TriggerPatterns::DoubleFlash(&cWingsMap)},
            {.column = Columns::TRIGGER, .slot = 12, .pattern = new TriggerPatterns::GlitterFade(&cWingsMap)},
            {.column = Columns::TRIGGER, .slot = 13, .pattern = new TriggerPatterns::Meteor(16, 50, 250, "Meteor wings out"), .indexMap = zigzag},
            {.column = Columns::TRIGGER, .slot = 14, .pattern = new TriggerPatterns::Meteor(16, 50, 250, "Meteor wings in"), .indexMap = zigzagReverse},
           
            // {.column = Columns::BASE, .slot = 11, .pattern = new VibePatterns::TriangleBurst(&wings2d)},
            {.column = Columns::BASE, .slot = 12, .pattern = new VibePatterns::RotatingTriangle(&wings2d)},
            {.column = Columns::BASE, .slot = 13, .pattern = new VibePatterns::TriangleStutter(&wings2d)},
            {.column = Columns::BASE, .slot = 14, .pattern = new VibePatterns::TrianglePulse(&wings2d)},
            {.column = Columns::BASE, .slot = 15, .pattern = new VibePatterns::TriangleOutlineGrow(&wings2d)},


            {.column = Columns::MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 1, .pattern = new MaskPatterns::GlowPulseMaskPattern(), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(8*60, true)},
            // {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(2*60, false)},
            {.column = Columns::MASK, .slot = 4, .pattern = new MaskPatterns::WingsFadePattern(&wingsPolar)},
            {.column = Columns::MASK, .slot = 5, .pattern = new MaskPatterns::WingsFadePattern(&wingsPolar,true)},

            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution, 60)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(10, 1)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 9, .pattern = new TestPatterns::OrderBarsPattern(distribution, 60, 4, "Order bars zigzag"),  .indexMap = zigzag},
        });

    distributeAndMonitor<BGR, RGBA>(hyp,input,&wingsMap,distribution,pixelLut);
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
