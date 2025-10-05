#include "video.hpp"
#include "colours.h"
#include "core/distribution/inputs/patternInput.hpp"
#include "core/distribution/luts/colourCorrectionLut.hpp"
#include "core/distribution/outputs/monitorOutput.hpp"
#include "core/distribution/outputs/neopixelOutput.hpp"
#include "core/distribution/pipes/convertPipe.hpp"
#include "core/generation/patterns/helpers/tempo/constantTempo.h"
#include "core/generation/pixelMap.hpp"
#include "core/hyperion.hpp"
#include "core/generation/palettes.hpp"

#include "mapping/ledsterMap.hpp"
#include "videoPattern.hpp"

void addLedsterPipe(Hyperion *hyp);
void addPaletteColumn(Hyperion *hyp);

int main()
{
  auto hyp = new Hyperion();
  addLedsterPipe(hyp);
  addPaletteColumn(hyp);

  hyp->hub.setFlashColumn(0, false, true);
  hyp->hub.setForcedSelection(0);
  hyp->hub.buttonPressed(0, 0);
  hyp->hub.buttonPressed(1, 0);

  hyp->hub.setColumnName(0,"Palette");
  hyp->hub.setColumnName(1,"Video");

  hyp->start();

  while (1)
    Thread::sleep(1000);
}

void addLedsterPipe(Hyperion *hyp)
{
  auto input = new ControlHubInput<RGBA>(
      ledsterMap.size(),
      &hyp->hub,
      1,
      {
          new BgPattern(),
          new VideoPattern("videos/processed/Kapel_Cirkels_1.bin","Cirkels"),
          new VideoPalettePattern("videos/processed/Kapel_Cirkels_1.bin","Cirkels palette"),
          new VideoPattern("videos/processed/Checkerboard_mask_1.bin","Checkerboard_mask_1.bin"),
          new VideoPattern("videos/processed/Spiraal_mask_1.bin","Spiraal_mask_1"),
      });

  auto ledsterPipe = new ConvertPipe<RGBA, RGB>(
      input,
      new MonitorOutput(&hyp->webServer, &ledsterMap));

  hyp->addPipe(ledsterPipe);
}

void addPaletteColumn(Hyperion *hyp)
{
  auto paletteColumn = new PaletteColumn(
      &hyp->hub,
      0,
      0,
      {
          &campfire,
          &pinkSunset,
          &sunset8,
          &heatmap,
          &heatmap2,
          &sunset2,
          &retro,
          &tunnel,

          &sunset6,
          &sunset7,
          &sunset1,
          &coralTeal,
          &deepBlueOcean,
          &redSalvation,
          &plumBath,
          &sunset4,
          &candy,
          &sunset3,
          &greatBarrierReef,
          &blueOrange,
          &peach,
          &denseWater,
          &purpleGreen,
          &sunset5,
          &salmonOnIce,
      });
  hyp->hub.subscribe(paletteColumn);
}