cmake_minimum_required(VERSION 3.1)
project(ethsift)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

set(EZSIFT_INCLUDE_DIR ezsift/include)
set(EZSIFT_SRC_DIR ezsift/src)
file(GLOB_RECURSE EZSIFT_SRC RELATIVE ${PROJECT_SOURCE_DIR} "${EZSIFT_SRC_DIR}/*.cpp")

include_directories(${EZSIFT_INCLUDE_DIR} include)

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

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Adding debug flags...")
  target_link_options(tester PRIVATE -pg)
  target_compile_options(ethsift PRIVATE -DDEBUG -g -pg -O0 -Wall -Wno-unused-variable)

  add_custom_target(gprof
    COMMAND ./tester lena.pgm eth_MeasureFull
    COMMAND gprof tester > gprof.txt
    COMMAND echo "Wrote gropf output to ${CMAKE_BINARY_DIR}/gropf.text"
    DEPENDS tester)
else()
  message("Adding optimise flags...")
  target_compile_options(ethsift PRIVATE -O3 -mfma -mavx2)

  add_custom_target(gprof
    COMMAND echo "Cannot run gprof! Please set CMAKE_BUILD_TYPE to Debug!")
endif()

add_custom_target(flamegraph
  COMMAND perf record -F MAX -g ./tester lena.pgm eth_MeasureFull
  COMMAND perf script | "${PROJECT_SOURCE_DIR}/perf/stackcollapse-perf.pl" > out.perf-folded
  COMMAND "${PROJECT_SOURCE_DIR}/perf/flamegraph.pl" out.perf-folded > flamegraph.svg
  COMMAND echo "Wrote flamegraph to ${CMAKE_BINARY_DIR}/flamegraph.svg"
  DEPENDS tester)
