#ifndef __AUDIOOP_CPP_H__

#include <pybind11/pybind11.h>

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

constexpr int16_t seg_aend[8] = {0x1F,  0x3F,  0x7F,  0xFF,
                                 0x1FF, 0x3FF, 0x7FF, 0xFFF};

constexpr int16_t seg_uend[8] = {0x3F,  0x7F,  0xFF,  0x1FF,
                                 0x3FF, 0x7FF, 0xFFF, 0x1FFF};

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

void check_size(int size);
void check_parameters(int len, int size);

void _init_submodule_audioop(pybind11::module_ &m);
#endif  // !__AUDIOOP_CPP_H__
