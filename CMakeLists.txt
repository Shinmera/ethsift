cmake_minimum_required(VERSION 3.1)
project(ethsift)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

option(MEASUREMENT_MODE "Which mode to use for measurement")
if(NOT MEASUREMENT_MODE)
  set(MEASUREMENT_MODE "chrono" CACHE STRING "Choose the type of measurement (rdtsc or chrono)" FORCE)
endif()

option(OPT_FLAGS "Which set of flags to use for optimisation")
if(NOT OPT_FLAGS)
  set(OPT_FLAGS "full" CACHE STRING "Choose the set of optimisation flags (full, avx, O3, O0)" FORCE)
endif()

set(EZSIFT_INCLUDE_DIR ezsift/include)
set(EZSIFT_SRC_DIR ezsift/src)
file(GLOB_RECURSE EZSIFT_SRC RELATIVE ${PROJECT_SOURCE_DIR} "${EZSIFT_SRC_DIR}/*.cpp")

include_directories(${EZSIFT_INCLUDE_DIR} include)

## Fetch GIT version
execute_process(
    COMMAND "git" describe --tags
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE ETHSIFT_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

set_property(GLOBAL APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/.git/index")

## EZSift Build
add_library(ezsift STATIC ${EZSIFT_SRC})
target_include_directories(ezsift PUBLIC ${EZSIFT_SRC_DIR})
set_property(TARGET ezsift PROPERTY CXX_STANDARD 14)

## ETHSift Build
add_library(ethsift STATIC
  "include/ethsift.h"
  "src/internal.h"
  "src/settings.h"
  "src/gaussian_kernel.c"
  "src/gaussian_pyramid.c"
  "src/dog_pyramid.c"
  "src/grad_rot_pyramid.c"
  "src/octaves.c"
  "src/downscale.c"
  "src/apply_kernel.c"
  "src/detect_keypoints.c"
  "src/refine_local_extrema.c"
  "src/histogram.c"
  "src/extract_descriptor.c"
  "src/allocate.c"
  "src/compute_keypoints.c"
  "src/init.c"
  "src/stub.c"
  "src/flop_counters.h"
  )
target_include_directories(ethsift PUBLIC src)
target_compile_definitions(ethsift PRIVATE ETHSIFT_VERSION="${ETHSIFT_VERSION}")
set_property(TARGET ethsift PROPERTY C_STANDARD 99)


## Test Harness Build
add_executable(tester
  "src/tester.cpp"
  "src/test_utils.cpp"
  "src/tests.cpp"
  "src/ethsift_measurements.cpp"
  "src/ezsift_measurements.cpp"
  )
set_property(TARGET tester PROPERTY CXX_STANDARD 14)
target_link_libraries(tester PRIVATE ezsift ethsift)
target_compile_options(tester PRIVATE -DDEBUG  -g -pg -O0 -Wall -Wno-unused-variable)
target_compile_definitions(tester PRIVATE ETHSIFT_DATA="${PROJECT_SOURCE_DIR}/data")
target_compile_definitions(tester PRIVATE ETHSIFT_LOGS="${PROJECT_SOURCE_DIR}/logs")

if(MEASUREMENT_MODE STREQUAL "rdtsc")
  message("Building for rdtsc measurement")
  target_compile_definitions(tester PRIVATE USE_RDTSC=1)
else()
  message("Building for chrono measurement")
  target_compile_definitions(tester PRIVATE USE_RDTSC=0)
endif()

## Executable to count flops and memory reads/writes
add_executable(count_flops
  "include/ethsift.h"
  "src/internal.h"
  "src/settings.h"
  "src/gaussian_kernel.c"
  "src/gaussian_pyramid.c"
  "src/dog_pyramid.c"
  "src/grad_rot_pyramid.c"
  "src/octaves.c"
  "src/downscale.c"
  "src/apply_kernel.c"
  "src/detect_keypoints.c"
  "src/refine_local_extrema.c"
  "src/histogram.c"
  "src/extract_descriptor.c"
  "src/allocate.c"
  "src/compute_keypoints.c"
  "src/stub.c"
  "src/init.c"
  "src/flop_counters.h"
  "src/count_flops.h"
  "src/count_flops.c"
  )
target_link_libraries(count_flops PRIVATE m)

set_property(TARGET count_flops PROPERTY C_STANDARD 99)
target_compile_options(count_flops PRIVATE -DDEBUG -g -pg -O0)
target_compile_definitions(count_flops PRIVATE ETHSIFT_DATA="${PROJECT_SOURCE_DIR}/data")
target_compile_definitions(count_flops PRIVATE ETHSIFT_LOGS="${PROJECT_SOURCE_DIR}/logs")
target_compile_definitions(count_flops PRIVATE IS_COUNTING)
target_compile_definitions(count_flops PRIVATE ETHSIFT_VERSION="${ETHSIFT_VERSION}")

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Adding debug flags")
  set_property(TARGET tester APPEND_STRING PROPERTY LINK_FLAGS " -pg")
  target_compile_options(ethsift PRIVATE -DDEBUG -g -pg -O0 -Wall -Wno-unused-variable)

  add_custom_target(gprof
    COMMAND ./tester lena.pgm eth_MeasureFull
    COMMAND gprof tester > gprof.txt
    COMMAND echo "Wrote gropf output to ${CMAKE_BINARY_DIR}/gropf.text"
    DEPENDS tester)
else()
  message("Adding optimise flags (${OPT_FLAGS})")
  if(OPT_FLAGS MATCHES full)
    target_compile_options(ethsift PRIVATE -O3 -mfma -mavx2 -march=skylake -flto -ffast-math -fno-unsafe-math-optimizations)
    set_property(TARGET ethsift APPEND_STRING PROPERTY LINK_FLAGS " -flto")
    set_property(TARGET tester APPEND_STRING PROPERTY LINK_FLAGS " -flto")
  elseif(OPT_FLAGS MATCHES fastmath)
    # unsafe-math-optimizations breaks rotation pyramid generation.
    target_compile_options(ethsift PRIVATE -O3 -ffast-math -fno-unsafe-math-optimizations)
  elseif(OPT_FLAGS MATCHES avx)
    target_compile_options(ethsift PRIVATE -O3 -mfma -mavx2 -march=native)
  elseif(OPT_FLAGS MATCHES O3NOTREE)
    target_compile_options(ethsift PRIVATE -O3 -fno-tree-vectorize)
  elseif(OPT_FLAGS MATCHES O3)
    target_compile_options(ethsift PRIVATE -O3)
  elseif(OPT_FLAGS MATCHES O2)
    target_compile_options(ethsift PRIVATE -O2)
  elseif(OPT_FLAGS MATCHES O1)
    target_compile_options(ethsift PRIVATE -O1)
  elseif(OPT_FLAGS MATCHES O0)
    target_compile_options(ethsift PRIVATE -O0)
  else()
    message(FATAL_ERROR "Invalid OPT_FLAGS ${OPT_FLAGS}")
  endif()

  add_custom_target(gprof
    COMMAND echo "Cannot run gprof! Please set CMAKE_BUILD_TYPE to Debug!")
endif()

add_custom_target(flamegraph
  COMMAND perf record -F MAX -g "${CMAKE_BINARY_DIR}/tester" lena.pgm eth_MeasureFull
  COMMAND perf script | "${PROJECT_SOURCE_DIR}/perf/stackcollapse-perf.pl" > out.perf-folded
  COMMAND "${PROJECT_SOURCE_DIR}/perf/flamegraph.pl" out.perf-folded > flamegraph.svg
  COMMAND echo "Wrote flamegraph to ${CMAKE_BINARY_DIR}/flamegraph.svg"
  DEPENDS tester)

add_custom_target(plots
  COMMAND python3 "${PROJECT_SOURCE_DIR}/perf_plots/main.py")
