#ifndef __FRAGMENT_CPP_H__

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace fragment {

class AudioFragment {
 private:
  std::vector<unsigned char> buffer;
  int32_t sampling_rate;
  int16_t width;
  int16_t channels;

 public:
  AudioFragment(const std::string& filename);
  AudioFragment(py::buffer* raw_audio);
  AudioFragment(py::buffer* fragment, int sampling_rate, int width,
                int channels);
  ~AudioFragment();

 public:
  py::array get_array_of_fragment();
  py::bytes get_audio(std::string format);
  int get_sampling_rate();
  int get_width();
  int get_channels();
  int audio_length();
};

}  // namespace fragment

void _init_submodule_fragment(pybind11::module_& m);
#endif  // !__FRAGMENT_CPP_H__
