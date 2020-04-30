#pragma once
#ifndef ETHSIFT_DATA
#error "ETHSIFT_DATA must be defined."
#endif

#ifndef ETHSIFT_LOGS
#error "ETHSIFT_LOGS must be defined."
#endif


#include "ethsift.h"
#include "ezsift.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <tuple>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <time.h>
#include <signal.h>
#include <chrono>
#include <vector>
#include <algorithm>
#include "settings.h"
#include "test_utils.h"
#include "tsc_x86.h"

// Specify which measurements you would like to run.
#define RUN_ETHSIFT_MEASUREMENTS 1
#define RUN_EZSIFT_MEASUREMENTS 0

// Specify how many measurement runs it should do per function
#define NR_RUNS 30

// Specify if using rdtsc or chrono for runtime measures
#define USE_RDTSC 1

#if USE_RDTSC
  extern myInt64 start;
  extern std::vector<myInt64> durations;
  typedef std::tuple<std::string, myInt64, double> LogTuple;
#else
  extern std::chrono::time_point<std::chrono::high_resolution_clock> start;
  extern std::vector<size_t> durations;
  typedef std::tuple<std::string, size_t, double> LogTuple;
#endif

extern std::vector<LogTuple> test_logs;
extern bool measurement_pending;


extern std::string* g_testImgName;

int register_failure(int test, const char *reason);
int register_test(const char *title, int has_measurement_comp, int (*func)());

// Macro to define new test cases.
// Note that the test title must be a valid C token, so it may only contain
// alphanumerics or underscores.
#define define_test(TITLE, HAS_MEASUREMENT_COMP, ...)                                          \
  int __testfun_ ## TITLE();                                            \
  static int __test_ ## TITLE = register_test(# TITLE, HAS_MEASUREMENT_COMP, __testfun_ ## TITLE); \
  int __testfun_ ## TITLE (){                                           \
    int __testid = __test_ ## TITLE;                                    \
    __VA_ARGS__                                                         \
    return 1;                                                           \
  };

// Macro to fail a test. You should call this with a good reason whenever the test should fail.
#define fail(...) {                             \
    char __message[1024] = {0};                 \
    sprintf(__message, __VA_ARGS__);            \
    register_failure(__testid, __message);      \
    return 0;}

// Return an absolute path to a file within the project root's data/ directory.
static char* data_file(const char* file) {
    const char* data = ETHSIFT_DATA;
    char* path = (char*)calloc(sizeof(char), strlen(data) + strlen(file) + 1);
    path = strcat(path, data);
    path = strcat(path, "/");
    path = strcat(path, file);
    return path;
}

// Return an absolute path to a file within the project root's data/ directory.
static char* get_testimg_path() {
    const char* cstr = g_testImgName->c_str();
    //std::cout << "IMAGE FILE: " << cstr  <<  std::endl;
    return data_file(cstr);
}

#if USE_RDTSC
  //// Same runtime measurements using RDTSC.

  // Start a time measurement section.
  // Note: If no explicit measurement sections are defined, the entire test
  //       is measured instead.
  // Note: Defined in header as static inline to avoid function call overhead
  static inline void start_measurement(){
    if(!measurement_pending){
      measurement_pending = true;
      start = start_tsc();
    }
  }

  // End a time measurement section.
  // Note: A single test may have multiple start/end sections. The report will
  //       accumulate the measurements from every section.
  static inline void end_measurement(){
    myInt64 runtime = stop_tsc(start);
    if(measurement_pending){
      durations.push_back(runtime);
      measurement_pending = false;
    }
  }
  
#else
  // Start a time measurement section.
  // Note: If no explicit measurement sections are defined, the entire test
  //       is measured instead.
  // Note: Defined in header as static inline to avoid function call overhead
  static inline void start_measurement(){
    if(!measurement_pending){
      measurement_pending = true;
      start = std::chrono::high_resolution_clock::now();
    }
  }

  // End a time measurement section.
  // Note: A single test may have multiple start/end sections. The report will
  //       accumulate the measurements from every section.
  static inline void end_measurement(){
    auto end = std::chrono::high_resolution_clock::now();
    if(measurement_pending){
      durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
      measurement_pending = false;
    }
  }
#endif

// Convenience macro to measure a section.
#define with_measurement(...) start_measurement(); __VA_ARGS__ end_measurement();

// Convenience macro to measure something repeatedly with heated caches.
#define with_repeating(...)                     \
  for(int _i=0; _i<NR_RUNS; ++_i) {             \
    __VA_ARGS__;                                \
  }                                             \
  for(int _i=0; _i<NR_RUNS; ++_i) {             \
    start_measurement();                        \
    __VA_ARGS__;                                \
    end_measurement();                          \
  }
