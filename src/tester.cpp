// File implementing a test harness of some kind.
#include "ethsift.h"
#include "ezsift.h"
#include <string.h>
#include <stdio.h>
#include <chrono>

int convert_image(const ezsift::Image<unsigned char> &input,
                  struct ethsift_image *output){
  output->width = input.w;
  output->height = input.h;
  output->pixels = (float*)calloc(sizeof(float), input.w*input.h);
  if(output->pixels == 0) return 0;
  for(size_t y=0; y<input.h; ++y){
    for(size_t x=0; x<input.w; ++x){
      size_t idx = y*input.w+x;
      output->pixels[idx] = (float) input.data[idx];
    }
  }
  return 1;
}

void fail(const char *message){
  fprintf(stderr, "\033[1;31m[ERROR]\033[0;0m %s\n", message);
  exit(1);
}

struct test{
  const char *title;
  int (*func)();
};

int run_test(struct test test){
  fprintf(stderr, "Running %-32s \033[0;90m...\033[0;0m ", test.title);
  auto t1 = std::chrono::high_resolution_clock::now();
  int ret = test.func();
  auto t2 = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
  fprintf(stderr, " %10iµs ", duration);
  fprintf(stderr, (ret==0)?"\033[1;31m[FAIL]":"\033[0;32m[OK  ]");
  fprintf(stderr, "\033[0;0m\n");
  return ret;
}

int run_tests(struct test *tests, uint32_t count){
  const char *failed[count];
  int failures = 0;
  int passes = 0;

  fprintf(stderr, "\033[1;33m --> \033[0;0mRunning %i tests\n", count);
  for(int i=0; i<count; ++i){
    if(run_test(tests[i])){
      passes++;
    }else{
      failed[failures] = tests[i].title;
      failures++;
    }
  }
  fprintf(stderr, "\nPassed: %3i", passes);
  fprintf(stderr, "\nFailed: %3i", failures);
  if(failures){
    fprintf(stderr, "\n\033[1;33m --> \033[0;0mThe following tests failed:\n");
    for(int i=0; i<failures; ++i){
      fprintf(stderr, "%s\n", failed[i]);
    }
  }
  return (failures == 0);
}

void compute_keypoints(char *file, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  ezsift::Image<unsigned char> img;
  if(img.read_pgm(file) != 0)
    fail("Failed to read image!");

  struct ethsift_image image = {0};
  if(!convert_image(img, &image))
    fail("Could not convert image");

  if(!ethsift_compute_keypoints(image, keypoints, keypoint_count))
    fail("Failed to compute keypoints");
}

void complete_run(char *file1, char *file2){
  uint32_t keypoint_count = 128;
  struct ethsift_keypoint keypoints[keypoint_count] = {0};
  compute_keypoints(file1, keypoints, &keypoint_count);
  fprintf(stdout, "Found %i keypoints in %s.\n", keypoint_count, file1);

  if(file2){
    uint32_t keypoint_count2 = 128;
    struct ethsift_keypoint keypoints2[keypoint_count] = {0};
    compute_keypoints(file2, keypoints2, &keypoint_count2);
    fprintf(stdout, "Found %i keypoints in %s.\n", keypoint_count2, file2);

    uint32_t match_count = 128;
    struct ethsift_match matches[match_count] = {0};
    if(!ethsift_match_keypoints(keypoints, keypoint_count, keypoints2, keypoint_count2, matches, &match_count))
      fail("Failed to match up keypoints.");

    // TODO: Show matches
  }else{
    // TODO: Show keypoints
  }
}

int main(int argc, char *argv[]){
  if(!ethsift_init())
    fail("Failed to initialise ETHSIFT");

  if(argc <= 1){
    int test_count = 2;
    struct test tests[2] = 
      {{"Dummy pass", [](){return 1;}},
       {"Dummy fail", [](){return 0;}}};
    return (run_tests(tests, test_count) == 0)? 1 : 0;
  }else{
    char *file1 = (1 < argc)? argv[1] : 0;
    char *file2 = (2 < argc)? argv[2] : 0;
    complete_run(file1, file2);
  }
  return 0;
}
