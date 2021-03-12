cmake_minimum_required(VERSION 3.16)

#include(CTest)
set(CMAKE_CXX_STANDARD 20)
include(CMakeCompileOptions.txt)
project(parcae_meta)

include_directories(parcae)
add_subdirectory(parcae)

include_directories(example)
add_subdirectory(example)

include(CMakeDoc.txt)
