cmake_minimum_required(VERSION 3.10)
project(is-wire)

set(build_tests true CACHE BOOL "build tests")
set(build_benchmarks false CACHE BOOL "build benchmarks")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

if (build_tests)
  enable_testing()
endif()

add_subdirectory(wire)

if (build_benchmarks)
  add_subdirectory(benchmark)
endif()