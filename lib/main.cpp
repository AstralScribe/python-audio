#include <pybind11/pybind11.h>

#include "audioop.h"
#include "fragment.h"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

PYBIND11_MODULE(_core, m) {
  _init_submodule_audioop(m);
  _init_submodule_fragment(m);

#ifdef VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
