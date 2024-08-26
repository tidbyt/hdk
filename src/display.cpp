#include "display.h"

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>


#ifdef TIDBYT_GEN2
#define R1 5
#define G1 23
#define BL1 4
#define R2 2
#define G2 22
#define BL2 32

#define CH_A 25
#define CH_B 21
#define CH_C 26
#define CH_D 19
#define CH_E -1  // assign to pin 14 if using more than two panels

#define LAT 18
#define OE 27
#define CLK 15
#else
#define R1 21
#define G1 2
#define BL1 22
#define R2 23
#define G2 4
#define BL2 27

#define CH_A 26
#define CH_B 5
#define CH_C 25
#define CH_D 18
#define CH_E -1  // assign to pin 14 if using more than two panels

#define LAT 19
#define OE 32
#define CLK 33
#endif

static MatrixPanel_I2S_DMA *_matrix;

int display_initialize() {
  // Initialize the panel.
  HUB75_I2S_CFG::i2s_pins pins = {R1,   G1,   BL1,  R2,   G2,  BL2, CH_A,
                                  CH_B, CH_C, CH_D, CH_E, LAT, OE,  CLK};

#ifdef TIDBYT_GEN2
  bool invert_clock_phase = false;
#else
  bool invert_clock_phase = true;
#endif

  HUB75_I2S_CFG mxconfig(64,                      // width
                         32,                      // height
                         1,                       // chain length
                         pins,                    // pin mapping
                         HUB75_I2S_CFG::FM6126A,  // driver chip
                         true,                    // double-buffering
                         HUB75_I2S_CFG::HZ_10M,   // clock speed
                         1,                       // latch blanking
                         invert_clock_phase       // invert clock phase
  );

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
