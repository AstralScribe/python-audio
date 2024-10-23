#include <pybind11/pybind11.h>

#include <string>

namespace py = pybind11;

enum filters { HANN, KAISER, FFT };

static std::vector<float> resample(const std::vector<float> &val, int orig_sr,
                                   int tgt_sr, int axis, filters filter,
                                   bool parallel) {
  if (orig_sr <= 0) {
    std::string error = "Orginal sr < 0; orig_sr = " + std::to_string(orig_sr);
    throw py::value_error(error);
  }
  if (tgt_sr <= 0) {
    std::string error = "Target sr < 0; tgt_sr= " + std::to_string(tgt_sr);
    throw py::value_error(error);
  }

  float scale = static_cast<float>(tgt_sr) / static_cast<float>(orig_sr);
  auto oldlen = val.size();
  int newlen = static_cast<int>(oldlen * scale);
  std::vector<float> result;

  if (orig_sr == tgt_sr) return val;

  switch (filter) {
    case HANN:
      break;
    case KAISER:
      break;
    case FFT:
      break;
    default:
      throw py::value_error("Invalid filter selected.");
  }
  return result;
}
