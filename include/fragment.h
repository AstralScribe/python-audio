#ifndef __FRAGMENT_CPP_H__

#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace fragment {

class AudioFragment {
 private:
  py::buffer* buffer;
  int sampling_rate;
  int width;
  int channels;

  std::vector<int16_t> array;

 public:
  explicit AudioFragment(py::buffer* buffer);
  AudioFragment(py::buffer* buffer, int sampling_rate, int width, int channels);
  ~AudioFragment();

 public:
  std::vector<int16_t> get_array_of_fragment();
  int get_sampling_rate();
  int get_width();
  int get_channels();
};

}  // namespace fragment

#endif  // !__FRAGMENT_CPP_H__
