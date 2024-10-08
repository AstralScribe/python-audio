cmake_minimum_required(VERSION 3.4...3.18)
project(_audioop)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(pybind11 REQUIRED)
pybind11_add_module(_audioop lib/audioop.cpp)
target_compile_definitions(_audioop PRIVATE VERSION_INFO="$VERSION_INFO")
