#include "core/hyperion.hpp"
#include "movingheadPatterns.hpp"
#include "patterns.hpp"
#include "mapping/ledMap.hpp"
#include "ledPatterns.hpp"

#define COL_PALETTE 0
#define COL_MOVINGHEAD 1
#define COL_COLANDER 2
#define COL_FAIRYLIGHT_PINSPOT 3
#define COL_LASERS 4
#define COL_BULBS 5
#define COL_LED1 6
#define COL_LED2 7
#define COL_LEDSTER 8
#define COL_STROBES 9

void addColanderPipe(Hyperion *hyp, Combine *pwmCombine, Combine *dmxCombine);
void addLaserPipe(Hyperion *hyp);
void addBulbPipe(Hyperion *hyp, Combine *dmxCombine);
void addFairylightPinspotPipe(Hyperion *hyp, Combine * pwmCombine);
void addMovingHeadPipe(Hyperion *hyp, Combine *dmxCombine);
void addPaletteColumn(Hyperion *hyp);
void addLed1Column(Hyperion *hyp);
void addLed2Column(Hyperion *hyp);

// LUT *PixelLut = new ColourCorrectionLUT(1.5, 255, 255, 255, 240);
LUT *LaserLut = new LaserLUT(0.5, 4096, 3048);
LUT *IncandescentLut = new IncandescentLUT(2.5, 4096, 200);
LUT *IncandescentLut8 = new IncandescentLUT(2.5, 255, 24);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);
LUT *GammaLut8 = new GammaLUT(2.5, 255);
LUT *ledLut = new ColourCorrectionLUT(2.7, 255, 255, 255, 255);

int main()
{
    auto hyp = new Hyperion();

    auto pwmCombine = new Combine();
    auto dmxCombine = new Combine();

    addColanderPipe(hyp, pwmCombine, dmxCombine);
    addLaserPipe(hyp);
    addBulbPipe(hyp, dmxCombine);
    addFairylightPinspotPipe(hyp, pwmCombine);
    addMovingHeadPipe(hyp, dmxCombine);
    addLed1Column(hyp);
    addLed2Column(hyp);

    hyp->createChain(pwmCombine,new PWMOutput());
    hyp->createChain(dmxCombine,new DMXOutput());

    addPaletteColumn(hyp);

    hyp->hub.setColumnName(COL_PALETTE, "Kleur");
    hyp->hub.setColumnName(COL_MOVINGHEAD, "Moving head");
    hyp->hub.setColumnName(COL_COLANDER, "Vergiet");
    hyp->hub.setColumnName(COL_FAIRYLIGHT_PINSPOT, "Fairylight / spiegelbol");
    hyp->hub.setColumnName(COL_LASERS, "Lasers");
    hyp->hub.setColumnName(COL_BULBS, "Peertjes");
    hyp->hub.setColumnName(COL_LEDSTER, "Ledster");
    hyp->hub.setColumnName(COL_STROBES, "Strobes");
    hyp->hub.setColumnName(COL_LED1, "LED 1");
    hyp->hub.setColumnName(COL_LED2, "LED 2");

    //   Tempo::AddSource(new ConstantTempo(120));

    for (int i = 1; i < 10; i++)
        hyp->hub.setFlashColumn(i, false, true);
    hyp->hub.setFlashRow(5);
    hyp->hub.setFlashRow(6);
    hyp->hub.setFlashRow(7);

    hyp->hub.buttonPressed(COL_PALETTE, 0);
    hyp->hub.setForcedSelection(COL_PALETTE);
    hyp->hub.setFlashColumn(COL_PALETTE, false, true);

    hyp->start();

    while (1)
        Thread::sleep(1000);
}

void addColanderPipe(Hyperion *hyp, Combine *pwmCombine, Combine *dmxCombine)
{
    const int numColander = 10;
    auto input = new ControlHubInput<Monochrome>(
        numColander,
        &hyp->hub,
        {
            {.column = COL_COLANDER, .slot = 0, .pattern = new OnPattern()},
            {.column = COL_COLANDER, .slot = 1, .pattern = new GlowOriginalPattern()},
            {.column = COL_COLANDER, .slot = 2, .pattern = new SinPattern()},
            {.column = COL_COLANDER, .slot = 3, .pattern = new BeatSingleFadePattern()},
            {.column = COL_COLANDER, .slot = 4, .pattern = new BeatMultiFadePattern()},

            {.column = COL_COLANDER, .slot = 5, .pattern = new BlinderPattern()},
            {.column = COL_COLANDER, .slot = 6, .pattern = new SlowStrobePattern()},
            {.column = COL_COLANDER, .slot = 7, .pattern = new GlitchPattern()},

            {.column = COL_COLANDER, .slot = 8, .pattern = new FastStrobePattern()},
            {.column = COL_COLANDER, .slot = 9, .pattern = new BeatAllFadePattern()},
            {.column = COL_COLANDER, .slot = 10, .pattern = new BeatShakePattern()},
            {.column = COL_COLANDER, .slot = 11, .pattern = new BeatStepPattern()},
            {.column = COL_COLANDER, .slot = 12, .pattern = new GlowPattern()},
            {.column = COL_COLANDER, .slot = 13, .pattern = new FastStrobePattern2()},
        });

    auto splitInput = new Slicer(
        {
            {0, numColander, true},
            {0, numColander, true},
            {0, numColander, true},
        });

    hyp->createChain(input,splitInput);

    hyp->createChain(
        splitInput->getSlice(0),
        new ConvertColor<Monochrome, Monochrome>(IncandescentLut8),
        dmxCombine->atDmxChannel(128)
    );

    hyp->createChain(
        splitInput->getSlice(1),
        new ConvertColor<Monochrome, Monochrome12>(IncandescentLut),
        pwmCombine->atOffset(0));

    hyp->createChain(
        splitInput->getSlice(2),
        new ConvertColor<Monochrome, Monochrome12>(IncandescentLut),
        new UDPOutput("hyperslave6.local", 9620, 60));

}

void addLaserPipe(Hyperion *hyp)
{
    auto input = new ControlHubInput<Monochrome>(
        12,
        &hyp->hub,
        {
            {.column = COL_LASERS, .slot = 0, .pattern = new OnPattern(255)},
            {.column = COL_LASERS, .slot = 1, .pattern = new LFOPattern<PWM>()},
            {.column = COL_LASERS, .slot = 2, .pattern = new SinPattern()},
            {.column = COL_LASERS, .slot = 3, .pattern = new BeatMultiFadePattern()},
            {.column = COL_LASERS, .slot = 4, .pattern = new BeatShakePattern()},

            {.column = COL_LASERS, .slot = 5, .pattern = new BlinderPattern()},
            {.column = COL_LASERS, .slot = 6, .pattern = new FastStrobePattern()},
            {.column = COL_LASERS, .slot = 7, .pattern = new SlowStrobePattern()},

            {.column = COL_LASERS, .slot = 8, .pattern = new GlowPattern()},
            {.column = COL_LASERS, .slot = 9, .pattern = new BeatAllFadePattern()},
            {.column = COL_LASERS, .slot = 10, .pattern = new BeatSingleFadePattern()},
            {.column = COL_LASERS, .slot = 11, .pattern = new GlitchPattern()},
            {.column = COL_LASERS, .slot = 12, .pattern = new BeatStepPattern()},
            {.column = COL_LASERS, .slot = 13, .pattern = new FastStrobePattern2()},
    });

    hyp->createChain(
        input,
        new ConvertColor<Monochrome, Monochrome12>(LaserLut),
        new UDPOutput("hyperslave7.local", 9620)
    );
}

void addBulbPipe(Hyperion *hyp, Combine *dmxCombine)
{
  const int numBulbs = 10;
  auto input = new ControlHubInput<Monochrome>(
      numBulbs,
      &hyp->hub,
      {
          {.column = COL_BULBS, .slot = 0, .pattern = new GlowPattern()},
          {.column = COL_BULBS, .slot = 1, .pattern = new SinPattern()},
          {.column = COL_BULBS, .slot = 2, .pattern = new LFOPattern<SawDown>()},
          {.column = COL_BULBS, .slot = 3, .pattern = new BeatShakePattern()},
          {.column = COL_BULBS, .slot = 4, .pattern = new BeatStepPattern()},

          {.column = COL_BULBS, .slot = 5, .pattern = new BlinderPattern()},
          {.column = COL_BULBS, .slot = 6, .pattern = new FastStrobePattern()},
          {.column = COL_BULBS, .slot = 7, .pattern = new GlitchPattern()},

          {.column = COL_BULBS, .slot = 8, .pattern = new SlowStrobePattern()},
          {.column = COL_BULBS, .slot = 9, .pattern = new BeatAllFadePattern()},
          {.column = COL_BULBS, .slot = 10, .pattern = new BeatSingleFadePattern()},
          {.column = COL_BULBS, .slot = 11, .pattern = new BeatMultiFadePattern()},
          {.column = COL_BULBS, .slot = 12, .pattern = new FastStrobePattern2()},
      });

  auto splitInput = new Slicer(
      {
          {0, numBulbs, true},
          {0, numBulbs, true},
      });

    hyp->createChain(input,splitInput);

    hyp->createChain(
        splitInput->getSlice(0),
        new ConvertColor<Monochrome, Monochrome>(GammaLut8),
        dmxCombine->atDmxChannel(256)
    );

    hyp->createChain(
        splitInput->getSlice(1),
        new ConvertColor<Monochrome, Monochrome12>(GammaLut12),
        new UDPOutput("hyperslave5.local", 9620)
    );
}

void addFairylightPinspotPipe(Hyperion *hyp, Combine *pwmCombine)
{
    auto pinspotInput = new ControlHubInput<Monochrome>(
              1,
              &hyp->hub,
              {
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 0, .pattern = new OnPattern(255)},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 1, .pattern = new SinPattern()},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 2, .pattern = new BeatAllFadePattern()},
              });
    hyp->createChain(
        pinspotInput,
        new ConvertColor<Monochrome, Monochrome12>(IncandescentLut),
        pwmCombine->atOffset(11)
    );

    auto fairylightInput = new ControlHubInput<Monochrome>(
              1,
              &hyp->hub,
              {
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 3, .pattern = new OnPattern(255)},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 4, .pattern = new SinPattern()},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 5, .pattern = new BeatAllFadePattern()},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 6, .pattern = new BlinderPattern()},
                  {.column = COL_FAIRYLIGHT_PINSPOT, .slot = 7, .pattern = new FastStrobePattern()},
              });
    hyp->createChain(
        fairylightInput,
        new ConvertColor<Monochrome, Monochrome12>(GammaLut12),
        pwmCombine->atOffset(12)
    );
}

void addMovingHeadPipe(Hyperion *hyp, Combine *dmxCombine)
{
    auto input = new ControlHubInput<MovingHead>(
        3,
        &hyp->hub,
        {
            {.column = COL_MOVINGHEAD, .slot = 0, .pattern = new MovingheadPatterns::WallPattern()},
            {.column = COL_MOVINGHEAD, .slot = 1, .pattern = new MovingheadPatterns::WallDJPattern()},
            {.column = COL_MOVINGHEAD, .slot = 2, .pattern = new MovingheadPatterns::SidesPattern()},
            {.column = COL_MOVINGHEAD, .slot = 3, .pattern = new MovingheadPatterns::Flash360Pattern()},
            {.column = COL_MOVINGHEAD, .slot = 4, .pattern = new MovingheadPatterns::GlitchPattern()},
            {.column = COL_MOVINGHEAD, .slot = 5, .pattern = new MovingheadPatterns::FrontPattern()},
            {.column = COL_MOVINGHEAD, .slot = 6, .pattern = new MovingheadPatterns::UVPattern()},
            {.column = COL_MOVINGHEAD, .slot = 7, .pattern = new MovingheadPatterns::GlitchPattern()},
        });

    hyp->createChain(
        input,
        new ConvertColor<MovingHead, Miniwash7>(),
        dmxCombine->atDmxChannel(30)
    );
}

void addLed1Column(Hyperion *hyp)
{
    int nleds = 4*8*60;
    int nbytes = nleds * sizeof(RGBA);
    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::CeilingChase()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::Fireworks()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::Chaser()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SegmentChasePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::FlashesPattern()},

            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::GlitchPattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::FadingNoisePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::StrobeHighlightPattern()},

            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::GlowPulsePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::BarLFO()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::GradientLFO()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::FadeFromRandom()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::RibbenClivePattern<NegativeCosFast>()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::RibbenFlashPattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::PixelGlitchPattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SegmentGlitchPattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::StrobePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SinChasePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SawChasePattern()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::FadeFromCenter()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SideWave()},
            {.column = COL_LED1, .slot = i++, .pattern = new Patterns::SinChase2Pattern()},
        });

#if (ESP_PLATFORM)
    auto split = new Splitter({
        nbytes/4,
        nbytes/4,
        nbytes/4,
        nbytes/4,
    });

    hyp->createChain(input,split);

    for (int i=0;i<split->size();i++)
        hyp->createChain(
            split->getSlice(i),
            new ConvertColor<RGBA,GRB>(ledLut),
            new NeopixelOutput(i+1)
        );
#else

    hyp->createChain(
        input,
        new ConvertColor<RGBA, RGB>(),
        new MonitorOutput(&hyp->webServer, &ledMap1)
    );
#endif
}

void addLed2Column(Hyperion *hyp)
{
    int nleds = 4*8*60;
    int nbytes = nleds * sizeof(RGBA);
    int i=0;

    auto input = new ControlHubInput<RGBA>(
        nleds,
        &hyp->hub,
        {
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::GlowPulsePattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::GradientLFO()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::FadeFromRandom()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SinChasePattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SawChasePattern()},

            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::StrobePattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::GlitchPattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::PixelGlitchPattern()},

            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SegmentChasePattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::BarLFO()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::RibbenClivePattern<NegativeCosFast>()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::RibbenFlashPattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::FadingNoisePattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SegmentGlitchPattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::FlashesPattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::StrobeHighlightPattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::FadeFromCenter()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SideWave()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::SinChase2Pattern()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::CeilingChase()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::Fireworks()},
            {.column = COL_LED2, .slot = i++, .pattern = new Patterns::Chaser()},

        });

#if (ESP_PLATFORM)
    auto split = new Splitter({
        nbytes/4,
        nbytes/4,
        nbytes/4,
        nbytes/4,
    });

    hyp->createChain(input,split);

    for (int i=0;i<split->size();i++)
        hyp->createChain(
            split->getSlice(i),
            new ConvertColor<RGBA,BGR>(ledLut),
            new NeopixelOutput(i+5)
        );
#else
    hyp->createChain(
        input,
        new ConvertColor<RGBA, RGB>(),
        new MonitorOutput(&hyp->webServer, &ledMap2)
    );
#endif
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
