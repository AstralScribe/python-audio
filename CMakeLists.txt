cmake_minimum_required(VERSION 3.4...3.18)
project(python-audio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)

find_package(pybind11 REQUIRED)
pybind11_add_module(_audioop lib/audioop.cpp)
pybind11_add_module(_fragment lib/fragment.cpp)
target_compile_definitions(_audioop PRIVATE VERSION_INFO="$VERSION_INFO")
