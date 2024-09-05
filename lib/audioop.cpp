#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <new>

namespace py = pybind11;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

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

#define st_ulaw2linear16(uc) (_st_ulaw2linear16[uc])
#define st_alaw2linear16(uc) (_st_alaw2linear16[uc])

constexpr int16_t _st_ulaw2linear16[256] = {
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956, -23932,
    -22908, -21884, -20860, -19836, -18812, -17788, -16764, -15996, -15484,
    -14972, -14460, -13948, -13436, -12924, -12412, -11900, -11388, -10876,
    -10364, -9852,  -9340,  -8828,  -8316,  -7932,  -7676,  -7420,  -7164,
    -6908,  -6652,  -6396,  -6140,  -5884,  -5628,  -5372,  -5116,  -4860,
    -4604,  -4348,  -4092,  -3900,  -3772,  -3644,  -3516,  -3388,  -3260,
    -3132,  -3004,  -2876,  -2748,  -2620,  -2492,  -2364,  -2236,  -2108,
    -1980,  -1884,  -1820,  -1756,  -1692,  -1628,  -1564,  -1500,  -1436,
    -1372,  -1308,  -1244,  -1180,  -1116,  -1052,  -988,   -924,   -876,
    -844,   -812,   -780,   -748,   -716,   -684,   -652,   -620,   -588,
    -556,   -524,   -492,   -460,   -428,   -396,   -372,   -356,   -340,
    -324,   -308,   -292,   -276,   -260,   -244,   -228,   -212,   -196,
    -180,   -164,   -148,   -132,   -120,   -112,   -104,   -96,    -88,
    -80,    -72,    -64,    -56,    -48,    -40,    -32,    -24,    -16,
    -8,     0,      32124,  31100,  30076,  29052,  28028,  27004,  25980,
    24956,  23932,  22908,  21884,  20860,  19836,  18812,  17788,  16764,
    15996,  15484,  14972,  14460,  13948,  13436,  12924,  12412,  11900,
    11388,  10876,  10364,  9852,   9340,   8828,   8316,   7932,   7676,
    7420,   7164,   6908,   6652,   6396,   6140,   5884,   5628,   5372,
    5116,   4860,   4604,   4348,   4092,   3900,   3772,   3644,   3516,
    3388,   3260,   3132,   3004,   2876,   2748,   2620,   2492,   2364,
    2236,   2108,   1980,   1884,   1820,   1756,   1692,   1628,   1564,
    1500,   1436,   1372,   1308,   1244,   1180,   1116,   1052,   988,
    924,    876,    844,    812,    780,    748,    716,    684,    652,
    620,    588,    556,    524,    492,    460,    428,    396,    372,
    356,    340,    324,    308,    292,    276,    260,    244,    228,
    212,    196,    180,    164,    148,    132,    120,    112,    104,
    96,     88,     80,     72,     64,     56,     48,     40,     32,
    24,     16,     8,      0};

static unsigned char st_14linear2ulaw(int16_t pcm_val) {
  int16_t mask;
  int16_t seg;
  unsigned char uval;

  mask = (pcm_val < 0) ? (pcm_val = -pcm_val, 0x7F) : 0xFF;
  pcm_val = (pcm_val >= CLIP) ? CLIP : pcm_val;
  pcm_val += (BIAS >> 2);
  seg = search(pcm_val, seg_uend, 8);

  if (seg >= 8) {
    return (unsigned char)(0x7F ^ mask);
  } else {
    uval = (unsigned char)(seg << 4) | ((pcm_val >> (seg + 1)) & 0xF);
    return (uval ^ mask);
  }
}

constexpr int16_t _st_alaw2linear16[256] = {
    -5504,  -5248,  -6016,  -5760,  -4480,  -4224,  -4992,  -4736,  -7552,
    -7296,  -8064,  -7808,  -6528,  -6272,  -7040,  -6784,  -2752,  -2624,
    -3008,  -2880,  -2240,  -2112,  -2496,  -2368,  -3776,  -3648,  -4032,
    -3904,  -3264,  -3136,  -3520,  -3392,  -22016, -20992, -24064, -23040,
    -17920, -16896, -19968, -18944, -30208, -29184, -32256, -31232, -26112,
    -25088, -28160, -27136, -11008, -10496, -12032, -11520, -8960,  -8448,
    -9984,  -9472,  -15104, -14592, -16128, -15616, -13056, -12544, -14080,
    -13568, -344,   -328,   -376,   -360,   -280,   -264,   -312,   -296,
    -472,   -456,   -504,   -488,   -408,   -392,   -440,   -424,   -88,
    -72,    -120,   -104,   -24,    -8,     -56,    -40,    -216,   -200,
    -248,   -232,   -152,   -136,   -184,   -168,   -1376,  -1312,  -1504,
    -1440,  -1120,  -1056,  -1248,  -1184,  -1888,  -1824,  -2016,  -1952,
    -1632,  -1568,  -1760,  -1696,  -688,   -656,   -752,   -720,   -560,
    -528,   -624,   -592,   -944,   -912,   -1008,  -976,   -816,   -784,
    -880,   -848,   5504,   5248,   6016,   5760,   4480,   4224,   4992,
    4736,   7552,   7296,   8064,   7808,   6528,   6272,   7040,   6784,
    2752,   2624,   3008,   2880,   2240,   2112,   2496,   2368,   3776,
    3648,   4032,   3904,   3264,   3136,   3520,   3392,   22016,  20992,
    24064,  23040,  17920,  16896,  19968,  18944,  30208,  29184,  32256,
    31232,  26112,  25088,  28160,  27136,  11008,  10496,  12032,  11520,
    8960,   8448,   9984,   9472,   15104,  14592,  16128,  15616,  13056,
    12544,  14080,  13568,  344,    328,    376,    360,    280,    264,
    312,    296,    472,    456,    504,    488,    408,    392,    440,
    424,    88,     72,     120,    104,    24,     8,      56,     40,
    216,    200,    248,    232,    152,    136,    184,    168,    1376,
    1312,   1504,   1440,   1120,   1056,   1248,   1184,   1888,   1824,
    2016,   1952,   1632,   1568,   1760,   1696,   688,    656,    752,
    720,    560,    528,    624,    592,    944,    912,    1008,   976,
    816,    784,    880,    848};

static unsigned char st_linear2alaw(int16_t pcm_val) {
  int16_t mask;
  int16_t seg;
  unsigned char aval;

  mask = (pcm_val < 0) ? (pcm_val = -pcm_val - 1, 0x55) : 0xD5;
  seg = search(pcm_val, seg_aend, 8);

  if (seg >= 8) {
    return (unsigned char)(0x7F ^ mask);
  } else {
    aval = (unsigned char)seg << SEG_SHIFT;
    if (seg < 2)
      aval |= (pcm_val >> 1) & QUANT_MASK;
    else
      aval |= (pcm_val >> seg) & QUANT_MASK;
    return (aval ^ mask);
  }
}

constexpr int indexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8,
};

constexpr int stepsizeTable[89] = {
    7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
    19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
    130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
    337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
    876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
    2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
    5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767};

#define GETINTX(T, cp, i) (*reinterpret_cast<T *>((unsigned char *)(cp) + (i)))
#define SETINTX(T, cp, i, val)                                                 \
  do {                                                                         \
    *reinterpret_cast<T *>((unsigned char *)(cp) + (i)) = static_cast<T>(val); \
  } while (0)

#define GETINT8(cp, i) GETINTX(signed char, (cp), (i))
#define GETINT16(cp, i) GETINTX(int16_t, (cp), (i))
#define GETINT32(cp, i) GETINTX(int32_t, (cp), (i))

#ifdef WORDS_BIGENDIAN
#define GETINT24(cp, i)                      \
  (((unsigned char *)(cp) + (i))[2] +        \
   (((unsigned char *)(cp) + (i))[1] << 8) + \
   (((signed char *)(cp) + (i))[0] << 16))
#else
#define GETINT24(cp, i)                      \
  (((unsigned char *)(cp) + (i))[0] +        \
   (((unsigned char *)(cp) + (i))[1] << 8) + \
   (((signed char *)(cp) + (i))[2] << 16))
#endif

#define SETINT8(cp, i, val) SETINTX(signed char, (cp), (i), (val))
#define SETINT16(cp, i, val) SETINTX(int16_t, (cp), (i), (val))
#define SETINT32(cp, i, val) SETINTX(int32_t, (cp), (i), (val))

#ifdef WORDS_BIGENDIAN
#define SETINT24(cp, i, val)                                       \
  do {                                                             \
    ((unsigned char *)(cp) + (i))[2] = static_cast<int>(val);      \
    ((unsigned char *)(cp) + (i))[1] = static_cast<int>(val) >> 8; \
    ((signed char *)(cp) + (i))[0] = static_cast<int>(val) >> 16;  \
  } while (0)
#else
#define SETINT24(cp, i, val)                                       \
  do {                                                             \
    ((unsigned char *)(cp) + (i))[0] = static_cast<int>(val);      \
    ((unsigned char *)(cp) + (i))[1] = static_cast<int>(val) >> 8; \
    ((signed char *)(cp) + (i))[2] = static_cast<int>(val) >> 16;  \
  } while (0)
#endif

#define GETRAWSAMPLE(size, cp, i)                       \
  ((size == 1)   ? static_cast<int> GETINT8((cp), (i))  \
   : (size == 2) ? static_cast<int> GETINT16((cp), (i)) \
   : (size == 3) ? static_cast<int> GETINT24((cp), (i)) \
                 : static_cast<int>                     \
                       GETINT32((cp), (i)))

#define SETRAWSAMPLE(size, cp, i, val) \
  do {                                 \
    if (size == 1)                     \
      SETINT8((cp), (i), (val));       \
    else if (size == 2)                \
      SETINT16((cp), (i), (val));      \
    else if (size == 3)                \
      SETINT24((cp), (i), (val));      \
    else                               \
      SETINT32((cp), (i), (val));      \
  } while (0)

#define GETSAMPLE32(size, cp, i)                              \
  ((size == 1)   ? static_cast<int> GETINT8((cp), (i)) << 24  \
   : (size == 2) ? static_cast<int> GETINT16((cp), (i)) << 16 \
   : (size == 3) ? static_cast<int> GETINT24((cp), (i)) << 8  \
                 : static_cast<int>                           \
                       GETINT32((cp), (i)))

#define SETSAMPLE32(size, cp, i, val)   \
  do {                                  \
    if (size == 1)                      \
      SETINT8((cp), (i), (val) >> 24);  \
    else if (size == 2)                 \
      SETINT16((cp), (i), (val) >> 16); \
    else if (size == 3)                 \
      SETINT24((cp), (i), (val) >> 8);  \
    else                                \
      SETINT32((cp), (i), (val));       \
  } while (0)

static void audioop_check_size(int size) {
  if (size < 1 || size > 4)
    throw py::value_error("Size should be 1, 2, 3 or 4");
}

static void audioop_check_parameters(int len, int size) {
  audioop_check_size(size);
  if (len % size != 0) throw py::value_error("Not a whole number of frames");
}

static int audioop_getsample_impl(py::buffer *fragment, int width, int index) {
  int val;
  py::buffer_info buf = fragment->request();
  audioop_check_parameters(buf.size, width);

  if (index < 0 || index >= buf.size) {
    throw py::index_error("Index out of range");
    return 0;
  }
  val = GETRAWSAMPLE(width, buf.ptr, index * width);
  return val;
}

static py::ssize_t audioop_max_impl(py::buffer *fragment, int width) {
  py::buffer_info buf = fragment->request();
  py::ssize_t i;
  unsigned int absval, max = 0;

  audioop_check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    int val = GETRAWSAMPLE(width, buf.ptr, i);
    absval = (val < 0) ? (unsigned int)-(int64_t)val : val;
    max = (absval > max) ? absval : max;
  }
  return max;
}

static py::tuple audioop_minmax_impl(py::buffer *fragment, int width) {
  int min = 0x7fffffff, max = -0x7FFFFFFF - 1;
  py::size_t i;
  py::tuple result(2);
  py::buffer_info buf = fragment->request();

  audioop_check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    int val = GETRAWSAMPLE(width, buf.ptr, i);
    if (val > max) max = val;
    if (val < min) min = val;
  }

  result[0] = min;
  result[1] = max;

  return result;
}

static int audioop_avg_impl(py::buffer *fragment, int width) {
  int avg;
  double sum = 0.0;

  py::size_t i;
  py::buffer_info buf = fragment->request();

  audioop_check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    sum += GETRAWSAMPLE(width, buf.ptr, i);
  }

  if (buf.size == 0) {
    avg = 0;
  } else {
    double num_frames = static_cast<double>(buf.size) / width;
    avg = static_cast<int>(floor(sum / num_frames));
  }

  return avg;
}

static unsigned int audioop_rms_impl(py::buffer *fragment, int width) {
  py::buffer_info buf = fragment->request();
  py::ssize_t i;

  unsigned int rms_val;
  double sum_sqre = 0.0;

  audioop_check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    double val = GETRAWSAMPLE(width, buf.ptr, i);
    sum_sqre += val * val;
  }

  if (buf.size == 0) {
    rms_val = 0;
  } else {
    double num_frames = static_cast<double>(buf.size) / width;
    rms_val = static_cast<unsigned int>(std::sqrt(sum_sqre / num_frames));
  }

  return rms_val;
}

static double _sum2(const int16_t *a, const int16_t *b, py::size_t len) {
  py::size_t i;
  double sum = 0.0;

  for (i = 0; i < len; i++) {
    sum += static_cast<double>(a[i]) * static_cast<double>(b[i]);
  }
  return sum;
}

static py::tuple audioop_findfit_impl(py::buffer *fragment,
                                      py::buffer *reference) {
  py::tuple output(2);
  py::buffer_info frag = fragment->request();
  py::buffer_info ref = reference->request();

  if (frag.size & 1 || ref.size & 1)
    throw py::value_error("Strings should be even-sized.");

  py::size_t len1, len2;

  const int16_t *cp1 = static_cast<const int16_t *>(frag.ptr);
  const int16_t *cp2 = static_cast<const int16_t *>(ref.ptr);
  len1 = frag.size >> 1;
  len2 = ref.size >> 1;

  if (len1 < len2) throw py::index_error("First sample should be longer.");

  double aj_m1, aj_lm1;
  double sum_ri_2, sum_aij_2, sum_aij_ri, result, best_result, factor;
  py::size_t j, best_j;

  sum_ri_2 = _sum2(cp2, cp2, len2);
  sum_aij_2 = _sum2(cp1, cp1, len2);
  sum_aij_ri = _sum2(cp1, cp2, len2);

  result = (sum_ri_2 * sum_aij_2 - sum_aij_ri * sum_aij_ri) / sum_aij_2;
  best_result = result;
  best_j = 0;

  for (j = 1; j <= len1 - len2; j++) {
    aj_m1 = static_cast<double>(cp1[j - 1]);
    aj_lm1 = static_cast<double>(cp1[j + len2 - 1]);

    sum_aij_2 = sum_aij_2 + aj_lm1 * aj_lm1 - aj_m1 * aj_m1;
    sum_aij_ri = _sum2(cp1 + j, cp2, len2);

    result = (sum_ri_2 * sum_aij_2 - sum_aij_ri * sum_aij_ri) / sum_aij_2;

    if (result < best_result) {
      best_result = result;
      best_j = j;
    }
  }

  factor = _sum2(cp1 + best_j, cp2, len2) / sum_ri_2;

  output[0] = best_j;
  output[1] = factor;
  return output;
}

static double audioop_findfactor_impl(py::buffer *fragment,
                                      py::buffer *reference) {
  py::buffer_info frag = fragment->request();
  py::buffer_info ref = reference->request();

  double sum_ri_2, sum_aij_ri, result;

  if (frag.size & 1 || ref.size & 1)
    throw py::value_error("Strings should be even-sized.");
  if (frag.size != ref.size)
    throw py::value_error("Samples should be of same size");

  const int16_t *cp1 = static_cast<const int16_t *>(frag.ptr);
  const int16_t *cp2 = static_cast<const int16_t *>(ref.ptr);
  py::size_t len = frag.size >> 1;

  sum_ri_2 = _sum2(cp2, cp2, len);
  sum_aij_ri = _sum2(cp1, cp2, len);
  result = sum_aij_ri / sum_ri_2;

  return result;
}

static py::size_t audioop_findmax_impl(py::buffer *fragment,
                                       py::size_t length) {
  py::buffer_info frag = fragment->request();
  if (frag.size & 1) throw py::value_error("Strings should be even-sized");

  const int16_t *cp1 = static_cast<const int16_t *>(frag.ptr);
  py::size_t len1 = frag.size >> 1;

  if (length < 0 || len1 < length)
    throw py::index_error("Input sample should be longer");

  py::size_t j, best_j;
  double aj_m1, aj_lm1;
  double result, best_result;

  result = _sum2(cp1, cp1, length);
  best_j = 0;

  for (j = 1; j <= len1 - length; j++) {
    aj_m1 = static_cast<double>(cp1[j - 1]);
    aj_lm1 = static_cast<double>(cp1[j + length - 1]);

    result += aj_lm1 * aj_lm1 - aj_m1 * aj_m1;
    if (result > best_result) {
      best_result = result;
      best_j = j;
    }
  }
  return best_j;
}

static unsigned int audioop_avgpp_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::size_t i;
  int prev_val, prev_extreme_valid = 0, prev_extreme = 0;
  double sum = 0.0;
  unsigned int avg;
  int diff, prev_diff, next_extreme = 0;

  audioop_check_parameters(frag.size, width);
  if (frag.size <= width) return 0;

  prev_val = GETRAWSAMPLE(width, frag.ptr, 0);
  prev_diff = 42;

  for (i = width; i < frag.size; i += width) {
    int val = GETRAWSAMPLE(width, frag.ptr, i);
    if (val != prev_val) {
      diff = val < prev_val;
      if (prev_diff == !diff) {
        if (prev_extreme_valid) {
          if (prev_val < prev_extreme)
            sum += static_cast<double>(static_cast<unsigned int>(prev_extreme) -
                                       static_cast<unsigned int>(prev_val));
          else
            sum += static_cast<double>(static_cast<unsigned int>(prev_val) -
                                       static_cast<unsigned int>(prev_extreme));
          next_extreme++;
        }
        prev_extreme_valid = 1;
        prev_extreme = prev_val;
      }
      prev_val = val;
      prev_diff = diff;
    }
  }
  if (next_extreme == 0)
    avg = 0;
  else
    avg = static_cast<unsigned int>(sum / static_cast<double>(next_extreme));

  return avg;
}

static unsigned int audioop_maxpp_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  audioop_check_parameters(frag.size, width);
  if (frag.size <= width) return 0;

  py::size_t i;
  int prev_val, prev_extreme_valid = 0, prev_extreme = 0;
  unsigned int max = 0, extreme_diff;
  int diff, prev_diff;

  prev_val = GETRAWSAMPLE(width, frag.ptr, 0);
  prev_diff = 42;

  for (i = 0; i < frag.size; i += width) {
    int val = GETRAWSAMPLE(width, frag.ptr, i);
    if (val != prev_val) {
      diff = val < prev_val;
      if (prev_diff != diff) {
        if (prev_extreme_valid) {
          if (prev_val < prev_extreme)
            extreme_diff = static_cast<unsigned int>(prev_extreme) -
                           static_cast<unsigned int>(prev_val);
          else
            extreme_diff = static_cast<unsigned int>(prev_val) -
                           static_cast<unsigned int>(prev_extreme);
          if (extreme_diff > max) max = extreme_diff;
        }
        prev_extreme_valid = 1;
        prev_extreme = prev_val;
      }
      prev_val = val;
      prev_diff = diff;
    }
  }
  return max;
}

static py::size_t audioop_cross_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  audioop_check_parameters(frag.size, width);

  py::size_t i, ncross = -1;
  int prev_val = 42;

  for (i = 0; i < frag.size; i += width) {
    int val = GETRAWSAMPLE(width, frag.ptr, i) < 0;
    if (val != prev_val) ncross++;
    prev_val = val;
  }
  return ncross;
}

// Function generating wrong values
static py::bytes audioop_mul_impl(py::buffer *fragment, int width,
                                  double factor) {
  py::buffer_info frag = fragment->request();
  py::size_t i;
  py::bytes rv;
  signed char *ncp;
  double maxval, minval;

  audioop_check_parameters(frag.size, width);

  maxval = static_cast<double>(maxvals[width]);
  minval = static_cast<double>(minvals[width]);

  try {
    ncp = new signed char[frag.size];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    double val = GETRAWSAMPLE(width, frag.ptr, i);
    int ival = fbound(val * factor, minval, maxval);
    SETRAWSAMPLE(width, ncp, i, ival);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size);
  delete[] ncp;

  return rv;
}

static py::bytes audioop_tomono_impl(py::buffer *fragment, int width,
                                     double lfactor, double rfactor) {
  py::buffer_info frag = fragment->request();
  signed char *cp, *ncp;
  py::size_t len, i;
  double maxval, minval;
  py::bytes rv;

  // cp = static_cast<signed char *>(frag.ptr);
  len = frag.size;
  audioop_check_parameters(len, width);
  if (((len / width) & 1) != 0)
    throw py::value_error("Not a whole number of frames");

  maxval = static_cast<double>(maxvals[width]);
  minval = static_cast<double>(minvals[width]);

  try {
    ncp = new signed char[len / 2];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < len; i += width * 2) {
    double val1 = GETRAWSAMPLE(width, frag.ptr, i);
    double val2 = GETRAWSAMPLE(width, frag.ptr, i + width);
    double val = val1 * lfactor + val2 * rfactor;
    int ival = fbound(val, minval, maxval);
    SETRAWSAMPLE(width, ncp, i / 2, ival);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), len / 2);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_tostereo_impl(py::buffer *fragment, int width,
                                       double lfactor, double rfactor) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  double maxval, minval;

  audioop_check_parameters(frag.size, width);

  maxval = static_cast<double>(maxvals[width]);
  minval = static_cast<double>(minvals[width]);

  if (frag.size > PY_SSIZE_T_MAX / 2)
    throw py::buffer_error("Not enough memory for output buffer");

  try {
    ncp = new signed char[frag.size * 2];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    double val = GETRAWSAMPLE(width, frag.ptr, i);
    int val1 = fbound(val * lfactor, minval, maxval);
    int val2 = fbound(val * rfactor, minval, maxval);
    SETRAWSAMPLE(width, ncp, i * 2, val1);          // Possible error source
    SETRAWSAMPLE(width, ncp, i * 2 + width, val2);  // Possible error source
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size * 2);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_add_impl(py::buffer *fragment1, py::buffer *fragment2,
                                  int width) {
  py::buffer_info frag1 = fragment1->request();
  py::buffer_info frag2 = fragment2->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  double maxval, minval, newval;

  py::size_t len1, len2;

  len1 = frag1.size;
  len2 = frag2.size;

  audioop_check_parameters(len1, width);
  audioop_check_parameters(len2, width);

  if (len1 != len2)
    throw py::index_error("Lenth of both fragment should be same");

  minval = static_cast<double>(minvals[width]);
  maxval = static_cast<double>(maxvals[width]);

  try {
    ncp = new signed char[len1];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < len1; i += width) {
    int val1 = GETRAWSAMPLE(width, frag1.ptr, i);
    int val2 = GETRAWSAMPLE(width, frag2.ptr, i);

    if (width < 4) {
      newval = val1 + val2;
      newval = std::clamp(newval, minval, maxval);
    } else {
      double fval = static_cast<double>(val1) + static_cast<double>(val2);
      newval = fbound(fval, minval, maxval);
    }
    SETRAWSAMPLE(width, ncp, i, newval);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), len1);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_bias_impl(py::buffer *fragment, int width, int bias) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned int val = 0, mask;

  audioop_check_parameters(frag.size, width);

  try {
    ncp = new signed char[frag.size];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  mask = masks[width];

  for (i = 0; i < frag.size; i += width) {
    if (width == 1) {
      val = GETINTX(unsigned char, frag.ptr, i);
    } else if (width == 2) {
      val = GETINTX(uint16_t, frag.ptr, i);
    } else if (width == 3) {
      val = static_cast<unsigned int>(GETINT24(frag.ptr, i)) & 0xffffffu;
    } else {
      assert(width == 4);
      val = GETINTX(uint32_t, frag.ptr, i);
    }

    val += static_cast<unsigned int>(bias);
    val &= mask;

    if (width == 1) {
      SETINTX(unsigned char, ncp, i, val);
    } else if (width == 2) {
      SETINTX(uint16_t, ncp, i, val);
    } else if (width == 3) {
      SETINT24(ncp, i, static_cast<int>(val));
    } else {
      assert(width == 4);
      SETINTX(uint32_t, ncp, i, val);
    }
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_reverse_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  unsigned char *ncp;

  audioop_check_parameters(frag.size, width);

  try {
    ncp = new unsigned char[frag.size];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    int val = GETRAWSAMPLE(width, frag.ptr, i);
    SETRAWSAMPLE(width, ncp, frag.size - i - width, val);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_byteswap_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  unsigned char *ncp;

  audioop_check_parameters(frag.size, width);

  try {
    ncp = new unsigned char[frag.size];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    int j;
    for (j = 0; j < width; j++)
      ncp[i + width - 1 - j] = (static_cast<unsigned char *>(frag.ptr))[i + j];
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_lin2lin_impl(py::buffer *fragment, int width,
                                      int newwidth) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i, j;

  audioop_check_parameters(frag.size, width);
  audioop_check_size(newwidth);

  if (frag.size / width > PY_SSIZE_T_MAX / newwidth)
    throw py::buffer_error("Not enough memory for output buffer");

  try {
    ncp = new unsigned char[(frag.size / width) * newwidth];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = j = 0; i < frag.size; i += width, j += newwidth) {
    int val = GETSAMPLE32(width, frag.ptr, i);
    SETSAMPLE32(newwidth, ncp, j, val);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp),
                 (frag.size / width) * newwidth);
  delete[] ncp;
  return rv;
}

static int _gcd(int a, int b) {
  while (b > 0) {
    int tmp = a % b;
    a = b;
    b = tmp;
  }
  return a;
}

// Not implemented yet
static py::bytes audioop_ratecv_impl(py::buffer *fragment, int width,
                                     int nchannels, int inrate, int outrate,
                                     py::object *state, int weightA,
                                     int weightB) {
  py::buffer_info frag = fragment->request();
  py::bytes samps, str, rv, channel;
  py::size_t len;
  char *ncp, *cp;
  int chan, d, *prev_i, *cur_i, cur_o, bytes_per_frame;

  audioop_check_size(width);
  if (nchannels < 1) throw py::value_error("# of channels should be >= 1");
  if (width > INT_MAX / nchannels)
    throw py::buffer_error("width * nchannels too big for a C int");

  bytes_per_frame = width * nchannels;
  try {
    ncp = new char[frag.size];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_lin2ulaw_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i;
  int j = 0;

  audioop_check_parameters(frag.size, width);

  try {
    ncp = new unsigned char[frag.size / width];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    int val = GETSAMPLE32(width, frag.ptr, i);
    ncp[j] = st_14linear2ulaw(val >> 18);
    j++;
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size / width);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_ulaw2lin_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned char *cp;
  int j = 0;

  audioop_check_size(width);
  if (frag.size > PY_SSIZE_T_MAX / width)
    throw py::buffer_error("not enough memory for output buffer");

  try {
    ncp = new signed char[frag.size * width];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  cp = static_cast<unsigned char *>(frag.ptr);
  for (i = 0; i < frag.size * width; i += width) {
    int val = st_ulaw2linear16(*cp++) << 16;
    SETSAMPLE32(width, ncp, i, val);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size * width);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_lin2alaw_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i;
  int j = 0;

  audioop_check_parameters(frag.size, width);

  try {
    ncp = new unsigned char[frag.size / width];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  for (i = 0; i < frag.size; i += width) {
    int val = GETSAMPLE32(width, frag.ptr, i);
    ncp[j] = st_linear2alaw(val >> 19);
    j++;
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size / width);
  delete[] ncp;
  return rv;
}

static py::bytes audioop_alaw2lin_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned char *cp;
  int j = 0;

  audioop_check_size(width);
  if (frag.size > PY_SSIZE_T_MAX / width)
    throw py::buffer_error("not enough memory for output buffer");

  try {
    ncp = new signed char[frag.size * width];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }
  cp = static_cast<unsigned char *>(frag.ptr);

  for (i = 0; i < frag.size * width; i += width) {
    // int temp = GETINT8(frag.ptr, j++);
    int val = st_alaw2linear16(*cp++) << 16;
    SETSAMPLE32(width, ncp, i, val);
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp), frag.size * width);
  delete[] ncp;
  return rv;
}

// Not implemented
static py::bytes audioop_lin2adpcm_impl(py::buffer *fragment, int width,
                                        py::object state);

// Not implemented
static py::bytes audioop_adpcm2lin_impl(py::buffer *fragment, int width,
                                        py::object state);

PYBIND11_MODULE(_audioop, m) {
  m.doc() = R"pbdoc(
        Audioop module
        -----------------------

        .. currentmodule:: _audioop

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

  m.def("_add", &audioop_add_impl, "");
  // m.def("_adpcm2lin", &audioop_adpcm2lin_impl, "");
  m.def("_alaw2lin", &audioop_alaw2lin_impl, "");
  m.def("_avg", &audioop_avg_impl, "");
  m.def("_avgpp", &audioop_avgpp_impl, "");
  m.def("_bias", &audioop_bias_impl, "");
  m.def("_byteswap", &audioop_byteswap_impl, "");
  m.def("_cross", &audioop_cross_impl, "");
  m.def("_findfactor", &audioop_findfactor_impl, "");
  m.def("_findfit", &audioop_findfit_impl, "");
  m.def("_findmax", &audioop_findmax_impl, "");
  m.def("_getsample", &audioop_getsample_impl, "");
  // m.def("_lin2adpcm", &audioop_lin2adpcm_impl, "");
  m.def("_lin2alaw", &audioop_lin2alaw_impl, "");
  m.def("_lin2lin", &audioop_lin2lin_impl, "");
  m.def("_lin2ulaw", &audioop_lin2ulaw_impl, "");
  m.def("_max", &audioop_max_impl, "");
  m.def("_maxpp", &audioop_maxpp_impl, "");
  m.def("_minmax", &audioop_minmax_impl, "");
  m.def("_mul", &audioop_mul_impl, "");
  // m.def("_ratecv", &audioop_ratecv_impl, "");
  m.def("_reverse", &audioop_reverse_impl, "");
  m.def("_rms", &audioop_rms_impl, "");
  m.def("_tomono", &audioop_tomono_impl, "");
  m.def("_tostereo", &audioop_tostereo_impl, "");
  m.def("_ulaw2lin", &audioop_ulaw2lin_impl, "");

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
