#ifndef ETHSIFT_H
#define ETHSIFT_H 1
#include <stdint.h>

#define DESCRIPTORS 128

#ifdef __cplusplus
extern "C" {
#endif 
  
  struct ethsift_image{
    float *pixels;
    uint32_t width;
    uint32_t height;
  };

  struct ethsift_coordinate{
    float x;
    float y;
    float scale;
  };

  struct ethsift_keypoint{
    uint32_t octave;
    uint32_t layer;
    struct ethsift_coordinate global_pos;
    struct ethsift_coordinate layer_pos;
    float orientation;
    float magnitude;
    float descriptors[DESCRIPTORS];
  };

  struct ethsift_match{
    uint32_t x1, y1, x2, y2;
  };

  //// General notes:
  // All API functions return a result indicator that should be
  // 0 on failure and greater than zero on success.

  // Possibly necessary initialisation of the library.
  int ethsift_init();

  // Smartly allocate the image pyramid contents (allocate pixels, set sizes).
  int ethsift_allocate_pyramid(struct ethsift_image pyramid[], uint32_t pyramid_count);

  // Compute the gaussian kernel weights of a given size.
  int ethsift_generate_gaussian_kernel(float *kernel, int kernel_size, int kernerl_rad, float sigma);

  int ethsift_generate_all_kernels(int layers_count, uint32_t gaussian_count, float **kernels_ptrs, int kernel_rads[], int kernel_sizes[]);
  
  int ethsift_free_kernels(float** kernel_ptrs, uint32_t gaussian_count);

  // Apply the gaussian kernel to the image and write the result to the output.
  int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, struct ethsift_image output);

  // Downscale the image by half and write the result to the output.
  int ethsift_downscale_half(struct ethsift_image image, struct ethsift_image output);

  // Fill the octave pyramid by downsampling repeatedly.
  int ethsift_generate_octaves(struct ethsift_image image, struct ethsift_image octaves[], uint32_t octave_count);

  // Fill the gaussian pyramid by applying the gaussian blur.
  int ethsift_generate_pyramid(struct ethsift_image octaves[], uint32_t octave_count, struct ethsift_image gaussians[], uint32_t gaussian_count);

  // Fill the difference of gaussians pyramid.
  int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image differences[], uint32_t layers);

  // Build the gradient and rotation pyramids
  int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t layers);

  // Compute the histogram for the given keypoint in the image
  int ethsift_compute_orientation_histogram(struct ethsift_image gradient, struct ethsift_image rotation, struct ethsift_keypoint *keypoint, float *histogram, float *max_histval);

  // Detect the keypoints in the image that SIFT finds interesting.
  // keypoint_count
  //   [in] how many keypoints we can store at most
  //   [out] how many keypoints we actually found
  int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count);

  // Refine the location of the keypoints to be sub-pixel accurate.
  int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint *keypoint);

  // Extract the keypoint descriptors.
  int ethsift_extract_descriptor(struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t keypoint_count);

  // Perform SIFT and compute all known keypoints.
  // keypoint_count
  //   [in] how many keypoints we can store at most
  //   [out] how many keypoints we actually found
  int ethsift_compute_keypoints(struct ethsift_image image, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count);

  // Match up the common keypoints between two sets.
  // match_count
  //   [in] how many matches we can store at most
  //   [out] how many matches we actually found
  int ethsift_match_keypoints(struct ethsift_keypoint a[], uint32_t a_count, struct ethsift_keypoint b[], uint32_t b_count, struct ethsift_match matches[], uint32_t *match_count);
 
#ifdef __cplusplus
}
#endif 
#endif
