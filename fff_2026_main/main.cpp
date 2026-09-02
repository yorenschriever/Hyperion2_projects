#include "common/distributeAndMonitor.hpp"
// #include "common/dmxAndMonitor.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-mapped-2d.hpp"
#include "common/patterns/patterns-mapped-3d.hpp"
#include "common/patterns/patterns-mask.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-trigger.hpp"

#include "core/hyperion.hpp"
#include "mapping/domeMap.hpp"
#include "mapping/obeliskMap.hpp"
#include "mapping/stageMap.hpp"

#include "patterns/patterns-aerial.hpp"
#include "patterns/patterns-all.hpp"
#include "patterns/patterns-dome.hpp"
#include "patterns/patterns-obelisk.hpp"
#include "patterns/patterns-stage.hpp"
#include "patterns/patterns-vibe.hpp"

#include "palettes.hpp"

void addDomeChain();
void addStageChain();
void addObeliskChain();
void addAerialChain();
void addLightningChain();
void addPaletteColumn();

LUT *ledLut = new ColorCorrectionLUT(2.7, 255, 255, 255, 255);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);

#define SHOW_DEBUG true

enum Columns
{
    PALETTE,
    STAGE_BG,
    STAGE_FG,
    STAGE_MASK,
    OBELISK,
    OBELISK_MASK,
    AERIAL,
ALL,

    DOME_BG,
    DOME_FG,
    DOME_MASK,
    STAGE_PULSE,
    OBELISK_PULSE,
    AERIAL_PULSE,
    DOME_PULSE,
    LIGHTNING,

    DEBUG
};

Hyperion hyp;

int main()
{
    addPaletteColumn();
    addDomeChain();
    addStageChain();
    addObeliskChain();
    addAerialChain();
    addLightningChain();

    hyp.hub.setColumnName(Columns::PALETTE, "Palette");
    hyp.hub.setColumnName(Columns::ALL, "All");
    hyp.hub.setColumnName(Columns::STAGE_BG, "Stage BG");
    hyp.hub.setColumnName(Columns::STAGE_FG, "Stage FG");
    hyp.hub.setColumnName(Columns::STAGE_MASK, "Stage Mask");    
    hyp.hub.setColumnName(Columns::OBELISK, "Obelisk");
    hyp.hub.setColumnName(Columns::OBELISK_MASK, "Obelisk Mask");
    hyp.hub.setColumnName(Columns::AERIAL, "Aerial");
    hyp.hub.setColumnName(Columns::DOME_BG, "Dome BG");
    hyp.hub.setColumnName(Columns::DOME_FG, "Dome FG");
    hyp.hub.setColumnName(Columns::DOME_MASK, "Dome Mask");
    hyp.hub.setColumnName(Columns::STAGE_PULSE, "Stage Pulse");
    hyp.hub.setColumnName(Columns::OBELISK_PULSE, "Obelisk Pulse");
    hyp.hub.setColumnName(Columns::AERIAL_PULSE, "Aerial Pulse");
    hyp.hub.setColumnName(Columns::DOME_PULSE, "Dome Pulse");
    hyp.hub.setColumnName(Columns::LIGHTNING, "Lightning");

    hyp.hub.buttonPressed(Columns::PALETTE, 0);
    hyp.hub.setForcedSelection(Columns::PALETTE);
    hyp.hub.setFlashColumn(Columns::PALETTE, false, true);

    hyp.hub.setFlashColumn(Columns::STAGE_PULSE);
    hyp.hub.setFlashColumn(Columns::OBELISK_PULSE);
    hyp.hub.setFlashColumn(Columns::AERIAL_PULSE);
    hyp.hub.setFlashColumn(Columns::DOME_PULSE);
    hyp.hub.setFlashColumn(Columns::LIGHTNING);

    hyp.start();

    Tempo::AddSource(new ConstantTempo(120));

    while (1)
        Thread::sleep(1000);
}

void addDomeChain()
{

    auto map = new PixelMap3d(createDomeMap());
    auto cmap = map->toCylindricalXY();
    auto flatmap = map->toTopView();
    auto pmap = flatmap->toPolar();

    auto frontMap = new PixelMap(normalizeMap(*(map->toFrontView())));

    auto allMap = new PixelMap3d(normalizeStage(*map));
    auto callMap = allMap->toCylindricalXY();
    auto allFlatMap = allMap->toTopView();

    int nLeds = map->size();
    auto *zigzag = new FlipMapper(240);
    zigzag->flip(60, 60)->flip(180, 60);

    Distribution distribution = {
        {"hypernode6.local", 9611, 4 * 60},
        {"hypernode7.local", 9611, 4 * 60},
        {"hypernode8.local", 9611, 4 * 60},
        {"hypernode9.local", 9611, 4 * 60},
        {"hypernode10.local", 9611, 4 * 60},

        {"hypernode6.local", 9615, 4 * 60},
        {"hypernode7.local", 9615, 4 * 60},
        {"hypernode8.local", 9615, 4 * 60},
        {"hypernode9.local", 9615, 4 * 60},
        {"hypernode10.local", 9615, 4 * 60},
    };

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::DOME_BG, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::DOME_BG, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::DOME_BG, .slot = 2, .pattern = new LedPatterns::DuoTonePattern(2 * 60)},
            {.column = Columns::DOME_BG, .slot = 3, .pattern = new LedPatterns::DuoTonePattern(60), .indexMap = zigzag},
            {.column = Columns::DOME_BG, .slot = 4, .pattern = new LedPatterns::GradientPattern(60), .indexMap = zigzag},
            {.column = Columns::DOME_BG, .slot = 5, .pattern = new Mapped2dPatterns::HorizontalGradientPattern(flatmap)},
            {.column = Columns::DOME_BG, .slot = 6, .pattern = new LedPatterns::BarChase(60)},

            {.column = Columns::DOME_FG, .slot = 0, .pattern = new DomePatterns::XY(map)},
            {.column = Columns::DOME_FG, .slot = 1, .pattern = new DomePatterns::Z(map)},
            {.column = Columns::DOME_FG, .slot = 2, .pattern = new DomePatterns::DotBeatPattern(cmap)},
            {.column = Columns::DOME_FG, .slot = 3, .pattern = new Mapped2dPatterns::Lighthouse(pmap)},
            {.column = Columns::DOME_FG, .slot = 4, .pattern = new Mapped2dPatterns::HorizontalSin(pmap)},
            {.column = Columns::DOME_FG, .slot = 5, .pattern = new Mapped2dPatterns::RadialFadePattern(pmap)},
            {.column = Columns::DOME_FG, .slot = 6, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(pmap)},
            
            {.column = Columns::DOME_MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern()},
            {.column = Columns::DOME_MASK, .slot = 1, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::DOME_MASK, .slot = 2, .pattern = new MaskPatterns::GlowPulseMaskPattern()},
            {.column = Columns::DOME_MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern(), .indexMap = zigzag},
            {.column = Columns::DOME_MASK, .slot = 4, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::DOME_MASK, .slot = 5, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},
            {.column = Columns::DOME_MASK, .slot = 6, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},

            {.column = Columns::ALL, .slot = 0, .pattern = new TriggerPatterns::LineLaunch(allFlatMap, 200, 40, -1)},
            {.column = Columns::ALL, .slot = 1, .pattern = new AllPatterns::Lighthouse(callMap)},
            {.column = Columns::ALL, .slot = 2, .pattern = new AllPatterns::GrowingCirclesPattern(allMap)},
            {.column = Columns::ALL, .slot = 3, .pattern = new AllPatterns::TopChase(callMap)},

            {.column = Columns::DOME_PULSE, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::DOME_PULSE, .slot = 1, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::DOME_PULSE, .slot = 2, .pattern = new LedPatterns::BeatShakePattern(2*60)},
            {.column = Columns::DOME_PULSE, .slot = 3, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::DOME_PULSE, .slot = 4, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::DOME_PULSE, .slot = 5, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::DOME_PULSE, .slot = 6, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::DOME_PULSE, .slot = 7, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::DOME_PULSE, .slot = 8, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::DOME_PULSE, .slot = 9, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base up"), .indexMap = zigzag},
            {.column = Columns::DOME_PULSE, .slot = 10, .pattern = new TriggerPatterns::LineLaunch(frontMap, 100, 20)},
            {.column = Columns::DOME_PULSE, .slot = 11, .pattern = new TriggerPatterns::LineLaunch(frontMap, 500, 20)},
            {.column = Columns::DOME_PULSE, .slot = 12, .pattern = new TriggerPatterns::LineLaunch(frontMap, 1000, 20)},
           

            // {.column = Columns::LEDBARS2, .slot = 0, .pattern = new LedPatterns::GlowPattern()},
            // {.column = Columns::LEDBARS2, .slot = 1, .pattern = new LedPatterns::GlowPulsePattern()},
            // {.column = Columns::LEDBARS2, .slot = 2, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap=zigzag},
            // {.column = Columns::LEDBARS2, .slot = 3, .pattern = new LedPatterns::SinPattern(), .indexMap=zigzag},
            // {.column = Columns::LEDBARS2, .slot = 4, .pattern = new LedPatterns::GradientChasePattern(), .indexMap=zigzag},
            // {.column = Columns::LEDBARS2, .slot = 5, .pattern = new LedPatterns::FadeFromRandom(), .indexMap=zigzag},

            // {.column = Columns::LEDBARS_TRIGGER, .slot = 0, .pattern = new LedPatterns::FlashesPattern()},
            // {.column = Columns::LEDBARS_TRIGGER, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            // {.column = Columns::LEDBARS_TRIGGER, .slot = 2, .pattern = new LedPatterns::PixelGlitchPattern()},
            // {.column = Columns::LEDBARS_TRIGGER, .slot = 3, .pattern = new LedPatterns::FadingNoisePattern()},
            // {.column = Columns::LEDBARS_TRIGGER, .slot = 4, .pattern = new LedPatterns::StrobeHighlightPattern()},
            // {.column = Columns::LEDBARS_TRIGGER, .slot = 5, .pattern = new LedPatterns::SegmentGlitchPattern()},

            #if SHOW_DEBUG
            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            #endif

        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addStageChain()
{

    auto map = new PixelMap3d(createStageMap());
    auto frontMap = new PixelMap(resizeAndTranslateMap(normalizeMap(*(map->toFrontView()), true), 1, -1, 0, -0.6));
    auto pfrontMap = frontMap->toPolar();
    auto frontMapUpsideDown = new PixelMap(resizeAndTranslateMap(*frontMap, 1, -1, 0, 0));
    auto flatmap = map->toTopView();

    auto allMap = new PixelMap3d(normalizeStage(*map));
    auto callMap = allMap->toCylindricalXY();
    auto allFlatMap = allMap->toTopView();

    int nLeds = map->size();

    FlipMapper *zigzag = new FlipMapper(nLeds);
    FlipMapper *zigzagReverse = new FlipMapper(nLeds);
    for (int i = 0, start = 0; i < stageMapSegmentSizes.size(); i++)
    {
        if (i % 2 == 1 ^ (i / 3) % 2 == 0){
            zigzag->flip(start, stageMapSegmentSizes[i]);
            zigzagReverse->flip(start, stageMapSegmentSizes[i]);
        }
        start += stageMapSegmentSizes[i];
    }
    zigzagReverse->flip(0, nLeds);

    Distribution distribution = {
        // dak
        {"hyperslaveX.local", 9611, 3 * 60},
        {"hyperslaveX.local", 9612, 3 * 60},
        {"hyperslaveX.local", 9613, 6 * 60},
        {"hyperslaveX.local", 9614, 6 * 60},

        // backdrop
        {"hyperslave1.local", 9611, 3 * 60},
        {"hyperslave1.local", 9612, 3 * 60},
        {"hyperslave1.local", 9613, 6 * 60},
        {"hyperslave1.local", 9614, 6 * 60},

        // voorkant
        {"hyperslave1.local", 9615, 3 * 60},
        {"hyperslave1.local", 9616, 3 * 60},
    };

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::STAGE_BG, .slot = 0, .pattern = new StagePatterns::StaticGradientPattern(stageMapSegmentSizes), .indexMap = zigzag},
            {.column = Columns::STAGE_BG, .slot = 1, .pattern = new StagePatterns::StaticGradientPattern(stageMapSegmentSizes)},
            {.column = Columns::STAGE_BG, .slot = 2, .pattern = new StagePatterns::StaticGradientTripletPattern(stageMapSegmentSizes)},
            {.column = Columns::STAGE_BG, .slot = 3, .pattern = new Mapped2dPatterns::RadialGradientPattern(pfrontMap)},
            {.column = Columns::STAGE_BG, .slot = 4, .pattern = new Mapped2dPatterns::HorizontalGradientPattern(frontMap)},
            {.column = Columns::STAGE_BG, .slot = 5, .pattern = new Mapped2dPatterns::VerticalGradientPattern(frontMap)},
            {.column = Columns::STAGE_BG, .slot = 6, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::STAGE_BG, .slot = 7, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::STAGE_BG, .slot = 8, .pattern = new LedPatterns::FlashesPattern()},
     
            {.column = Columns::STAGE_FG, .slot = 0, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},
            {.column = Columns::STAGE_FG, .slot = 1, .pattern = new Mapped2dPatterns::Lighthouse(pfrontMap)},
            {.column = Columns::STAGE_FG, .slot = 2, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 3, .pattern = new Mapped2dPatterns::DotBeatPattern(pfrontMap)},
            {.column = Columns::STAGE_FG, .slot = 4, .pattern = new Mapped2dPatterns::HorizontalSin(pfrontMap)},
            {.column = Columns::STAGE_FG, .slot = 5, .pattern = new Mapped2dPatterns::RadialFadePattern(pfrontMap)},
            {.column = Columns::STAGE_FG, .slot = 6, .pattern = new LedPatterns::RibbenFlashPattern(20)},
            {.column = Columns::STAGE_FG, .slot = 7, .pattern = new LedPatterns::FadeFromRandom(60), .indexMap = zigzag},
            {.column = Columns::STAGE_FG, .slot = 8, .pattern = new VibePatterns::FireworkBurstPattern(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 9, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap = zigzag},
            {.column = Columns::STAGE_FG, .slot = 10, .pattern = new VibePatterns::MeteorShowerPattern(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 11, .pattern = new VibePatterns::BeerBubblesPattern(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 12, .pattern = new VibePatterns::TrianglePulse(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 13, .pattern = new VibePatterns::TriangleStutter(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 14, .pattern = new VibePatterns::TriangleOutlineGrow(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 15, .pattern = new VibePatterns::RotatingTriangle(frontMap)},
            {.column = Columns::STAGE_FG, .slot = 16, .pattern = new VibePatterns::FloatingOrbsPattern(frontMap)},

            {.column = Columns::STAGE_MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern()},
            {.column = Columns::STAGE_MASK, .slot = 1, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            {.column = Columns::STAGE_MASK, .slot = 2, .pattern = new MaskPatterns::GlowPulseMaskPattern()},
            {.column = Columns::STAGE_MASK, .slot = 3, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::STAGE_MASK, .slot = 4, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},
            {.column = Columns::STAGE_MASK, .slot = 5, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},

            {.column = Columns::ALL, .slot = 0, .pattern = new TriggerPatterns::LineLaunch(allFlatMap, 200, 150, -1)},
            {.column = Columns::ALL, .slot = 1, .pattern = new AllPatterns::Lighthouse(callMap)},
            {.column = Columns::ALL, .slot = 2, .pattern = new AllPatterns::GrowingCirclesPattern(allMap)},
            {.column = Columns::ALL, .slot = 3, .pattern = new AllPatterns::TopChase(callMap)},

            {.column = Columns::STAGE_PULSE, .slot = 0, .pattern = new LedPatterns::BeatShakePattern(20)},
            {.column = Columns::STAGE_PULSE, .slot = 1, .pattern = new TriggerPatterns::GrowingCirclePattern(pfrontMap)},
            {.column = Columns::STAGE_PULSE, .slot = 2, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::STAGE_PULSE, .slot = 3, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base up"), .indexMap = zigzagReverse},
            {.column = Columns::STAGE_PULSE, .slot = 4, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base down"), .indexMap = zigzag},
            {.column = Columns::STAGE_PULSE, .slot = 5, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::STAGE_PULSE, .slot = 6, .pattern = new TriggerPatterns::FadeFromRandom(60)},
            {.column = Columns::STAGE_PULSE, .slot = 7, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::STAGE_PULSE, .slot = 8, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::STAGE_PULSE, .slot = 9, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::STAGE_PULSE, .slot = 10, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::STAGE_PULSE, .slot = 11, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::STAGE_PULSE, .slot = 12, .pattern = new LedPatterns::BeatShakePattern(2 * 60)},
            {.column = Columns::STAGE_PULSE, .slot = 13, .pattern = new LedPatterns::BeatShakePattern(10)},
            {.column = Columns::STAGE_PULSE, .slot = 14, .pattern = new TriggerPatterns::LineLaunch(frontMapUpsideDown, 100, 20)},
            {.column = Columns::STAGE_PULSE, .slot = 15, .pattern = new TriggerPatterns::LineLaunch(frontMapUpsideDown, 500, 20)},
            {.column = Columns::STAGE_PULSE, .slot = 16, .pattern = new TriggerPatterns::LineLaunch(frontMapUpsideDown, 1000, 20)},

            #if SHOW_DEBUG
            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            #endif
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
    // distributeAndMonitor<BGR>(&hyp, input, frontMap, distribution, ledLut, 0.01);
}

void addObeliskChain()
{

    auto map = new PixelMap3d(createObeliskMap());
    auto cmap = map->toCylindricalXY();
    auto map2d = new PixelMap(normalizeMap(*(map->toSideView())));
    auto flatmap = map->toTopView();

    auto allMap = new PixelMap3d(normalizeStage(*map));
    auto callMap = allMap->toCylindricalXY();
    auto allFlatMap = allMap->toTopView();

    int nLeds = map->size();
    // IndexMap *zigzag = new ZigZagMapper(60, true);
    FlipMapper *reverseMap = new FlipMapper(2 * 60);
    reverseMap->flip(0, 120);

    Distribution distribution = {
        {"hypernode1.local", 9611, 3 * 2 * 60},
        {"hypernode2.local", 9611, 3 * 2 * 60},
        {"hypernode3.local", 9611, 3 * 2 * 60},
        {"hypernode4.local", 9611, 3 * 2 * 60},
        {"hypernode5.local", 9611, 3 * 2 * 60},
    };

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::OBELISK, .slot = 0, .pattern = new LedPatterns::BeatShakePattern(2 * 60)},
            {.column = Columns::OBELISK, .slot = 1, .pattern = new TriggerPatterns::GlitterFade(cmap)},
            {.column = Columns::OBELISK, .slot = 2, .pattern = new ObeliskPatterns::HorizontalSin(cmap)},
            {.column = Columns::OBELISK, .slot = 3, .pattern = new ObeliskPatterns::VerticallyIsolated(cmap)},
            {.column = Columns::OBELISK, .slot = 4, .pattern = new ObeliskPatterns::StaticGradientPattern(map)},
            {.column = Columns::OBELISK, .slot = 5, .pattern = new ObeliskPatterns::OnBeatColumnChaseUpPattern(map)},
            {.column = Columns::OBELISK, .slot = 6, .pattern = new ObeliskPatterns::GrowShrink(cmap)},
            {.column = Columns::OBELISK, .slot = 7, .pattern = new ObeliskPatterns::RotatingRingsPattern(cmap)},
            {.column = Columns::OBELISK, .slot = 8, .pattern = new LedPatterns::FadeFromRandom(2 * 60)},
            {.column = Columns::OBELISK, .slot = 9, .pattern = new ObeliskPatterns::FlyingEmbersPattern(3 * 60)},
            {.column = Columns::OBELISK, .slot = 10, .pattern = new LedPatterns::SegmentChasePattern(2 * 60)},
            
            {.column = Columns::OBELISK_MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern()},
            {.column = Columns::OBELISK_MASK, .slot = 1, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = reverseMap},
            {.column = Columns::OBELISK_MASK, .slot = 2, .pattern = new MaskPatterns::GlowPulseMaskPattern()},
            {.column = Columns::OBELISK_MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern()},
            {.column = Columns::OBELISK_MASK, .slot = 4, .pattern = new MaskPatterns::SideChainCompressorMask()},
            {.column = Columns::OBELISK_MASK, .slot = 5, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},
            {.column = Columns::OBELISK_MASK, .slot = 6, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},

            {.column = Columns::ALL, .slot = 0, .pattern = new TriggerPatterns::LineLaunch(allFlatMap, 200, 40, -1)},
            {.column = Columns::ALL, .slot = 1, .pattern = new AllPatterns::Lighthouse(callMap)},
            {.column = Columns::ALL, .slot = 2, .pattern = new AllPatterns::GrowingCirclesPattern(allMap)},
            {.column = Columns::ALL, .slot = 3, .pattern = new AllPatterns::TopChase(callMap)},

            {.column = Columns::OBELISK_PULSE, .slot = 0, .pattern = new LedPatterns::BeatShakePattern(2*60)},
            {.column = Columns::OBELISK_PULSE, .slot = 1, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 2, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base up"), .indexMap = reverseMap},
            {.column = Columns::OBELISK_PULSE, .slot = 3, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base down"),},{.column = Columns::STAGE_PULSE, .slot = 5, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 4, .pattern = new TriggerPatterns::FadeFromRandom(2*60)},
            {.column = Columns::OBELISK_PULSE, .slot = 5, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 6, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 7, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 8, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 9, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::OBELISK_PULSE, .slot = 10, .pattern = new LedPatterns::BeatShakePattern(20)},
            {.column = Columns::OBELISK_PULSE, .slot = 11, .pattern = new TriggerPatterns::LineLaunch(map2d, 100, 20)},
            {.column = Columns::OBELISK_PULSE, .slot = 12, .pattern = new TriggerPatterns::LineLaunch(map2d, 500, 20)},
            {.column = Columns::OBELISK_PULSE, .slot = 13, .pattern = new TriggerPatterns::LineLaunch(map2d, 1000, 20)},
            {.column = Columns::OBELISK_PULSE, .slot = 14, .pattern = new TriggerPatterns::SlowPulsePattern()},
            
            

            #if SHOW_DEBUG
            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            #endif
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addAerialChain()
{

    auto map = new PixelMap3d(createObeliskAerialMap());
    auto flatmap = map->toTopView();

    auto allMap = new PixelMap3d(normalizeStage(*map));
    auto callMap = allMap->toCylindricalXY();
    auto allFlatMap = allMap->toTopView();

    int nLeds = map->size();
    IndexMap *zigzag = new ZigZagMapper(3 * 60, true);

    Distribution distribution = {
        {"hypernode1.local", 9615, 2 * 3 * 60},
        {"hypernode2.local", 9615, 2 * 3 * 60},
        {"hypernode3.local", 9615, 2 * 3 * 60},
        {"hypernode4.local", 9615, 2 * 3 * 60},
        {"hypernode5.local", 9615, 2 * 3 * 60},
    };

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::AERIAL, .slot = 0, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::AERIAL, .slot = 1, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::AERIAL, .slot = 2, .pattern = new LedPatterns::DuoTonePattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = 3, .pattern = new LedPatterns::DuoToneGradientPattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = 4, .pattern = new LedPatterns::GradientPattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = 5, .pattern = new LedPatterns::GradientPattern(3 * 60), .indexMap = zigzag},
            {.column = Columns::AERIAL, .slot = 6, .pattern = new AerialPatterns::SinChasePattern()},
            {.column = Columns::AERIAL, .slot = 7, .pattern = new AerialPatterns::SinChasePattern(), .indexMap = zigzag},
            {.column = Columns::AERIAL, .slot = 8, .pattern = new LedPatterns::RibbenFlashPattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = 9, .pattern = new LedPatterns::RibbenClivePattern<Glow>(1000, 1, 0.025, 3 * 60)},
            {.column = Columns::AERIAL, .slot = 10, .pattern = new LedPatterns::RibbenClivePattern<Glow>(10000, 1, 0.025, 3 * 60)},

            {.column = Columns::AERIAL_PULSE, .slot = 0, .pattern = new AerialPatterns::DoubleFlash()},
            {.column = Columns::AERIAL_PULSE, .slot = 1, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 2, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 3, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 4, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 5, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 6, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 7, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::AERIAL_PULSE, .slot = 8, .pattern = new LedPatterns::BeatShakePattern(20)},

            #if SHOW_DEBUG
            {.column = Columns::DEBUG, .slot = 0, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
            {.column = Columns::DEBUG, .slot = 1, .pattern = new TestPatterns::OneColor(RGB(255, 0, 0), "Red")},
            {.column = Columns::DEBUG, .slot = 2, .pattern = new TestPatterns::OneColor(RGB(0, 255, 0), "Green")},
            {.column = Columns::DEBUG, .slot = 3, .pattern = new TestPatterns::OneColor(RGB(0, 0, 255), "Blue")},
            {.column = Columns::DEBUG, .slot = 4, .pattern = new TestPatterns::OneColor(RGB(255, 255, 255), "White")},
            {.column = Columns::DEBUG, .slot = 5, .pattern = new TestPatterns::OneColor(RGB(127, 127, 127), "White 50%")},
            {.column = Columns::DEBUG, .slot = 6, .pattern = new TestPatterns::Palette(60, 20)},
            {.column = Columns::DEBUG, .slot = 7, .pattern = new TestPatterns::Gamma(10)},
            {.column = Columns::DEBUG, .slot = 8, .pattern = new TestPatterns::Motion()},
            #endif
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addLightningChain()
{
    int size = 10;

    Distribution distribution = {
        {"hypernodeX.local", 9620, 10},
    };

    auto input = new ControlHubInput<Monochrome>(
        size,
        &hyp.hub,
        {
            {.column = Columns::LIGHTNING, .slot = 0, .pattern = new MonochromePatterns::BeatStepPattern()},
            {.column = Columns::LIGHTNING, .slot = 1, .pattern = new MonochromePatterns::BeatSingleFadePattern()},
            {.column = Columns::LIGHTNING, .slot = 2, .pattern = new MonochromePatterns::BeatMultiFadePattern()},
            {.column = Columns::LIGHTNING, .slot = 3, .pattern = new MonochromePatterns::BeatShakePattern()},
            {.column = Columns::LIGHTNING, .slot = 4, .pattern = new MonochromePatterns::SlowStrobePattern()},
            {.column = Columns::LIGHTNING, .slot = 5, .pattern = new MonochromePatterns::FastStrobePattern()},  // millis
            {.column = Columns::LIGHTNING, .slot = 6, .pattern = new MonochromePatterns::FastStrobePattern2()}, // frames
            {.column = Columns::LIGHTNING, .slot = 7, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::LIGHTNING, .slot = 8, .pattern = new MonochromePatterns::SingleGlitchPattern()},

        });

    auto map = new PixelMap3d(
        resizeAndTranslateMap3d(
            rotate3d(circleMap3d(size, 0.1), 90, (float[3]){1, 0, 0}),
            1.0,
            0, 1, 0.8));

    distributeAndMonitor<Monochrome12>(&hyp, input, map, distribution, GammaLut12, 0.02);
}

void addPaletteColumn()
{
    auto paletteColumn = new PaletteColumn(
        &hyp.hub,
        0,
        0,
        {
            &rejected_hypothesis,
            &metamorphosis,
            &neon_fracture,
            &volatile_reactions,
            &electroflux,
            &vhsTracking,
            &plasmaRewind,

            &mad_science,
            &chromatic_frost,
            &bioluminescent_grove,
            &electro_pulse,
            &volatile_synthesis,
            &crimson_reverie,
            &ionosphere,

            &cancan,
            
            &scarletPromise,
            &midnightChrome,
            &laserGrid,
            &turboNoir,
            &velvetThorn,

            &pinkSunset,
            &heatmap,
            &campfire,
            &retro,
            &tunnel,
            &heatmap2,
            &redSalvation,

            // &sunset4,
            // &blueOrange,
            // &purpleGreen,

            // &sunset2,
            // &sunset8,
            // &heatmap2,
            // &sunset3,
            // &sunset6,
            // &sunset7,
            // &coralTeal,
            // &deepBlueOcean,
            // &plumBath,
            // &sunset1,
            // &candy,
            // &greatBarrierReef,
            // &peach,
            // &denseWater,
            // &sunset5,
            // &salmonOnIce,
        });
    hyp.hub.subscribe(paletteColumn);
}
