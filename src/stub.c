#include "internal.h"


char *ethsift_version(){
#ifndef ETHSIFT_VERSION
#error "ETHSIFT_VERSION is required."
#else
  return ETHSIFT_VERSION;
#endif
}

int ethsift_match_keypoints(struct ethsift_keypoint a[], uint32_t a_count, struct ethsift_keypoint b[], uint32_t b_count, struct ethsift_match matches[], uint32_t *match_count){
  return 0;
}
