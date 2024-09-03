cmake_minimum_required(VERSION 3.18)
project(_audioop)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(pybind11)

pybind11_add_module(_audioop lib/audioop.cpp)
target_compile_definitions(_audioop PRIVATE VERSION_INFO="$VERSION_INFO")
