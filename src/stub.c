#include "internal.h"

int ethsift_init(){
  return 1;
}

int ethsift_allocate_pyramid(struct ethsift_image pyramid[], uint32_t pyramid_count){
  return 0;
}

// int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, struct ethsift_image output){
//   return 0;
// }

// int ethsift_downscale_linear(struct ethsift_image image, struct ethsift_image output){
//   return 0;
// }

int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image differences[], uint32_t layers){
  return 0;
}

int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t layers){
  return 0;
}

int ethsift_compute_orientation_histogram(struct ethsift_image gradient, struct ethsift_image rotation, struct ethsift_keypoint *keypoint, float *histogram){
  return 0;
}

// int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
//   return 0;
// }

int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint *keypoint){
  return 0;
}

int ethsift_extract_descriptor(struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t keypoint_count){
  return 0;
}

int ethsift_compute_keypoints(struct ethsift_image image, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  return 0;
}

int ethsift_match_keypoints(struct ethsift_keypoint a[], uint32_t a_count, struct ethsift_keypoint b[], uint32_t b_count, struct ethsift_match matches[], uint32_t *match_count){
  return 0;
}
