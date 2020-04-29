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
  "src/stub.c"
  )
target_include_directories(ethsift PUBLIC src)
set_property(TARGET ethsift PROPERTY C_STANDARD 99)
target_compile_options(ethsift PRIVATE -g -O0 -Wall -Wno-unused-variable -mfma -mavx2)

## Test Harness Build
add_executable(tester
  "src/tester.cpp"
  "src/tests.cpp"
  "src/ethsift_measurements.cpp"
  "src/ezsift_measurements.cpp")
set_property(TARGET tester PROPERTY CXX_STANDARD 14)
target_link_libraries(tester PRIVATE ezsift ethsift)
target_compile_options(tester PRIVATE -g -O0 -Wall -Wno-unused-variable)
target_compile_definitions(tester PRIVATE ETHSIFT_DATA="${PROJECT_SOURCE_DIR}/data")

if(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Adding debug flag...")
  target_compile_definitions(ethsift PRIVATE -DDEBUG)
  target_compile_definitions(tester PRIVATE -DDEBUG)
endif()
