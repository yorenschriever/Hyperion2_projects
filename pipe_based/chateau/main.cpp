#include "core/distribution/inputs/inputSlicer.hpp"
#include "core/distribution/inputs/inputSplitter.hpp"
#include "core/distribution/luts/colourCorrectionLut.hpp"
#include "core/distribution/luts/gammaLut.hpp"
#include "core/distribution/luts/incandescentLut.hpp"
#include "core/distribution/luts/laserLut.hpp"
#include "core/distribution/outputs/monitorOutput.hpp"
#include "core/distribution/outputs/neopixelOutput.hpp"
#include "core/hyperion.hpp"
#include "distribution/inputs/patternCycleInput.hpp"
#include "ledParPatterns.hpp"
#include "core/generation/palettes.hpp"
#include "patterns.hpp"
#include <vector>

void addColanderPipe(Hyperion *hyp);
void addLaserPipe(Hyperion *hyp);
void addBulbPipe(Hyperion *hyp);
void addPinspotPipe(Hyperion *hyp);
void addLampionPipe(Hyperion *hyp);
void addLedParPipe(Hyperion *hyp);
void addPaletteColumn(Hyperion *hyp);

LUT *PixelLut = new ColourCorrectionLUT(1.5, 255, 255, 255, 240);
LUT *LaserLut = new LaserLUT(0.5, 4096, 3048);
LUT *IncandescentLut = new IncandescentLUT(2.5, 4096, 200);
LUT *GammaLut12 = new GammaLUT(2.5, 4096);

const char *hyper_l = "hyperslave5.local";
const char *hyper_r = "hyperslave7.local";
const char *hyper_truss = "hyperslave6.local";

int main()
{
  auto hyp = new Hyperion();

  addColanderPipe(hyp);
  addLaserPipe(hyp);
  addBulbPipe(hyp);
  addPinspotPipe(hyp);
  addLampionPipe(hyp);
  addLedParPipe(hyp);
  addPaletteColumn(hyp);

  // hyp->hub.setFlashColumn(7);

  hyp->hub.setColumnName(1, "Palette");
  hyp->hub.setColumnName(1, "Vergieten");
  hyp->hub.setColumnName(2, "Lasers");
  hyp->hub.setColumnName(3, "Bulbs");
  hyp->hub.setColumnName(4, "Pinspots");
  hyp->hub.setColumnName(5, "Lampion");
  hyp->hub.setColumnName(6, "LedPar");

  //Tempo::AddSource(new ConstantTempo(120));

  hyp->hub.buttonPressed(0, 0);
  for (int i=0;i<7;i++)
    hyp->hub.setFlashColumn(i, false, true);
  hyp->hub.setFlashRow(5);
  hyp->hub.setFlashRow(6);
  hyp->hub.setFlashRow(7);
  hyp->hub.setForcedSelection(0);

  hyp->start();

  while (1)
    Thread::sleep(1000);
}

void addColanderPipe(Hyperion *hyp)
{

  auto input = new ControlHubInput<Monochrome>(
      12,
      &hyp->hub,
      {
          {.column = 1, .slot = 0, .pattern = new OnPattern()},
          {.column = 1, .slot = 1, .pattern = new GlowPattern()},
          {.column = 1, .slot = 2, .pattern = new SinPattern()},
          {.column = 1, .slot = 3, .pattern = new BeatSingleFadePattern()},
          {.column = 1, .slot = 4, .pattern = new BeatMultiFadePattern()},

          {.column = 1, .slot = 5, .pattern = new BlinderPattern()},
          {.column = 1, .slot = 6, .pattern = new SlowStrobePattern()},
          {.column = 1, .slot = 7, .pattern = new GlitchPattern()},

          {.column = 1, .slot = 8, .pattern = new FastStrobePattern()},
          {.column = 1, .slot = 9, .pattern = new BeatAllFadePattern()},
          {.column = 1, .slot = 10, .pattern = new BeatShakePattern()},
          {.column = 1, .slot = 11, .pattern = new BeatStepPattern()},
          {.column = 1, .slot = 12, .pattern = new GlowOriginalPattern()},
          {.column = 1, .slot = 13, .pattern = new FastStrobePattern2()},
      });

  const char *hosts[2] = {hyper_l, hyper_r};

  auto splitInput = new InputSlicer(input, {{0, 6, true},
                                            {6, 6, true},
                                            {0, 12, false}});

  for (int i = 0; i < splitInput->size() - 1; i++)
  {
    auto pipe = new ConvertPipe<Monochrome, Monochrome12>(
        splitInput->getInput(i),
        new UDPOutput(hosts[i], 9620, 60),
        IncandescentLut);
    hyp->addPipe(pipe);
  }

}

void addLaserPipe(Hyperion *hyp)
{

  auto splitInput = new InputSplitter(
      new ControlHubInput<Monochrome>(
          12,
          &hyp->hub,
          {
              {.column = 2, .slot = 0, .pattern = new OnPattern(255)},
              {.column = 2, .slot = 1, .pattern = new LFOPattern<PWM>()},
              {.column = 2, .slot = 2, .pattern = new SinPattern()},
              {.column = 2, .slot = 3, .pattern = new BeatMultiFadePattern()},
              {.column = 2, .slot = 4, .pattern = new BeatShakePattern()},

              {.column = 2, .slot = 5, .pattern = new BlinderPattern()},
              {.column = 2, .slot = 6, .pattern = new FastStrobePattern()},
              {.column = 2, .slot = 7, .pattern = new SlowStrobePattern()},

              {.column = 2, .slot = 8, .pattern = new GlowPattern()},
              {.column = 2, .slot = 9, .pattern = new BeatAllFadePattern()},
              {.column = 2, .slot = 10, .pattern = new BeatSingleFadePattern()},
              {.column = 2, .slot = 11, .pattern = new GlitchPattern()},
              {.column = 2, .slot = 12, .pattern = new BeatStepPattern()},
              {.column = 2, .slot = 13, .pattern = new FastStrobePattern2()},
          }),
      {4, 4, 4},
      true);

  const char *hosts[3] = {hyper_l, hyper_r, hyper_truss};

  for (int i = 0; i < splitInput->size(); i++)
  {
    auto pipe = new ConvertPipe<Monochrome, Monochrome12>(
        splitInput->getInput(i),
        new UDPOutput(hosts[i], 9621, 60),
        LaserLut);
    hyp->addPipe(pipe);
  }
}

void addBulbPipe(Hyperion *hyp)
{

  auto splitInput = new InputSplitter(
      new ControlHubInput<Monochrome>(
          6,
          &hyp->hub,
          {
              {.column = 3, .slot = 0, .pattern = new GlowPattern()},
              {.column = 3, .slot = 1, .pattern = new SinPattern()},
              {.column = 3, .slot = 2, .pattern = new LFOPattern<SawDown>()},
              {.column = 3, .slot = 3, .pattern = new BeatShakePattern()},
              {.column = 3, .slot = 4, .pattern = new BeatStepPattern()},

              {.column = 3, .slot = 5, .pattern = new BlinderPattern()},
              {.column = 3, .slot = 6, .pattern = new FastStrobePattern()},
              {.column = 3, .slot = 7, .pattern = new GlitchPattern()},

              {.column = 3, .slot = 8, .pattern = new SlowStrobePattern()},
              {.column = 3, .slot = 9, .pattern = new BeatAllFadePattern()},
              {.column = 3, .slot = 10, .pattern = new BeatSingleFadePattern()},
              {.column = 3, .slot = 11, .pattern = new BeatMultiFadePattern()},
              {.column = 3, .slot = 12, .pattern = new FastStrobePattern2()},

          }),
      {6},
      true);

  for (int i = 0; i < splitInput->size(); i++)
  {
    auto pipe = new ConvertPipe<Monochrome, Monochrome12>(
        splitInput->getInput(i),
        new UDPOutput(hyper_truss, 9620, 60),
        GammaLut12);
    hyp->addPipe(pipe);
  }
}

void addPinspotPipe(Hyperion *hyp)
{

  auto input = new ControlHubInput<Monochrome>(
      4,
      &hyp->hub,
      {
          {.column = 4, .slot = 0, .pattern = new OnPattern(255)},
          {.column = 4, .slot = 1, .pattern = new LFOPattern<SawDown>()},
          {.column = 4, .slot = 2, .pattern = new GlowPattern()},
          {.column = 4, .slot = 3, .pattern = new SinPattern()},
          {.column = 4, .slot = 4, .pattern = new BeatStepPattern()},

          {.column = 4, .slot = 5, .pattern = new BlinderPattern()},
          {.column = 4, .slot = 6, .pattern = new GlitchPattern()},
          {.column = 4, .slot = 7, .pattern = new BeatSingleFadePattern()},

          {.column = 4, .slot = 8, .pattern = new FastStrobePattern()},
          {.column = 4, .slot = 9, .pattern = new SlowStrobePattern()},
          {.column = 4, .slot = 10, .pattern = new BeatAllFadePattern()},
          {.column = 4, .slot = 11, .pattern = new BeatShakePattern()},
          {.column = 4, .slot = 12, .pattern = new BeatMultiFadePattern()},
          {.column = 4, .slot = 13, .pattern = new FastStrobePattern2()},
      });

  auto pipe = new ConvertPipe<Monochrome, Monochrome>(
      input,
      new UDPOutput(hyper_truss, 9619, 60));

  hyp->addPipe(pipe);
}

void addLampionPipe(Hyperion *hyp)
{

  auto input = new ControlHubInput<Monochrome>(
      4,
      &hyp->hub,
      {
          {.column = 5, .slot = 0, .pattern = new OnPattern(255)},
          {.column = 5, .slot = 1, .pattern = new LFOPattern<SawDown>()},
          {.column = 5, .slot = 2, .pattern = new GlowPattern()},
          {.column = 5, .slot = 3, .pattern = new SinPattern()},
          {.column = 5, .slot = 4, .pattern = new BeatStepPattern()},

          {.column = 5, .slot = 5, .pattern = new BlinderPattern()},
          {.column = 5, .slot = 6, .pattern = new GlitchPattern()},
          {.column = 5, .slot = 7, .pattern = new BeatSingleFadePattern()},

          {.column = 5, .slot = 8, .pattern = new FastStrobePattern()},
          {.column = 5, .slot = 9, .pattern = new SlowStrobePattern()},
          {.column = 5, .slot = 10, .pattern = new BeatAllFadePattern()},
          {.column = 5, .slot = 11, .pattern = new BeatShakePattern()},
          {.column = 5, .slot = 12, .pattern = new BeatMultiFadePattern()},
          {.column = 5, .slot = 13, .pattern = new FastStrobePattern2()},
      });

  auto pipe = new ConvertPipe<Monochrome, Monochrome>(
      input,
      new UDPOutput(hyper_r, 9619, 60));

  hyp->addPipe(pipe);
}

void addLedParPipe(Hyperion *hyp)
{

  auto input = new ControlHubInput<RGBA>(
      8,
      &hyp->hub,
      {
          {.column = 6, .slot = 0, .pattern = new Ledpar::OnPattern()},
          {.column = 6, .slot = 1, .pattern = new Ledpar::DuoTonePattern()},
          {.column = 6, .slot = 2, .pattern = new Ledpar::SinPattern()},
          {.column = 6, .slot = 3, .pattern = new Ledpar::LfoPattern<SawDown>()},
          {.column = 6, .slot = 4, .pattern = new Ledpar::PaletteLfoPattern<SawDown>()},

          {.column = 6, .slot = 5, .pattern = new Ledpar::BlinderPattern()},
          {.column = 6, .slot = 6, .pattern = new Ledpar::SlowStrobePattern()},
          {.column = 6, .slot = 7, .pattern = new Ledpar::FastStrobePattern()},
      });

  auto pipe = new ConvertPipe<RGBA, RGBWAmberUV>(
      input,
      new UDPOutput(hyper_l, 9619, 60));
  hyp->addPipe(pipe);

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
          &sunset2,
          &retro,
          &sunset1,
          &sunset3,
          &blueOrange,
          &purpleGreen,
          
          &campfire,
          &sunset8,
          &heatmap2,
          &tunnel,
          &sunset6,
          &sunset7,
          &coralTeal,
          &deepBlueOcean,
          &redSalvation,
          &plumBath,
          &sunset4,
          &candy,
          &greatBarrierReef,
          &peach,
          &denseWater,
          &sunset5,
          &salmonOnIce,
      });
  hyp->hub.subscribe(paletteColumn);
}
