cmake_minimum_required(VERSION 3.0)
project(_audioop)

set(CMAKE_CXX_STANDARD 17)  # Replace 17 with the desired C++ version (11, 14, 17, 20, etc.)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pybind11)

pybind11_add_module(_audioop lib/audioop.cpp)
target_compile_definitions(_audioop PRIVATE VERSION_INFO="0.1.0")
