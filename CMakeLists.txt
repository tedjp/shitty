# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 20)

# Absolute paths to source files
cmake_policy(SET CMP0076 NEW)

project ("shitty")

# Include source.
add_subdirectory ("src")
