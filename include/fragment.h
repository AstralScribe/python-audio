#ifndef __FRAGMENT_CPP_H__

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace fragment {

struct WAVHeader {
  char riff[4] = {'R', 'I', 'F', 'F'};
  uint32_t chunk_size;
  char wave[4] = {'W', 'A', 'V', 'E'};
  char fmt[4] = {'f', 'm', 't', ' '};
  uint32_t sub_chunk1_size = 16;
  uint16_t audio_format = 1;
  uint16_t channels;
  uint32_t sampling_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample = 16;
  char data[4] = {'d', 'a', 't', 'a'};
  uint32_t sub_chunk2_size;
};

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
  py::array get_array_of_fragments();
  py::bytes _get_raw_bytes();
  int get_sampling_rate();
  int get_width();
  int get_channels();
  float audio_duration();

  std::vector<unsigned char> riff_header();
  py::bytes py_riff_header();
};

}  // namespace fragment

void _init_submodule_fragment(pybind11::module_& m);
#endif  // !__FRAGMENT_CPP_H__
