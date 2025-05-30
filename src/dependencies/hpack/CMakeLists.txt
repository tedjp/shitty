# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(
	huffy
)

add_library(hpack)
target_link_libraries(
	hpack
	huffy
)

target_sources(hpack
	PRIVATE
	error.h
	header.h
	number.c
	number.h
	header.cpp
)

add_executable(hpack-test test.c)
target_link_libraries(hpack-test hpack)
