#include "fragment.h"

#include <fstream>
#include <iostream>

#include "audioop.h"

namespace fragment {

AudioFragment::AudioFragment(std::string &filename) {
  std::ifstream file(filename);
  file >> std::noskipws;
  std::vector<unsigned char> audio_vector(
      (std::istream_iterator<unsigned char>(file)),
      (std::istream_iterator<unsigned char>()));

  unsigned char *audio_file = audio_vector.data();
  std::string header;
  header.assign(reinterpret_cast<char *>(audio_file), 4);
  if (header != "RIFF")
    std::runtime_error("Only wav file is currently supported");

  // int32_t file_size = *reinterpret_cast<int32_t *>(audio_file + 4);
  int16_t format = *reinterpret_cast<int16_t *>(audio_file + 20);
  channels = *reinterpret_cast<int16_t *>(audio_file + 22);
  width = 2;
  sampling_rate = *reinterpret_cast<int32_t *>(audio_file + 24);
  size = audio_vector.size() - 44;

  std::cout << audio_vector.size() << std::endl;

  if (format == 1) {
    buffer = audio_file + 44;
  } else {
    throw std::runtime_error("Only PCM is currently supported");
  }
}

AudioFragment::AudioFragment(py::buffer *fragment) {
  py::buffer_info buf = fragment->request();
  py::size_t size = buf.size;
  unsigned char *audio_file = static_cast<unsigned char *>(buf.ptr);

  std::string header;
  header.assign(reinterpret_cast<char *>(audio_file), 4);
  if (header != "RIFF")
    std::runtime_error("Only wav file is currently supported");

  // int32_t file_size = *reinterpret_cast<int32_t *>(audio_file + 4);
  int16_t format = *reinterpret_cast<int16_t *>(audio_file + 20);
  channels = *reinterpret_cast<int16_t *>(audio_file + 22);
  width = 2;
  sampling_rate = *reinterpret_cast<int32_t *>(audio_file + 24);

  std::cout << "Format: " << format << std::endl;

  if (format == 1) {
    buffer = audio_file + 44;
  } else {
    throw std::runtime_error("Only PCM is currently supported");
  }
}

AudioFragment::AudioFragment(py::buffer *fragment, int sampling_rate, int width,
                             int channels)
    : buffer(static_cast<unsigned char *>(fragment->request().ptr)),
      size(fragment->request().size),
      sampling_rate(sampling_rate),
      width(width),
      channels(channels) {
  check_size(width);
  check_parameters(size, width);
}
AudioFragment::~AudioFragment() {}

py::array AudioFragment::get_array_of_fragment() {
  std::vector<int16_t> audio_array;
  int array_size = size / width;
  audio_array.reserve(array_size);

  for (auto i = 0; i < size; i += width) {
    audio_array.push_back(buffer[i + 1] << 8 | buffer[i]);
  }

  return py::array(py::dtype::of<int16_t>(), {audio_array.size()},
                   {sizeof(int16_t)}, audio_array.data());
}

int AudioFragment::get_sampling_rate() { return sampling_rate; }
int AudioFragment::get_width() { return width; }
int AudioFragment::get_channels() { return channels; }

}  // namespace fragment

void _init_submodule_fragment(pybind11::module_ &m) {
  auto m_a = m.def_submodule("fragment", "Manages Audio.");
  py::class_<fragment::AudioFragment>(m_a, "AudioFragment")
      .def(py::init<std::string &>())
      .def(py::init<py::buffer *>())
      .def(py::init<py::buffer *, int, int, int>())
      .def("get_array_of_fragment",
           &fragment::AudioFragment::get_array_of_fragment, "")
      .def("get_sampling_rate", &fragment::AudioFragment::get_sampling_rate, "")
      .def("get_width", &fragment::AudioFragment::get_width, "")
      .def("get_channels", &fragment::AudioFragment::get_channels, "");
}
