#include "ethsift.h"
#include "ezsift.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <chrono>

extern std::chrono::time_point<std::chrono::high_resolution_clock> start;
extern size_t duration;
extern bool measurement_pending;

int register_test(const char *title, int (*func)());

// Macro to define new test cases.
// Note that the test title must be a valid C token, so it may only contain
// alphanumerics or underscores.
#define define_test(TITLE,BODY) static int __test_ ## TITLE = register_test(# TITLE, []()BODY);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<unsigned char> &input, struct ethsift_image *output);

// Directly load an ethsift image
int load_image(const char *file, struct ethsift_image &image);


// Compare two images for pixel precise equality
int compare_image(ezsift::Image<unsigned char> &ez_img, struct ethsift_image &eth_img);

// Compare two images for pixel precise equality
int compare_image(struct ethsift_image a, struct ethsift_image b);

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
