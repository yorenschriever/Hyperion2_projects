#include "common/distributeAndMonitor.hpp"
// #include "common/dmxAndMonitor.hpp"
#include "common/patterns/patterns-led.hpp"
#include "common/patterns/patterns-mask.hpp"
#include "common/patterns/patterns-monochrome.hpp"
#include "common/patterns/patterns-test.hpp"
#include "common/patterns/patterns-mapped-2d.hpp"
#include "common/patterns/patterns-mapped-3d.hpp"
#include "common/patterns/patterns-trigger.hpp"

#include "core/hyperion.hpp"
#include "mapping/domeMap.hpp"
#include "mapping/obeliskMap.hpp"
#include "mapping/stageMap.hpp"

#include "patterns/patterns-dome.hpp"
#include "patterns/patterns-all.hpp"
#include "patterns/patterns-obelisk.hpp"
#include "patterns/patterns-stage.hpp"
#include "patterns/patterns-vibe.hpp"
#include "patterns/patterns-aerial.hpp"

void addDomeChain();
void addStageChain();
void addObeliskChain();
void addAerialChain();
void addLightningChain();
void addPaletteColumn();

LUT *ledLut = new ColorCorrectionLUT(2.7, 255, 255, 255, 255);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);

enum Columns
{
    PALETTE,

    DOME,
    STAGE,
    OBELISK,
    AERIAL,
    LIGHTNING,
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
    hyp.hub.setColumnName(Columns::DOME, "Dome");
    hyp.hub.setColumnName(Columns::STAGE, "Stage");
    hyp.hub.setColumnName(Columns::OBELISK, "Obelisk");
    hyp.hub.setColumnName(Columns::AERIAL, "Aerial");
    // hyp.hub.setColumnName(Columns::LEDBARS2, "Ledbars FG");
    // hyp.hub.setColumnName(Columns::LEDBARS_TRIGGER, "Ledbars Tr");
    // hyp.hub.setColumnName(Columns::SPARKS, "Sparks");

    // hyp.hub.setFlashColumn(Columns::LEDBARS_TRIGGER);
    // hyp.hub.setFlashColumn(Columns::SPARKS);

    hyp.hub.buttonPressed(Columns::PALETTE, 0);
    hyp.hub.setForcedSelection(Columns::PALETTE);
    hyp.hub.setFlashColumn(Columns::PALETTE, false, true);

    // hyp.hub.buttonPressed(Columns::LEDBARS, 8);

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

    int nLeds = map->size();
    auto *zigzag = new FlipMapper(240);
    zigzag->flip(60, 60)->flip(180, 60);

    Distribution distribution = {
        {"hypernode3.local", 9611, 4 * 60},
        {"hypernode3.local", 9612, 4 * 60},
        {"hypernode3.local", 9613, 4 * 60},
        {"hypernode3.local", 9614, 4 * 60},
        {"hypernode3.local", 9615, 4 * 60},
        {"hypernode3.local", 9616, 4 * 60},
        {"hypernode3.local", 9617, 4 * 60},
        {"hypernode3.local", 9618, 4 * 60},

        {"hypernode4.local", 9611, 4 * 60},
        {"hypernode4.local", 9612, 4 * 60},
        {"hypernode4.local", 9613, 4 * 60},
        {"hypernode4.local", 9614, 4 * 60},
        // {"hypernode4.local",9615,4*60},
        // {"hypernode4.local",9616,4*60},
        // {"hypernode4.local",9617,4*60},
        // {"hypernode4.local",9618,4*60},
    };

    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::DOME, .slot = i++, .pattern = new DomePatterns::XY(map)},
            {.column = Columns::DOME, .slot = i++, .pattern = new DomePatterns::Z(map)},
            {.column = Columns::DOME, .slot = i++, .pattern = new DomePatterns::DotBeatPattern(cmap)},
            {.column = Columns::DOME, .slot = i++, .pattern = new Mapped2dPatterns::Lighthouse(pmap)},
            {.column = Columns::DOME, .slot = i++, .pattern = new Mapped2dPatterns::HorizontalSin(pmap)},
            {.column = Columns::DOME, .slot = i++, .pattern = new Mapped2dPatterns::RadialFadePattern(pmap)},
            {.column = Columns::DOME, .slot = i++, .pattern = new Mapped2dPatterns::RadialGlitterFadePattern(pmap)},
            {.column = Columns::DOME, .slot = i++, .pattern = new Mapped2dPatterns::HorizontalGradientPattern(flatmap)},
            

            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(2 * 60)},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(60), .indexMap = zigzag},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::GradientPattern(60), .indexMap = zigzag},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::OnPattern({255, 0, 0}, "Red")},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::OnPattern({0, 255, 0}, "Green")},
            {.column = Columns::DOME, .slot = i++, .pattern = new LedPatterns::OnPattern({0, 0, 255}, "Blue")},

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

            {.column = Columns::DOME, .slot = i++, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addStageChain()
{

    auto map = new PixelMap3d(createStageMap());
    auto frontMap = new PixelMap(resizeAndTranslateMap(normalizeMap(*(map->toFrontView()), true), 1, -1, 0, -0.6));
    auto pfrontMap = frontMap->toPolar();

    int nLeds = map->size();

    FlipMapper *zigzag = new FlipMapper(nLeds);
    for (int i = 0, start = 0; i < stageMapSegmentSizes.size(); i++)
    {
        if (i%2==1 ^ (i/3)%2==0)
            zigzag->flip(start, stageMapSegmentSizes[i]);
        start += stageMapSegmentSizes[i];
    }

    Distribution distribution = {
        // dak
        {"hypernode1.local", 9611, 3 * 60},
        {"hypernode1.local", 9612, 3 * 60},
        {"hypernode1.local", 9613, 6 * 60},
        {"hypernode1.local", 9614, 6 * 60},

        // backdrop
        {"hypernode2.local", 9611, 3 * 60},
        {"hypernode2.local", 9612, 3 * 60},
        {"hypernode2.local", 9615, 6 * 60},
        {"hypernode2.local", 9616, 6 * 60},

        // voorkant
        {"hypernode2.local", 9613, 3 * 60},
        {"hypernode2.local", 9614, 3 * 60},
    };

    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::STAGE, .slot = i++, .pattern = new StagePatterns::StaticGradientPattern(stageMapSegmentSizes), .indexMap = zigzag},
            {.column = Columns::STAGE, .slot = i++, .pattern = new StagePatterns::StaticGradientPattern(stageMapSegmentSizes)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new StagePatterns::StaticGradientTripletPattern(stageMapSegmentSizes)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},

            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::BeatShakePattern(20)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::RadialGradientPattern(pfrontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::HorizontalGradientPattern(frontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::VerticalGradientPattern(frontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::Lighthouse(pfrontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::GrowingCirclesPattern(frontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::DotBeatPattern(pfrontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::HorizontalSin(pfrontMap)},
            {.column = Columns::STAGE, .slot=i++, .pattern = new Mapped2dPatterns::RadialFadePattern(pfrontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::RibbenFlashPattern(20)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::FadeFromRandom(60), .indexMap = zigzag},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::FireworkBurstPattern(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::SegmentChasePattern(), .indexMap = zigzag},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::MeteorShowerPattern(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::BeerBubblesPattern(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::TrianglePulse(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::TriangleStutter(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::TriangleOutlineGrow(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::RotatingTriangle(frontMap)},
            {.column = Columns::STAGE, .slot = i++, .pattern = new VibePatterns::FloatingOrbsPattern(frontMap)},            
            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::STAGE, .slot = i++, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::STAGE, .slot = i++, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::STAGE, .slot = i++, .pattern = new TriggerPatterns::PulsePattern()},

            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::STAGE, .slot = i++, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},

            {.column = Columns::STAGE, .slot = i++, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
    // distributeAndMonitor<BGR>(&hyp, input, frontMap, distribution, ledLut, 0.01);
}

void addObeliskChain()
{

    auto map = new PixelMap3d(createObeliskMap());
    auto cmap = map->toCylindricalXY();
    auto map2d = new PixelMap(normalizeMap(*(map->toSideView())));

    int nLeds = map->size();
    // IndexMap *zigzag = new ZigZagMapper(60, true);
    FlipMapper *reverseMap = new FlipMapper(2*60);
    reverseMap->flip(0, 120);

    Distribution distribution = {
        {"hypernode5.local", 9611, 3 * 2 * 60},
        // {"hypernode5.local", 9615, 2 * 3 * 60},

        {"hypernode6.local", 9611, 3 * 2 * 60},
        // {"hypernode6.local", 9615, 2 * 3 * 60},

        {"hypernode7.local", 9611, 3 * 2 * 60},
        // {"hypernode7.local", 9615, 2 * 3 * 60},

        {"hypernode8.local", 9611, 3 * 2 * 60},
        // {"hypernode8.local", 9615, 2 * 3 * 60},

        {"hypernode9.local", 9611, 3 * 2 * 60},
        // {"hypernode9.local", 9615, 2 * 3 * 60},
    };

    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::HorizontalSin(cmap)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::VerticallyIsolated(cmap)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::StaticGradientPattern(map)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::OnBeatColumnChaseUpPattern(map)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::GrowShrink(cmap)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::RotatingRingsPattern(cmap)},
            
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::BeatShakePattern(2*60)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::FadeFromRandom(2*60)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new ObeliskPatterns::FlyingEmbersPattern(3*60)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::FadingNoisePattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::LineLaunch(map2d, 100, 20)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::LineLaunch(map2d, 500, 20)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::LineLaunch(map2d, 1000, 20)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::SlowPulsePattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::FadeFromRandom(2*60) },
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base up"), .indexMap = reverseMap},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::Meteor(15, 50, 600, "Meteor base down"), },
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::PulsePattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new TriggerPatterns::GlitterFade(cmap)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::FlashesPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::StrobePattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::StrobeHighlightPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::PixelGlitchPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::FadeFromRandom(2*60)},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new MaskPatterns::SegmentGlitchMaskPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::SegmentGlitchPattern()},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::SegmentChasePattern(2*60)},

            // {.column = Columns::MASK, .slot = 0, .pattern = new MaskPatterns::SinChaseMaskPattern(), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 1, .pattern = new MaskPatterns::GlowPulseMaskPattern(), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 2, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, true), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 3, .pattern = new MaskPatterns::SegmentGradientMaskPattern(3*60, false), .indexMap = zigzag},
            // {.column = Columns::MASK, .slot = 6, .pattern = new MaskPatterns::SideChainCompressorMask()},
            // {.column = Columns::MASK, .slot = 7, .pattern = new MaskPatterns::RibbenFlashMaskPattern(20)},
            




            // {.column = Columns::OBELISK, .slot = i++, .pattern = new Min::SegmentChasePattern()},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new Min::LineLaunch(&columnMap3d)},

            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(6 * 60)},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(30), .indexMap = zigzag},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(60), .indexMap = zigzag},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::GradientPattern(6 * 60, 60), .indexMap = zigzag},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::GradientPattern(6 * 60), .indexMap = zigzag},
            // {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::GradientPattern(60), .indexMap = zigzag},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::OnPattern({255, 0, 0}, "Red")},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::OnPattern({0, 255, 0}, "Green")},
            {.column = Columns::OBELISK, .slot = i++, .pattern = new LedPatterns::OnPattern({0, 0, 255}, "Blue")},

            {.column = Columns::OBELISK, .slot = i++, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addAerialChain()
{

    auto map = new PixelMap3d(createObeliskAerialMap());

    int nLeds = map->size();
    IndexMap *zigzag = new ZigZagMapper(3*60, true);

    Distribution distribution = {
        // {"hypernode5.local", 9611, 3 * 2 * 60},
        {"hypernode5.local", 9615, 2 * 3 * 60},

        // {"hypernode6.local", 9611, 3 * 2 * 60},
        {"hypernode6.local", 9615, 2 * 3 * 60},

        // {"hypernode7.local", 9611, 3 * 2 * 60},
        {"hypernode7.local", 9615, 2 * 3 * 60},

        // {"hypernode8.local", 9611, 3 * 2 * 60},
        {"hypernode8.local", 9615, 2 * 3 * 60},

        // {"hypernode9.local", 9611, 3 * 2 * 60},
        {"hypernode9.local", 9615, 2 * 3 * 60},
    };

    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nLeds,
        &hyp.hub,
        {
            {.column = Columns::AERIAL, .slot = i++, .pattern = new AerialPatterns::DoubleFlash()},

            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::PalettePattern(0, "Primary")},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::PalettePattern(1, "Secondary")},

            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::DuoTonePattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::DuoToneGradientPattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::GradientPattern(3 * 60)},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new LedPatterns::GradientPattern(3 * 60), .indexMap = zigzag},

            {.column = Columns::AERIAL, .slot = i++, .pattern = new AerialPatterns::GradientChasePattern()},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new AerialPatterns::GradientChasePattern(), .indexMap = zigzag},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new AerialPatterns::SinChasePattern()},
            {.column = Columns::AERIAL, .slot = i++, .pattern = new AerialPatterns::SinChasePattern(), .indexMap = zigzag},

            {.column = Columns::AERIAL, .slot = i++, .pattern = new TestPatterns::OrderBarsPattern(distribution)},
        });

    distributeAndMonitor<BGR>(&hyp, input, map, distribution, ledLut, 0.01);
}

void addLightningChain()
{
    int size = 10;

    Distribution distribution = {
        {"hyperslave4.local",9620,10},
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
            {.column = Columns::LIGHTNING, .slot = 5, .pattern = new MonochromePatterns::FastStrobePattern()}, //millis
            {.column = Columns::LIGHTNING, .slot = 6, .pattern = new MonochromePatterns::FastStrobePattern2()}, //frames
            {.column = Columns::LIGHTNING, .slot = 7, .pattern = new MonochromePatterns::GlitchPattern()},
            {.column = Columns::LIGHTNING, .slot = 8, .pattern = new MonochromePatterns::SingleGlitchPattern()},
            
        });

    auto map = new PixelMap3d(
        resizeAndTranslateMap3d(
            rotate3d(circleMap3d(size, 0.1), 90, (float[3]){1, 0, 0}),
            1.0,
            0, 1, 0.8
        )
    );

    distributeAndMonitor<Monochrome12>(&hyp,input,map,distribution,GammaLut12, 0.02);
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
