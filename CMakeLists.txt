cmake_minimum_required(VERSION 3.1)
project(ethsift C CXX)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11")

set(EZSIFT_INCLUDE_DIR ezsift/include)
set(EZSIFT_SRC_DIR ezsift/src)
file(GLOB_RECURSE EZSIFT_SRC RELATIVE ${PROJECT_SOURCE_DIR} "${EZSIFT_SRC_DIR}/*.cpp")

include_directories(${EZSIFT_INCLUDE_DIR} include)

## EZSift Build
add_library(ezsift STATIC ${EZSIFT_SRC})
target_include_directories(ezsift PUBLIC ${EZSIFT_SRC_DIR})

## ETHSift Build
add_library(ethsift STATIC
  "include/ethsift.h"
  "src/stub.cpp")
target_include_directories(ethsift PUBLIC src)
set_target_properties(ethsift PROPERTIES LINKER_LANGUAGE CXX)

## Test Harness Build
add_executable(tester
  "src/tester.cpp")
target_link_libraries(tester PRIVATE ezsift ethsift)
