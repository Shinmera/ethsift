#pragma once
#ifndef ETHSIFT_DATA
#error "ETHSIFT_DATA must be defined."
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

extern std::chrono::time_point<std::chrono::high_resolution_clock> start;
extern std::vector<size_t> durations;
typedef std::tuple<std::string, size_t, size_t> LogTuple;
extern std::vector<LogTuple> test_logs;
extern bool measurement_pending;
#define NR_RUNS 30
#define EPS 0.001
#define OCTAVE_COUNT 6
#define GAUSSIAN_COUNT 6
#define DOG_COUNT 5
#define GRAD_ROT_LAYERS 3

#define LENA_KEYPOINTS 136

static std::string* g_testImgName;

int register_failure(int test, const char *reason);
int register_test(const char *title, int has_measurement_comp, int (*func)());

// Macro to define new test cases.
// Note that the test title must be a valid C token, so it may only contain
// alphanumerics or underscores.
#define define_test(TITLE, HAS_MEASUREMENT_COMP,...)                                          \
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
    std::cout << "PATH TO IMAGE FILE: " << path << std::endl;
    return path;
}

// Return an absolute path to a file within the project root's data/ directory.
static char* data_file() {
    const char* cstr = g_testImgName->c_str();
    std::cout << "IMAGE FILE: " << cstr << "     should be "<<  g_testImgName <<  std::endl;
    return data_file(cstr);
}


// Allocate the pixel array in the given output image according to its width and height.
struct ethsift_image allocate_image(uint32_t width, uint32_t height);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<unsigned char> &input, struct ethsift_image *output);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<float> &input, struct ethsift_image *output);

// Convert ezSift Keypoint to eth_sift keypoint
struct ethsift_keypoint convert_keypoint(ezsift::SiftKeypoint *k);

// Directly load an ethsift image
int load_image(const char *file, struct ethsift_image &image);

// Compare two images for pixel precise equality
int compare_image(const ezsift::Image<unsigned char> &ez_img, struct ethsift_image &eth_img);

// Compare two images for pixel precise equality. Returns 1 if they match, 0 otherwise.
int compare_image(struct ethsift_image a, struct ethsift_image b);

// Compare two images for approximate equality. Returns 1 if they match, 0 otherwise.
// Pixels are considered to be equal if their difference is smaller than eps.
int compare_image_approx(const ezsift::Image<unsigned char> &ez_img, struct ethsift_image &eth_img);
int compare_image_approx(const ezsift::Image<float> &ez_img, struct ethsift_image &eth_img);

// Compare two images for approximate equality. Returns 1 if they match, 0 otherwise.
// Pixels are considered to be equal if their difference is smaller than eps.
int compare_image_approx(struct ethsift_image a, struct ethsift_image b, float eps);

// Compare an ezsift kernel with an ethsift kernel for correctness
int compare_kernel(std::vector<float> ez_kernel, float* eth_kernel, int eth_kernel_size);

// Compare an ezsift descriptor with an ethsift descriptor for correctness
int compare_descriptor(float* ez_descriptors, float* eth_descriptors);

// Write an eth_sift image to pgm format
int write_image(struct ethsift_image image, const char* filename);

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

// Convenience macro to measure a section.
#define with_measurement(...) start_measurement(); __VA_ARGS__ end_measurement();
