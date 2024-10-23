#include "audioop.h"

#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <new>
#include <numeric>

namespace py = pybind11;

int fbound(double val, double minval, double maxval) {
  val = std::clamp(val, minval, maxval);
  val = std::floor(val);
  return static_cast<int>(val);
}

int16_t search(int16_t val, const int16_t *table, int size) {
  for (int i = 0; i < size; i++)
    if (val <= *table++) return i;
  return size;
}

unsigned char st_14linear2ulaw(int16_t pcm_val) {
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

unsigned char st_linear2alaw(int16_t pcm_val) {
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

void check_size(int size) {
  if (size < 1 || size > 4)
    throw py::value_error("Size should be 1, 2, 3 or 4");
}

void check_parameters(int len, int size) {
  check_size(size);
  if (len % size != 0) throw py::value_error("Not a whole number of frames");
}

int getsample_impl(py::buffer *fragment, int width, int index) {
  int val;
  py::buffer_info buf = fragment->request();
  check_parameters(buf.size, width);

  if (index < 0 || index >= buf.size) {
    throw py::index_error("Index out of range");
    return 0;
  }
  val = GETRAWSAMPLE(width, buf.ptr, index * width);
  return val;
}

py::ssize_t max_impl(py::buffer *fragment, int width) {
  py::buffer_info buf = fragment->request();
  py::ssize_t i;
  unsigned int absval, max = 0;

  check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    int val = GETRAWSAMPLE(width, buf.ptr, i);
    absval = (val < 0) ? (unsigned int)-(int64_t)val : val;
    max = (absval > max) ? absval : max;
  }
  return max;
}

py::tuple minmax_impl(py::buffer *fragment, int width) {
  int min = 0x7fffffff, max = -0x7FFFFFFF - 1;
  py::size_t i;
  py::tuple result(2);
  py::buffer_info buf = fragment->request();

  check_parameters(buf.size, width);

  for (i = 0; i < buf.size; i += width) {
    int val = GETRAWSAMPLE(width, buf.ptr, i);
    if (val > max) max = val;
    if (val < min) min = val;
  }

  result[0] = min;
  result[1] = max;

  return result;
}

int avg_impl(py::buffer *fragment, int width) {
  int avg;
  double sum = 0.0;

  py::size_t i;
  py::buffer_info buf = fragment->request();

  check_parameters(buf.size, width);

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

unsigned int rms_impl(py::buffer *fragment, int width) {
  py::buffer_info buf = fragment->request();
  py::ssize_t i;

  unsigned int rms_val;
  double sum_sqre = 0.0;

  check_parameters(buf.size, width);

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

double _sum2(const int16_t *a, const int16_t *b, py::size_t len) {
  py::size_t i;
  double sum = 0.0;

  for (i = 0; i < len; i++) {
    sum += static_cast<double>(a[i]) * static_cast<double>(b[i]);
  }
  return sum;
}

py::tuple findfit_impl(py::buffer *fragment, py::buffer *reference) {
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

double findfactor_impl(py::buffer *fragment, py::buffer *reference) {
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

py::size_t findmax_impl(py::buffer *fragment, py::size_t length) {
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

unsigned int avgpp_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::size_t i;
  int prev_val, prev_extreme_valid = 0, prev_extreme = 0;
  double sum = 0.0;
  unsigned int avg;
  int diff, prev_diff, next_extreme = 0;

  check_parameters(frag.size, width);
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

unsigned int maxpp_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  check_parameters(frag.size, width);
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

py::size_t cross_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  check_parameters(frag.size, width);

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
py::bytes mul_impl(py::buffer *fragment, int width, double factor) {
  py::buffer_info frag = fragment->request();
  py::size_t i;
  py::bytes rv;
  signed char *ncp;
  double maxval, minval;

  check_parameters(frag.size, width);

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

py::bytes tomono_impl(py::buffer *fragment, int width, double lfactor,
                      double rfactor) {
  py::buffer_info frag = fragment->request();
  signed char *cp, *ncp;
  py::size_t len, i;
  double maxval, minval;
  py::bytes rv;

  // cp = static_cast<signed char *>(frag.ptr);
  len = frag.size;
  check_parameters(len, width);
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

py::bytes tostereo_impl(py::buffer *fragment, int width, double lfactor,
                        double rfactor) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  double maxval, minval;

  check_parameters(frag.size, width);

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

py::bytes add_impl(py::buffer *fragment1, py::buffer *fragment2, int width) {
  py::buffer_info frag1 = fragment1->request();
  py::buffer_info frag2 = fragment2->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  double maxval, minval, newval;

  py::size_t len1, len2;

  len1 = frag1.size;
  len2 = frag2.size;

  check_parameters(len1, width);
  check_parameters(len2, width);

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

py::bytes bias_impl(py::buffer *fragment, int width, int bias) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned int val = 0, mask;

  check_parameters(frag.size, width);

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

py::bytes reverse_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  unsigned char *ncp;

  check_parameters(frag.size, width);

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

py::bytes byteswap_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  unsigned char *ncp;

  check_parameters(frag.size, width);

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

py::bytes lin2lin_impl(py::buffer *fragment, int width, int newwidth) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i, j;

  check_parameters(frag.size, width);
  check_size(newwidth);

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

// Not implemented yet
py::bytes ratecv_impl(py::buffer *fragment, int width, int nchannels,
                      int inrate, int outrate, py::object *state, int weightA,
                      int weightB) {
  py::buffer_info frag = fragment->request();
  py::bytes samps, str, rv, channel;
  py::size_t len;
  char *ncp, *cp;
  int chan, d, cur_o, bytes_per_frame;

  check_size(width);
  if (nchannels < 1) throw py::value_error("# of channels should be >= 1");
  if (width > INT_MAX / nchannels)
    throw py::buffer_error("width * nchannels too big for a C int");

  bytes_per_frame = width * nchannels;

  assert(frag.size >= 0);
  check_parameters(frag.size, bytes_per_frame);
  if (weightA < 1 || weightB < 0)
    throw std::runtime_error("weightA should be >= 1, weightB should be >= 0");
  if (inrate <= 0 || outrate <= 0)
    throw std::runtime_error("sampling rate not > 0");

  d = std::gcd(inrate, outrate);
  inrate /= d;
  outrate /= d;

  d = std::gcd(weightA, weightB);
  weightA /= d;
  weightB /= d;

  std::vector<int> prev_i, cur_i;
  prev_i.reserve(nchannels);
  cur_i.reserve(nchannels);
  len = frag.size / bytes_per_frame; /* # of frames */

  if (state->is_none()) {
    d = -outrate;
    for (chan = 0; chan < nchannels; chan++) prev_i[chan] = cur_i[chan] = 0;
  } else {
    if (!py::isinstance<py::tuple>(*state)) {
      throw std::runtime_error("state must be a tuple or None");
    }

    py::tuple state_tuple = state->cast<py::tuple>();
    d = py::cast<int>(state_tuple[0]);
    py::tuple samps = state_tuple[1].cast<py::tuple>();

    if (samps.size() != nchannels) {
      throw std::runtime_error("illegal state argument");
    }
    for (int chan = 0; chan < nchannels; chan++) {
      py::tuple channel_state = samps[chan].cast<py::tuple>();
      prev_i[chan] = py::cast<int>(channel_state[0]);
      cur_i[chan] = py::cast<int>(channel_state[1]);
    }
  }

  int q = 1 + (frag.size - 1) / inrate;

  try {
    ncp = new char[q * outrate * bytes_per_frame];
  } catch (const std::bad_alloc &e) {
    std::cerr << "Memory allocation failed: " << e.what() << std::endl;
    throw py::buffer_error("Memory allocation failed");
  }

  cp = static_cast<char *>(frag.ptr);

  for (;;) {
    while (d < 0) {
      if (len == 0) {
        py::tuple result_state(nchannels);
        for (chan = 0; chan < nchannels; chan++) {
          result_state[chan] = py::make_tuple(prev_i[chan], cur_i[chan]);
        }
      }
    }
  }

  rv = py::bytes(reinterpret_cast<const char *>(ncp),
                 q * outrate * bytes_per_frame);
  delete[] ncp;
  return rv;
}

py::bytes lin2ulaw_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i;
  int j = 0;

  check_parameters(frag.size, width);

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

py::bytes ulaw2lin_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned char *cp;
  int j = 0;

  check_size(width);
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

py::bytes lin2alaw_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  unsigned char *ncp;
  py::size_t i;
  int j = 0;

  check_parameters(frag.size, width);

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

py::bytes alaw2lin_impl(py::buffer *fragment, int width) {
  py::buffer_info frag = fragment->request();
  py::bytes rv;
  py::size_t i;
  signed char *ncp;
  unsigned char *cp;
  int j = 0;

  check_size(width);
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
py::bytes lin2adpcm_impl(py::buffer *fragment, int width, py::object state);

// Not implemented
py::bytes adpcm2lin_impl(py::buffer *fragment, int width, py::object state);

void _init_submodule_audioop(py::module_ &m) {
  auto m_a =
      m.def_submodule("audioop", "Functions from removed battery audioop.");

  m_a.def("add", &add_impl, "");
  // m_a.def("adpcm2lin", &adpcm2lin_impl, "");
  m_a.def("alaw2lin", &alaw2lin_impl, "");
  m_a.def("avg", &avg_impl, "");
  m_a.def("avgpp", &avgpp_impl, "");
  m_a.def("bias", &bias_impl, "");
  m_a.def("byteswap", &byteswap_impl, "");
  m_a.def("cross", &cross_impl, "");
  m_a.def("findfactor", &findfactor_impl, "");
  m_a.def("findfit", &findfit_impl, "");
  m_a.def("findmax", &findmax_impl, "");
  m_a.def("getsample", &getsample_impl, "");
  // m_a.def("lin2adpcm", &lin2adpcm_impl, "");
  m_a.def("lin2alaw", &lin2alaw_impl, "");
  m_a.def("lin2lin", &lin2lin_impl, "");
  m_a.def("lin2ulaw", &lin2ulaw_impl, "");
  m_a.def("max", &max_impl, "");
  m_a.def("maxpp", &maxpp_impl, "");
  m_a.def("minmax", &minmax_impl, "");
  m_a.def("mul", &mul_impl, "");
  // m_a.def("ratecv", &ratecv_impl, "");
  m_a.def("reverse", &reverse_impl, "");
  m_a.def("rms", &rms_impl, "");
  m_a.def("tomono", &tomono_impl, "");
  m_a.def("tostereo", &tostereo_impl, "");
  m_a.def("ulaw2lin", &ulaw2lin_impl, "");
}
