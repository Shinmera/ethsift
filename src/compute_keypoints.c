#include "internal.h"

/// <summary> 
/// Perform SIFT and compute all known keypoints.
/// </summary>
/// <param name="image"> IN: Image to compute the SIFT descriptors of. </param>
/// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
/// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
///                               OUT: Number of keypoints found. </param> 
/// <returns> 1 IF computation was successful, ELSE 0. </returns>
int ethsift_compute_keypoints(struct ethsift_image image, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count) {

  // Number of layers in one octave; same as s in the paper.
  const int layers = ETHSIFT_INTVLS;
  // Number of Gaussian images in one octave.
  const int gaussian_count = layers + 3;
  // Number of DoG images in one octave.
  const int dog_count = layers + 2;
  // Number of octaves according to the size of image.
  const int octave_count = (int)log2f((float)int_min((int) image.width, (int) image.height)) - 3; // 2 or 3, need further research


  // Allocate the pyramids!
 /* struct ethsift_image eth_octaves[octave_count];
  ethsift_allocate_pyramid(eth_octaves, image.width, image.height, octave_count, 1);*/

  struct ethsift_image eth_gaussians[octave_count * gaussian_count];
  ethsift_allocate_pyramid(eth_gaussians, image.width, image.height, octave_count, gaussian_count);

  struct ethsift_image eth_gradients[octave_count*gaussian_count];
  ethsift_allocate_pyramid(eth_gradients, image.width, image.height, octave_count, gaussian_count);

  struct ethsift_image eth_rotations[octave_count*gaussian_count];
  ethsift_allocate_pyramid(eth_rotations, image.width, image.height, octave_count, gaussian_count);

  struct ethsift_image eth_differences[octave_count*dog_count];
  ethsift_allocate_pyramid(eth_differences, image.width, image.height, octave_count, dog_count);


  //Create Octaves for ethSift    
  //ethsift_generate_octaves(image, eth_octaves, octave_count);

  //Create Gaussians for ethSift    
  ethsift_generate_gaussian_pyramid(image, octave_count, eth_gaussians, gaussian_count);

  // Caculate Difference of Gaussians
  ethsift_generate_difference_pyramid(eth_gaussians, gaussian_count, eth_differences, dog_count, octave_count);

  ethsift_generate_gradient_pyramid(eth_gaussians, gaussian_count, eth_gradients, eth_rotations, layers, octave_count);
  
  // Ethsift keypoint detection:
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, octave_count, gaussian_count, keypoints, keypoint_count);

  ethsift_extract_descriptor(eth_gradients, eth_rotations, octave_count, gaussian_count, keypoints, *keypoint_count);

  // Free up memory allocated for all the pyramids 
  //ethsift_free_pyramid(eth_octaves);
  ethsift_free_pyramid(eth_gaussians);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);
  ethsift_free_pyramid(eth_differences);

  return 1;
}