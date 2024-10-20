#include "fragment.h"

#include <pybind11/pybind11.h>

#include <cstdint>

#include "audioop.h"

namespace fragment {
AudioFragment::AudioFragment(py::buffer *buffer) : buffer(buffer) {
  py::buffer_info buf = buffer->request();
  py::size_t size = buf.size;
}
AudioFragment::AudioFragment(py::buffer *buffer, int sampling_rate, int width,
                             int channels)
    : buffer(buffer),
      sampling_rate(sampling_rate),
      width(width),
      channels(channels) {
  py::buffer_info buf = buffer->request();
  py::size_t size = buf.size;

  audioop_check_size(width);
  audioop_check_parameters(size, width);

  int array_size = size / width;
  array.reserve(array_size);

  for (int i = 0; i < array_size; i += width) {
    array.push_back(GETINT16(buf.ptr, i));
  }
}
std::vector<int16_t> AudioFragment::get_array_of_fragment() { return array; }
int AudioFragment::get_sampling_rate() { return sampling_rate; }
int AudioFragment::get_width() { return width; }
int AudioFragment::get_channels() { return channels; }

}  // namespace fragment

class Pet {
 public:
  Pet(const std::string &name) : name(name) {}
  void setName(const std::string &name_) { name = name_; }
  const std::string &getName() const { return name; }

 private:
  std::string name;
};
// py::class_<Pet>(m, "Pet")
//     .def(py::init<const std::string &>())
//     .def_property
//     ("name", &Pet::getName, &Pet::setName)
//
// PYBIND11_MODULE(_fragment, m) {
//   py::class_<fragment::AudioFragment>(m, "AudioFragment")
//     .def(py::init<py::buffer *, int, int, int>())
//     .def()
// }
