#pragma once
#ifndef ETHSIFT_DATA
#error "ETHSIFT_DATA must be defined."
#endif

#ifndef ETHSIFT_LOGS
#error "ETHSIFT_LOGS must be defined."
#endif

// Specify if using rdtsc or chrono for runtime measurements
#define USE_RDTSC 1

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

// Specify which measurements you would like to run.
#define RUN_ETHSIFT_MEASUREMENTS 1
#define RUN_EZSIFT_MEASUREMENTS 0

// Specify how many measurement runs it should do per function
extern int NR_RUNS;

// Scaling factor for the MAD. Assuming normal distribution:
#define K_FACTOR 1.4826

#if USE_RDTSC
  #include "tsc_x86.h"
  extern myInt64 start;
#else
  extern std::chrono::time_point<std::chrono::high_resolution_clock> start;
#endif

extern std::vector<size_t> durations;
typedef std::tuple<std::string, size_t, double> LogTuple;
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
#define fail(...) {                                     \
    char *__message = (char*)calloc(1024,sizeof(char)); \
    sprintf(__message, __VA_ARGS__);                    \
    register_failure(__testid, __message);              \
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

// Start a time measurement section.
// Note: If no explicit measurement sections are defined, the entire test
//       is measured instead.
// Note: Defined in header as static inline to avoid function call overhead
__attribute__((always_inline))
static inline void start_measurement(){
  if(!measurement_pending){
    measurement_pending = true;
    #if USE_RDTSC
      start = start_tsc();
    #else
      start = std::chrono::high_resolution_clock::now();
    #endif
  }
}

// End a time measurement section.
// Note: A single test may have multiple start/end sections. The report will
//       accumulate the measurements from every section.
__attribute__((always_inline))
static inline void end_measurement(){
  #if USE_RDTSC
    myInt64 runtime = stop_tsc(start);
    if(measurement_pending){
      durations.push_back((size_t) runtime);
      measurement_pending = false;
    }
  #else
    auto end = std::chrono::high_resolution_clock::now();
    if(measurement_pending){
      durations.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
      measurement_pending = false;
    }
  #endif
}

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
