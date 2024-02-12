#include "display.h"

#include "../lib/HUB75/ESP32-HUB75-MatrixPanel-DMA/src/ESP32-HUB75-MatrixPanel-I2S-DMA.h"

#define R1 4
#define G1 5
#define BL1 6
#define R2 7
#define G2 15
#define BL2 16

#define CH_A 18
#define CH_B 8
#define CH_C 3
#define CH_D 42
#define CH_E -1  // assign to pin 14 if using more than two panels

#define LAT 40
#define OE 2
#define CLK 41

static MatrixPanel_I2S_DMA *_matrix;

int display_initialize() {
  // Initialize the panel.
  HUB75_I2S_CFG::i2s_pins pins = {R1,   G1,   BL1,  R2,   G2,  BL2, CH_A,
                                  CH_B, CH_C, CH_D, CH_E, LAT, OE,  CLK};
  HUB75_I2S_CFG mxconfig(64,                      // width
                         32,                      // height
                         1,                       // chain length
                         pins,                    // pin mapping
                         HUB75_I2S_CFG::FM6126A,  // driver chip
                         true,                    // double-buffering
                         HUB75_I2S_CFG::HZ_10M);
  _matrix = new MatrixPanel_I2S_DMA(mxconfig);

  // Set brightness and clear the screen.
  _matrix->setBrightness8(DISPLAY_DEFAULT_BRIGHTNESS);
  if (!_matrix->begin()) {
    return 1;
  }
  _matrix->fillScreenRGB888(0, 0, 0);

  return 0;
}

void display_shutdown() {
  display_clear();
  _matrix->stopDMAoutput();
}

void display_draw(const uint8_t *pix, int width, int height,
		  int channels, int ixR, int ixG, int ixB) {
  for (unsigned int i = 0; i < height; i++) {
    for (unsigned int j = 0; j < width; j++) {
      const uint8_t *p = &pix[(i * width + j) * channels];
      uint8_t r = p[ixR];
      uint8_t g = p[ixG];
      uint8_t b = p[ixB];

      _matrix->drawPixelRGB888(j, i, r, g, b);
    }
  }
  _matrix->flipDMABuffer();
}

void display_clear() { _matrix->fillScreenRGB888(0, 0, 0); }
