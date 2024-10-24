cmake_minimum_required(VERSION 3.4...3.18)
project(python-audio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
set(SOURCES
  lib/audioop.cpp
  lib/fragment.cpp
  lib/utils.cpp
  lib/main.cpp
)

find_package(pybind11 REQUIRED)
pybind11_add_module(_core ${SOURCES})
target_compile_definitions(_core PRIVATE VERSION_INFO="$VERSION_INFO")
