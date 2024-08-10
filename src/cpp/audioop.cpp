#include <algorithm>
#include <cmath>
#include <cstdint>

#include "python3.12/Python.h"

#define PY_SSIZE_T_CLEAN
#define BIAS 0x84 /* define the add-in bias for 16 bit samples */
#define CLIP 32635
#define SIGN_BIT (0x80)  /* Sign bit for an A-law byte. */
#define QUANT_MASK (0xf) /* Quantization field mask. */
#define SEG_SHIFT (4)    /* Left shift for segment number. */
#define SEG_MASK (0x70)  /* Segment field mask. */

constexpr int minvals[] = {0, -0x80, -0x8000, -0x800000, -0x7FFFFFFF - 1};
constexpr int maxvals[] = {0, 0x7F, 0x7FFF, 0x7FFFFF, 0x7FFFFFFF};
constexpr unsigned int masks[] = {0, 0xFF, 0xFFFF, 0xFFFFFF, 0xFFFFFFFF};

static int fbound(double val, double minval, double maxval) {
  val = std::clamp(val, minval, maxval);
  val = std::floor(val);
  return static_cast<int>(val);
}

constexpr int16_t seg_aend[8] = {0x1F,  0x3F,  0x7F,  0xFF,
                                 0x1FF, 0x3FF, 0x7FF, 0xFFF};

constexpr int16_t seg_uend[8] = {0x3F,  0x7F,  0xFF,  0x1FF,
                                 0x3FF, 0x7FF, 0xFFF, 0x1FFF};

static int16_t search(int16_t val, const int16_t *table, int size) {
  int i;

  for (i = 0; i < size; i++) {
    if (val <= *table++) return (i);
  }
  return (size);
}
