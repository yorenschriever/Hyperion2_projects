#include "colours.h"
#include "core/distribution/inputs/inputSplitter.hpp"
#include "core/distribution/inputs/patternInput.hpp"
#include "core/distribution/luts/colourCorrectionLut.hpp"
#include "core/distribution/outputs/cloneOutput.hpp"
#include "core/distribution/outputs/monitorOutput.hpp"
#include "core/distribution/outputs/neopixelOutput.hpp"
#include "core/distribution/outputs/udpOutput.hpp"
#include "core/distribution/pipes/convertPipe.hpp"
#include "core/generation/patterns/helpers/tempo/constantTempo.h"
#include "core/generation/pixelMap.hpp"
#include "core/generation/pixelMapSplitter.hpp"
#include "core/hyperion.hpp"
#include "distribution/inputs/controlHubInput.hpp"
#include "generation/controlHub/paletteColumn.hpp"
#include "mapping/wingMap.hpp"
#include "core/generation/palettes.hpp"
#include "patterns.hpp"

#include "../ledster/mapping/ledsterMap.hpp"

void addWingsPipe(Hyperion *hyp);
void addPaletteColumn(Hyperion *hyp);

PixelMap::Polar pWingMap = wingMap.toPolarRotate90();

LUT *PixelLut = new ColourCorrectionLUT(1.5, 255, 255, 255, 240);

int main()
{
  auto hyp = new Hyperion();

  addWingsPipe(hyp);
  addPaletteColumn(hyp);

  hyp->hub.setFlashColumn(7);

  Tempo::AddSource(new ConstantTempo(120));

  hyp->hub.buttonPressed(0, 0);

  hyp->start();

  while (1)
    Thread::sleep(1000);
}

void addWingsPipe(Hyperion *hyp)
{

  auto input = new ControlHubInput<RGBA>(
          wingMap.size(),
          &hyp->hub,
          {
              // static
              {.column = 1, .slot = 0, .pattern = new HorizontalGradientPattern(&wingMap)},
              {.column = 1, .slot = 1, .pattern = new RadialGradientPattern(&pWingMap)},

              // 
              {.column = 2, .slot = 0, .pattern = new ChevronsPattern(&wingMap)},
              {.column = 2, .slot = 1, .pattern = new RadialGlitterFadePattern2(&pWingMap)},
              {.column = 2, .slot = 2, .pattern = new RadialRainbowPattern(&pWingMap)},

              // 
              {.column = 3, .slot = 0, .pattern = new RadialFadePattern(&wingMap)},
              {.column = 3, .slot = 1, .pattern = new GrowingCirclesPattern(&wingMap)},
              {.column = 3, .slot = 2, .pattern = new AngularFadePattern(&pWingMap)},

              // 
              {.column = 4, .slot = 0, .pattern = new RibbenClivePattern<Glow>(10000, 1, 0.15)},
              {.column = 4, .slot = 1, .pattern = new ClivePattern<SawDown>(32, 1000, 1, 0.1)},
              {.column = 4, .slot = 2, .pattern = new ClivePattern<SoftPWM>(32, 1000, 1, 0.5)},

              // 
              {.column = 5, .slot = 0, .pattern = new SegmentChasePattern()},
              {.column = 5, .slot = 1, .pattern = new Lighthouse(&pWingMap)},
              {.column = 5, .slot = 2, .pattern = new XY(&wingMap)},
              {.column = 5, .slot = 3, .pattern = new GlowPulsePattern()},

              // flash
              {.column = 6, .slot = 0, .pattern = new FlashesPattern()},
              {.column = 6, .slot = 1, .pattern = new StrobePattern()},
              {.column = 6, .slot = 2, .pattern = new GrowingStrobePattern(&pWingMap)},
              {.column = 6, .slot = 3, .pattern = new LineLaunch(&wingMap)},
              {.column = 6, .slot = 4, .pattern = new PixelGlitchPattern()},

              // flash
              {.column = 7, .slot = 0, .pattern = new FlashesPattern()},
              {.column = 7, .slot = 1, .pattern = new StrobePattern()},
              {.column = 7, .slot = 2, .pattern = new GrowingStrobePattern(&pWingMap)},
              {.column = 7, .slot = 3, .pattern = new LineLaunch(&wingMap)},
              {.column = 7, .slot = 4, .pattern = new PixelGlitchPattern()},
          });

  int sz = 60 * 8 * sizeof(RGBA);
  auto splitInput = new InputSlicer(
      input,
      {
        {0*sz, sz, true},
        {1*sz, sz, true},
        {2*sz, sz, true},
        {3*sz, sz, true},
        {0, 4*sz, false},
       });

  const char *hosts[4] = {
      "hyperslave1.local",
      "hyperslave1.local",
      "hyperslave2.local",
      "hyperslave2.local"};

  int ports[4] = {
      9611,
      9615,
      9611,
      9615};

  for (int i = 0; i < splitInput->size()-1; i++)
  {
    auto pipe = new ConvertPipe<RGBA, RGB>(
        splitInput->getInput(i),
        new UDPOutput(hosts[i], ports[i], 60));
    hyp->addPipe(pipe);
  }

  hyp->addPipe(new ConvertPipe<RGBA, RGB>(
        splitInput->getInput(4),
        new MonitorOutput(&hyp->webServer,&wingMap)
        ));

}

void addPaletteColumn(Hyperion *hyp)
{
  auto paletteColumn = new PaletteColumn(&hyp->hub, 0,0, {
    &plumBath,
    &deepBlueOcean,
    &heatmap2,
    &salmonOnIce,
    &greatBarrierReef,
    &pinkSunset,
    &campfire,
    &tunnel,
    &heatmap,
    &sunset1,
    &purpleGreen,
    &redSalvation,
    &denseWater,
    });

  hyp->hub.subscribe(paletteColumn);
}


