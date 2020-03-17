// File implementing a test harness of some kind.
#include "ethsift.h"
#include "ezsift.h"
#include <string.h>
#include <stdio.h>

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
  fprintf(stderr, "[ERROR] %s\n", message);
  exit(1);
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

int main(int argc, char *argv[]){
  char *file1 = (char*)"../ezsift/data/img1.pgm";
  char *file2 = 0;
  if(1 < argc) file1 = argv[1];
  if(2 < argc) file2 = argv[2];
  
  if(!ethsift_init())
    fail("Failed to initialise ETHSIFT");

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
  return 0;
}
