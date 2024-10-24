#include "fragment.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "audioop.h"

namespace fragment {

AudioFragment::AudioFragment(const std::string &filename) {
  std::ifstream file(filename);
  file >> std::noskipws;
  std::vector<unsigned char> audio_vector(
      (std::istream_iterator<unsigned char>(file)),
      (std::istream_iterator<unsigned char>()));
  file.close();

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

  if (format == 1) {
    size_t size = audio_vector.size();
    buffer.assign(audio_file + 44, audio_file + size);
  } else {
    throw std::runtime_error("Only PCM is currently supported");
  }
}

AudioFragment::AudioFragment(py::buffer *raw_audio) {
  py::buffer_info buf = raw_audio->request();
  unsigned char *audio_file = static_cast<unsigned char *>(buf.ptr);

  std::string header;
  header.assign(reinterpret_cast<char *>(audio_file), 4);
  if (header != "RIFF")
    std::runtime_error("Only wav file is currently supported");

  int16_t format = *reinterpret_cast<int16_t *>(audio_file + 20);
  channels = *reinterpret_cast<int16_t *>(audio_file + 22);
  width = 2;
  sampling_rate = *reinterpret_cast<int32_t *>(audio_file + 24);

  if (format == 1) {
    size_t size = buf.size;
    buffer.assign(audio_file + 44, audio_file + size);
  } else {
    throw std::runtime_error("Only PCM is currently supported");
  }
}

AudioFragment::AudioFragment(py::buffer *fragment, int sampling_rate, int width,
                             int channels)
    : sampling_rate(sampling_rate), width(width), channels(channels) {
  py::buffer_info buf = fragment->request();
  unsigned char *audio_file = static_cast<unsigned char *>(buf.ptr);
  size_t size = buf.size;
  buffer.assign(audio_file, audio_file + size);
  check_size(width);
  check_parameters(buf.size, width);
}

AudioFragment::~AudioFragment() {}

py::array AudioFragment::get_array_of_fragments() {
  std::vector<int16_t> audio_array;
  int array_size = buffer.size() / width;
  audio_array.reserve(array_size);

  for (auto i = 0; i < buffer.size(); i += width) {
    audio_array.push_back(buffer[i + 1] << 8 | buffer[i]);
  }

  return py::array(py::dtype::of<int16_t>(), {audio_array.size()},
                   {sizeof(int16_t)}, audio_array.data());
}

int AudioFragment::get_sampling_rate() { return sampling_rate; }
int AudioFragment::get_width() { return width; }
int AudioFragment::get_channels() { return channels; }

std::vector<unsigned char> AudioFragment::riff_header() {
  WAVHeader header;
  header.sampling_rate = sampling_rate;
  header.channels = channels;
  header.sub_chunk2_size = buffer.size();

  header.byte_rate = sampling_rate * channels * header.bits_per_sample / 8;
  header.block_align = channels * header.bits_per_sample / 8;
  header.chunk_size = 36 + header.sub_chunk2_size;

  std::vector<unsigned char> wav_header(44);
  std::memcpy(wav_header.data(), &header, sizeof(WAVHeader));

  return wav_header;
}

py::bytes AudioFragment::py_riff_header() {
  py::bytes rv;
  std::vector<unsigned char> header = riff_header();
  std::string header_string(header.begin(), header.end());
  rv = py::bytes(header_string);
  return rv;
}

py::bytes AudioFragment::_get_raw_bytes() {
  py::bytes rv;
  std::string raw_string(buffer.begin(), buffer.end());
  rv = py::bytes(raw_string);
  return rv;
}

}  // namespace fragment

void _init_submodule_fragment(pybind11::module_ &m) {
  auto m_a = m.def_submodule("fragment", "Manages Audio.");
  py::class_<fragment::AudioFragment>(m_a, "AudioFragment")
      .def(py::init([](const std::string &filepath) {
             return std::make_unique<fragment::AudioFragment>(filepath);
           }),
           py::arg("file_path"))

      .def(py::init([](py::buffer &buf) {
             return std::make_unique<fragment::AudioFragment>(&buf);
           }),
           py::arg("audio_buffer"))
      .def(py::init<py::buffer *, int, int, int>(), py::arg("audio_buffer"),
           py::arg("sampling_rate"), py::arg("width"), py::arg("channels"))
      .def("get_array_of_fragments",
           &fragment::AudioFragment::get_array_of_fragments, "")
      .def("_get_raw_bytes", &fragment::AudioFragment::_get_raw_bytes, "")
      .def("_create_riff_header", &fragment::AudioFragment::py_riff_header, "")
      .def("get_sampling_rate", &fragment::AudioFragment::get_sampling_rate, "")
      .def("get_width", &fragment::AudioFragment::get_width, "")
      .def("get_channels", &fragment::AudioFragment::get_channels, "");
}
