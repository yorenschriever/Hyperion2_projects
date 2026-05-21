#include "hyperion.hpp"
#include "common/distributeAndMonitor.hpp"
#include "common/distributeAndPreview.hpp"
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
#include "palettes.hpp"
#include "wings-zigzag.hpp"

#define BAR_V1 BGR
#define BAR_V2 GBR

enum Columns
{
    PALETTE,
    BG,
    SWEEPS,
    ACCENTS,
    ENERGY,
    TRIGGER,
    MASK,
    FLASH,
    DEBUG,
};

void addBase(Hyperion *hyp);
void addWings(Hyperion *hyp);

void addPaletteColumn(Hyperion *hyp);

LUT *pixelLut = new ColorCorrectionLUT(1.5, 255, 255, 255, 240);


PixelMap3d::Cylindrical cBaseMap = *(baseMap.toCylindricalXZ());
PixelMap base2d = resizeAndTranslateMap(*(baseMap.to2d()), 1.45, 0, 0.06*1.45);
PixelMap::Polar basePolar = *(base2d.toPolar());

PixelMap3d::Cylindrical cWingsMap = *(resizeAndTranslateMap3d(wingsMap, 1, 0, 0.06).toCylindricalXY()); 
PixelMap wings2d = resizeAndTranslateMap(*(wingsMap.to2d()), 1.45, 0, 0.06*1.45);
PixelMap::Polar wingsPolar = *(wings2d.toPolar());

PixelMap all2d = resizeAndTranslateMap(combineMaps({base2d, wings2d}), -1);

int main()
{
    auto hyp = new Hyperion();

    // hyp->createChain(
    //     new PatternInput<RGBA>(all2d.size(), new LedPatterns::PalettePattern(0, "Primary")),
    //     new MonitorOutput(&hyp->webServer,&all2d)
    // );

    addBase(hyp);
    addWings(hyp);
    addPaletteColumn(hyp);

    hyp->hub.setColumnName(Columns::PALETTE, "Kleur");
    hyp->hub.setColumnName(Columns::BG, "Background");
    hyp->hub.setColumnName(Columns::SWEEPS, "Sweeps");
    hyp->hub.setColumnName(Columns::ACCENTS, "Accents");
    hyp->hub.setColumnName(Columns::ENERGY, "Energy");
    hyp->hub.setColumnName(Columns::TRIGGER, "Trigger");
    hyp->hub.setColumnName(Columns::MASK, "Mask");
    hyp->hub.setColumnName(Columns::FLASH, "Flash");
    hyp->hub.setColumnName(Columns::DEBUG, "Debug");

    hyp->hub.buttonPressed(Columns::PALETTE, 0);
    hyp->hub.setForcedSelection(Columns::PALETTE);
    hyp->hub.setFlashColumn(Columns::PALETTE, false, true);
    hyp->hub.setFlashColumn(Columns::TRIGGER, true);
    hyp->hub.setFlashColumn(Columns::FLASH, true);

    hyp->hub.buttonPressed(Columns::BG, 2);
    hyp->hub.buttonPressed(Columns::BG, 4);

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

        // {"hypernode1.local",9615,6*60},

        {"hypernode2.local",9611,6*60},
        {"hypernode2.local",9612,6*60},
        {"hypernode2.local",9613,3*60},
        // {"hypernode2.local",9614,6*60},
    };

    SlotPatternsGenerator baseSlotPatternsGenerator = [&](){
        return std::vector<ControlHubInput<RGBA>::SlotPattern>({
            {.column = Columns::BG, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::BG, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::BG, .slot = 2, .pattern = new BaseFadePattern(&baseMap, "Primary gradient")},
            {.column = Columns::BG, .slot = 3, .pattern = new BasePatterns::PaletteGradient(&base2d)},
            {.column = Columns::BG, .slot = 4, .pattern = new BasePatterns::BaseBeamsPattern(), .indexMap = zigzag},
            {.column = Columns::BG, .slot = 5, .pattern = new LedPatterns::RibbenFlashPattern(20)},
            {.column = Columns::BG, .slot = 6, .pattern = new LedPatterns::FadeFromRandom(3*60), .indexMap = zigzag},
            {.column = Columns::BG, .slot = 7, .pattern = new VibePatterns::InfernoParticles(&base2d)},
            {.column = Columns::BG, .slot = 8, .pattern = new Static::RadialSaw(&basePolar)},


            // {.column = Columns::SWEEPS, .slot = 0, .pattern = new Wings::GrowShrink(&basePolar)}, //WINGS ONLY?
            {.column = Columns::SWEEPS, .slot = 1, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(&base2d)}, //WINGS ONLY?
            {.column = Columns::SWEEPS, .slot = 2, .pattern = new Mapped2dPatterns::Lighthouse(&basePolar)},
            {.column = Columns::SWEEPS, .slot = 3, .pattern = new Static::AngularSweep(&basePolar)},
            {.column = Columns::SWEEPS, .slot = 4, .pattern = new BasePatterns::MeshPattern(&cBaseMap)},
            {.column = Columns::SWEEPS, .slot = 5, .pattern = new BasePatterns::BarberPole(&cBaseMap)},
            // {.column = Columns::SWEEPS, .slot = 6, .pattern = new Mapped2dPatterns::RadialFadePattern(&basePolar)}, //WINGS ONLY?
            // {.column = Columns::SWEEPS, .slot = 7, .pattern = new Mapped2dPatterns::DotBeatPattern(&basePolar)}, //WINGS ONLY?
            {.column = Columns::SWEEPS, .slot = 8, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(&basePolar)},
            {.column = Columns::SWEEPS, .slot = 9, .pattern = new VibePatterns::FireworkBurstPattern(&base2d)},


            {.column = Columns::ACCENTS, .slot = 0, .pattern = new Static::HorizontalWave(&base2d)},
            {.column = Columns::ACCENTS, .slot = 1, .pattern = new BasePatterns::AngularWave(&cBaseMap)},
            {.column = Columns::ACCENTS, .slot = 2, .pattern = new BasePatterns::RotatingRingsPattern(&cBaseMap)},
            {.column = Columns::ACCENTS, .slot = 3, .pattern = new BasePatterns::SegmentChasePattern(3*60), .indexMap = zigzag},
            {.column = Columns::ACCENTS, .slot = 4, .pattern = new VibePatterns::MeteorShowerPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 5, .pattern = new VibePatterns::BeerBubblesPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 6, .pattern = new VibePatterns::TrianglePulse(&base2d)},
            {.column = Columns::ACCENTS, .slot = 7, .pattern = new VibePatterns::TriangleStutter(&base2d)},
            {.column = Columns::ACCENTS, .slot = 8, .pattern = new VibePatterns::TriangleOutlineGrow(&base2d)},
            {.column = Columns::ACCENTS, .slot = 9, .pattern = new VibePatterns::RotatingTriangle(&base2d)},
            {.column = Columns::ACCENTS, .slot = 10, .pattern = new VibePatterns::FloatingOrbsPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 11, .pattern = new VibePatterns::MatrixRainPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 12, .pattern = new VibePatterns::BurningTornadoPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 13, .pattern = new VibePatterns::HeartbeatPattern(&base2d)},
            {.column = Columns::ACCENTS, .slot = 14, .pattern = new Heart(&baseMap)},
            {.column = Columns::ACCENTS, .slot = 15, .pattern = new VibePatterns::HeartStackPattern(&base2d)},
            

            {.column = Columns::ENERGY, .slot = 0, .pattern = new BasePatterns::FlyingEmbersPattern(3*60), .indexMap = zigzag},
            //wings cycle flashes
            {.column = Columns::ENERGY, .slot = 2, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::ENERGY, .slot = 3, .pattern = new HeartZoom(&baseMap)},


            {.column = Columns::TRIGGER, .slot = 0, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::TRIGGER, .slot = 1, .pattern = new TriggerPatterns::FadeFromRandom(3*60), .indexMap = zigzag },
            {.column = Columns::TRIGGER, .slot = 2, .pattern = new TriggerPatterns::GrowingCirclePattern(&basePolar)},  
            {.column = Columns::TRIGGER, .slot = 3, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base up"), .indexMap = zigzag},
            //wings meteor wings out
            {.column = Columns::TRIGGER, .slot = 5, .pattern = new TriggerPatterns::LineLaunch(&base2d, 500, 20)},
            {.column = Columns::TRIGGER, .slot = 6, .pattern = new TriggerPatterns::FadingNoisePattern()},
            //wings double flash
            {.column = Columns::TRIGGER, .slot = 8, .pattern = new TriggerPatterns::LineLaunch(&base2d)},
            {.column = Columns::TRIGGER, .slot = 9, .pattern = new TriggerPatterns::LineLaunch(&base2d, 1000, 20)},
            {.column = Columns::TRIGGER, .slot = 10, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base down"), .indexMap = zigzagReverse},
            //wings meteor wings in
            {.column = Columns::TRIGGER, .slot = 12, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::TRIGGER, .slot = 13, .pattern = new TriggerPatterns::HeartZoom(&baseMap)},
            {.column = Columns::TRIGGER, .slot = 14, .pattern = new TriggerPatterns::GlitterFade(&cBaseMap)},


            {.column = Columns::MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 1, .pattern = new MaskPatterns::GlowPulseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, true), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, false), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 6, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::MASK, .slot = 7, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},
            

            {.column = Columns::FLASH, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::FLASH, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::FLASH, .slot = 2, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::FLASH, .slot = 3, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::FLASH, .slot = 4, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::FLASH, .slot = 5, .pattern = new LedPatterns::FadeFromRandom(180), .indexMap = zigzag},
            {.column = Columns::FLASH, .slot = 6, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},


            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution, 60)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            {.column = Columns::DEBUG, .slot = 9, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 10, .pattern = new TestPatterns::OrderBarsPattern(distribution, 60, 4, "Order bars zigzag"),  .indexMap = zigzag},
        });
    };

    distributeAndPreview<BAR_V2, RGBA>(hyp,baseSlotPatternsGenerator,&baseMap,distribution,pixelLut);
    // distributeAndMonitor<BGR, RGBA>(hyp,input,&baseMap,distribution,pixelLut);

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

    SlotPatternsGenerator wingsSlotPatternsGenerator = [&](){
        return std::vector<ControlHubInput<RGBA>::SlotPattern>({
            {.column = Columns::BG, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::BG, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::BG, .slot = 2, .pattern = new WingsFadePattern(&cWingsMap, "Primary gradient")},
            // {.column = Columns::BG, .slot = 3, .pattern = new BasePatterns::PaletteGradient(&base2d)}, //TODO
            {.column = Columns::BG, .slot = 4, .pattern = new Wings::WingsBeamsPattern(&wingsPolar)},
            {.column = Columns::BG, .slot = 5, .pattern = new LedPatterns::RibbenFlashPattern(20)},
            {.column = Columns::BG, .slot = 6, .pattern = new LedPatterns::FadeFromRandom(3*60), .indexMap = zigzag},
            {.column = Columns::BG, .slot = 7, .pattern = new VibePatterns::InfernoParticles(&wings2d)},
            {.column = Columns::BG, .slot = 8, .pattern = new Static::RadialSaw(&wingsPolar)},

                
            {.column = Columns::SWEEPS, .slot = 0, .pattern = new Wings::GrowShrink(&wingsPolar)}, 
            {.column = Columns::SWEEPS, .slot = 1, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(&wings2d)}, 
            {.column = Columns::SWEEPS, .slot = 2, .pattern = new Mapped2dPatterns::Lighthouse(&wingsPolar)},
            {.column = Columns::SWEEPS, .slot = 3, .pattern = new Static::AngularSweep(&wingsPolar)},
            // {.column = Columns::SWEEPS, .slot = 4, .pattern = new Wings::MeshPattern(&cWingsMap)}, //BASE ONLY
            // {.column = Columns::SWEEPS, .slot = 5, .pattern = new Wings::BarberPole(&cWingsMap)}, //BASE ONLY
            {.column = Columns::SWEEPS, .slot = 6, .pattern = new Mapped2dPatterns::RadialFadePattern(&wingsPolar)}, 
            {.column = Columns::SWEEPS, .slot = 7, .pattern = new Mapped2dPatterns::DotBeatPattern(&wingsPolar)}, 
            {.column = Columns::SWEEPS, .slot = 8, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(&wingsPolar)},
            {.column = Columns::SWEEPS, .slot = 9, .pattern = new VibePatterns::FireworkBurstPattern(&wings2d)},


            // {.column = Columns::ACCENTS, .slot = 0, .pattern = new Static::HorizontalWave(&base2d)}, //BASE ONLY
            // {.column = Columns::ACCENTS, .slot = 1, .pattern = new BasePatterns::AngularWave(&cBaseMap)}, //BASE ONLY
            // {.column = Columns::ACCENTS, .slot = 2, .pattern = new BasePatterns::RotatingRingsPattern(&cBaseMap)}, //BASE ONLY
            {.column = Columns::ACCENTS, .slot = 3, .pattern = new BasePatterns::SegmentChasePattern(), .indexMap = zigzag},
            {.column = Columns::ACCENTS, .slot = 4, .pattern = new VibePatterns::MeteorShowerPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 5, .pattern = new VibePatterns::BeerBubblesPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 6, .pattern = new VibePatterns::TrianglePulse(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 7, .pattern = new VibePatterns::TriangleStutter(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 8, .pattern = new VibePatterns::TriangleOutlineGrow(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 9, .pattern = new VibePatterns::RotatingTriangle(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 10, .pattern = new VibePatterns::FloatingOrbsPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 11, .pattern = new VibePatterns::MatrixRainPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 12, .pattern = new VibePatterns::BurningTornadoPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 13, .pattern = new VibePatterns::HeartbeatPattern(&wings2d)},
            {.column = Columns::ACCENTS, .slot = 14, .pattern = new Heart(&wingsMap)},
            {.column = Columns::ACCENTS, .slot = 15, .pattern = new VibePatterns::HeartStackPattern(&wings2d)},
            

            // {.column = Columns::ENERGY, .slot = 0, .pattern = new BasePatterns::FlyingEmbersPattern(3*60), .indexMap = zigzag}, //BASE ONLY
            {.column = Columns::ENERGY, .slot = 1, .pattern = new Wings::WingCycleFlashesPattern()},
            {.column = Columns::ENERGY, .slot = 2, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::ENERGY, .slot = 3, .pattern = new HeartZoom(&wingsMap)},


            {.column = Columns::TRIGGER, .slot = 0, .pattern = new TriggerPatterns::SlowPulsePattern()},
            // {.column = Columns::TRIGGER, .slot = 1, .pattern = new TriggerPatterns::FadeFromRandom(60), .indexMap = zigzag },
            {.column = Columns::TRIGGER, .slot = 2, .pattern = new TriggerPatterns::GrowingCirclePattern(&wingsPolar)},  
            // {.column = Columns::TRIGGER, .slot = 3, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base up"), .indexMap = zigzag},
            {.column = Columns::TRIGGER, .slot = 4, .pattern = new TriggerPatterns::Meteor(16, 50, 250, "Meteor wings out"), .indexMap = zigzag},
            // {.column = Columns::TRIGGER, .slot = 5, .pattern = new TriggerPatterns::LineLaunch(&wings2d, 500, 20)},
            {.column = Columns::TRIGGER, .slot = 6, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::TRIGGER, .slot = 7, .pattern = new TriggerPatterns::DoubleFlash(&cWingsMap)},
            // {.column = Columns::TRIGGER, .slot = 8, .pattern = new TriggerPatterns::LineLaunch(&wings2d)},
            // {.column = Columns::TRIGGER, .slot = 9, .pattern = new TriggerPatterns::LineLaunch(&wings2d, 1000, 20)},
            {.column = Columns::TRIGGER, .slot = 10, .pattern = new TriggerPatterns::Meteor(18, 50, 250, "Meteor base down"), .indexMap = zigzagReverse},
            {.column = Columns::TRIGGER, .slot = 11, .pattern = new TriggerPatterns::Meteor(16, 50, 250, "Meteor wings in"), .indexMap = zigzagReverse},
            {.column = Columns::TRIGGER, .slot = 12, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::TRIGGER, .slot = 13, .pattern = new TriggerPatterns::HeartZoom(&wingsMap)},
            {.column = Columns::TRIGGER, .slot = 14, .pattern = new TriggerPatterns::GlitterFade(&cWingsMap)},


            {.column = Columns::MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 1, .pattern = new MaskPatterns::GlowPulseMaskPattern(), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, true), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, false), .indexMap = zigzag},
            {.column = Columns::MASK, .slot = 4, .pattern = new MaskPatterns::WingsFadePattern(&wingsPolar)},
            {.column = Columns::MASK, .slot = 5, .pattern = new MaskPatterns::WingsFadePattern(&wingsPolar,true)},            
            {.column = Columns::MASK, .slot = 6, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::MASK, .slot = 7, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},
            

            {.column = Columns::FLASH, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::FLASH, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::FLASH, .slot = 2, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::FLASH, .slot = 3, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::FLASH, .slot = 4, .pattern = new LedPatterns::SegmentGlitchPattern()},
            // {.column = Columns::FLASH, .slot = 5, .pattern = new LedPatterns::FadeFromRandom()},
            {.column = Columns::FLASH, .slot = 6, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},


            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::DistributionPattern(distribution, 60)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            {.column = Columns::DEBUG, .slot = 9, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 10, .pattern = new TestPatterns::OrderBarsPattern(distribution, 60, 4, "Order bars zigzag"),  .indexMap = zigzag},

        });
    };

    // distributeAndMonitor<BGR, RGBA>(hyp,input,&wingsMap,distribution,pixelLut);
    distributeAndPreview<BAR_V2, RGBA>(hyp,wingsSlotPatternsGenerator,&wingsMap,distribution,pixelLut);
}

void addPaletteColumn(Hyperion *hyp)
{
    auto paletteColumn = new PaletteColumn(
        &hyp->hub,
        0,
        0,
        {

&crimsonTide2,
&toulouse2,
&velvetNoose,
&champagneSin,
&cancan,
&laceDiable,

&vhsTracking,
&crimsonTide,
&matador,


// v
&gaslight,
&midnightCorset,
// X
&bloodMoon,
&redLipLullaby,
&sinfuleTouch,
&lastWaltz,
&scarletPromise,
&burlesqueOath,
&absinthe,
&velvetKiss,
//red


&petitMort,

&cancan2,
&absinthe2,
&petticoat,
&montmartre,
&toulouse,

&cancan3,
&absinthe3,
&petticoat2,
&redlight,
&lautrec,

//synthwave
&midnightChrome,
&laserGrid,

&turboNoir,
&plasmaRewind,

            &emberForge,
            
            &dragonBlood,
            &volcanica,
            &emberGlow,
            &velvetThorn,
            &dragonScale,
            &ember,
            &solstice,
            &cayenne,
            &ember,
            
            &ignition,
            &crimsonRitual,
            &voltageRush,
            &serpentKiss,
            &shatteredNeon,
            &throneOfThorns,




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
