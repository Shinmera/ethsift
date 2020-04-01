#include "internal.h"

int ethsift_init(){
  return 1;
}

int ethsift_compute_keypoints(struct ethsift_image image, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  return 0;
}

int ethsift_match_keypoints(struct ethsift_keypoint a[], uint32_t a_count, struct ethsift_keypoint b[], uint32_t b_count, struct ethsift_match matches[], uint32_t *match_count){
  return 0;
}
