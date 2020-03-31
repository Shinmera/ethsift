#pragma once
#ifndef ETHSIFT_DATA
#error "ETHSIFT_DATA must be defined."
#endif

#include "ethsift.h"
#include "ezsift.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <vector>

extern std::chrono::time_point<std::chrono::high_resolution_clock> start;
extern size_t duration;
extern bool measurement_pending;
#define EPS 0.001
#define OCTAVE_COUNT 6
#define GAUSSIAN_COUNT 6
#define DOG_LAYERS 5
#define GRAD_ROT_LAYERS 3

int register_test(const char *title, int (*func)());

// Macro to define new test cases.
// Note that the test title must be a valid C token, so it may only contain
// alphanumerics or underscores.
#define define_test(TITLE,...) static int __test_ ## TITLE = register_test(# TITLE, []()__VA_ARGS__);

// Return an absolute path to a file within the project root's data/ directory.
static char *data_file(const char *file){
  const char *data = ETHSIFT_DATA;
  char *path = (char*)calloc(sizeof(char), strlen(data)+strlen(file)+1);
  path = strcat(path, data);
  path = strcat(path, "/");
  path = strcat(path, file);
  return path;
}

// Allocate the pixel array in the given output image according to its width and height.
struct ethsift_image allocate_image(uint32_t width, uint32_t height);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<unsigned char> &input, struct ethsift_image *output);
int convert_image(const ezsift::Image<float> &input, struct ethsift_image *output);

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


int compare_kernel(std::vector<float> ez_kernel, float* eth_kernel, int eth_kernel_size);

int write_image(struct ethsift_image image);

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
    duration += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    measurement_pending = false;
  }
}

// Convenience macro to measure a section.
#define with_measurement(BODY) start_measurement(); BODY end_measurement();
