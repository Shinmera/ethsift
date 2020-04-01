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
    float x;      // Row
    float y;      // Column
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

  
  /// <summary> 
  /// Possibly necessary initialisation of the library.
  /// </summary>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_init();

  
  /// <summary> 
  /// Smartly allocate the image pyramid contents (allocate pixels, set sizes).
  /// </summary>
  /// <param name="pyramid"> OUT: The pyramid we want to allocate. </param>
  /// <param name="pyramid_count"> IN: Size of the pyramid (has to be known). </param>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_allocate_pyramid(struct ethsift_image pyramid[], uint32_t pyramid_count);

  /// <summary> 
  /// Creates a gaussian kernel for image filtering.
  /// </summary>
  /// <param name="kernel"> OUT: Kernel to generate. </param>
  /// <param name="kernel_size"> IN: Kernel size. </param>
  /// <param name="kernerl_rad"> IN: Kernel radius. </param>
  /// <param name="sigma"> IN: Standard deviation of the gaussian kernel. </param> 
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_gaussian_kernel(float *kernel, int kernel_size, int kernerl_rad, float sigma);

  /// <summary> 
  /// Creates a gaussian kernel for image filtering.
  /// </summary>
  /// <param name="layers_count"> IN: Amount of layers. </param>
  /// <param name="gaussian_count"> IN: Amount of gaussian kernels to generate. </param>
  /// <param name="kernel_ptrs"> OUT: Pointers to the kernels </param>
  /// <param name="kernel_rads"> OUT: The radii of all the kernels stored in an array. </param> 
  /// <param name="kernel_sizes"> OUT: The sizes of all the kernels stored in an array. </param> 
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_all_kernels(int layers_count, uint32_t gaussian_count, float **kernels_ptrs, int kernel_rads[], int kernel_sizes[]);

  /// <summary> 
  /// Frees up the allocated memory of the kernels
  /// </summary>
  /// <param name="kernel_ptrs"> IN: Pointers to the kernels for freeing up</param>
  /// <param name="gaussian_count"> IN: Amount of gaussian kernels. </param>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_free_kernels(float** kernel_ptrs, uint32_t gaussian_count);

  
  /// <summary> 
  /// Apply the gaussian kernel to the image and write the result to the output.
  /// </summary>
  /// <param name="image"> IN: Input image to blur. </param>
  /// <param name="kernel"> IN: The gaussian kernel/filter we use for blurring. </param>
  /// <param name="kernel_size"> IN: Size of gaussian kernels. </param>
  /// <param name="kernel_rad"> IN: Radius of the kernel. </param>
  /// <param name="output"> OUT: Blurred output image. </param>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, struct ethsift_image output);

  /// <summary> 
  /// Downscale the image by half and write the result to the output.
  /// </summary>
  /// <param name="image"> IN: Image to downscale. </param>
  /// <param name="output"> OUT: Downscaled image. </param>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_downscale_half(struct ethsift_image image, struct ethsift_image output);

  /// <summary> 
  /// Fill the octave pyramid by downsampling repeatedly.
  /// </summary>
  /// <param name="image"> IN: The original image we want to process. </param>
  /// <param name="octaves"> OUT: Octaves to generate. </param>
  /// <param name="kernerl_rad"> IN: Octaves image array size. </param>
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_octaves(struct ethsift_image image, struct ethsift_image octaves[], uint32_t octave_count);

  /// <summary> 
  /// Creates a pyramid of images containing blurred versions of the input image.
  /// </summary>
  /// <param name="octaves"> IN: The octaves of the input image. </param>
  /// <param name="octave_count"> IN: Number of octaves. </param>
  /// <param name="gaussians"> IN/OUT: Struct of gaussians to compute. 
  /// NOTE: Size = octave_count * gaussian_count. </param>
  /// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param> 
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_gaussian_pyramid(struct ethsift_image octaves[], uint32_t octave_count, struct ethsift_image gaussians[], uint32_t gaussian_count);

  /// <summary> 
  /// Build the gradient and rotation pyramids
  /// NOTE: Size of Pyramids = octave_count * gaussian_count with empty entries!
  /// </summary>
  /// <param name="gaussians"> IN: The octaves of the input image. </param>
  /// <param name="gaussian_count"> IN: Number of octaves. </param>
  /// <param name="gradients"> OUT: Struct of gradients to compute.  </param>
  /// <param name="rotations"> OUT: Struct of rotations to compute.  </param>
  /// <param name="layers"> IN: Number of layers in the gradients and rotation pyramids.  </param>
  /// <param name="octave_count"> IN: Number of octaves.  </param>
  /// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param> 
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image differences[], uint32_t layers, uint32_t octave_count);

  /// <summary> 
  /// Build the gradient and rotation pyramids
  /// NOTE: Size of Pyramids = octave_count * gaussian_count with empty entries!
  /// </summary>
  /// <param name="gaussians"> IN: The octaves of the input image. </param>
  /// <param name="gaussian_count"> IN: Number of octaves. </param>
  /// <param name="gradients"> OUT: Struct of gradients to compute.  </param>
  /// <param name="rotations"> OUT: Struct of rotations to compute.  </param>
  /// <param name="layers"> IN: Number of layers in the gradients and rotation pyramids.  </param>
  /// <param name="octave_count"> IN: Number of octaves.  </param>
  /// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param> 
  /// <returns> 1 IF generation was successful, ELSE 0. </returns>
  int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], uint32_t gaussian_count, struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t layers, uint32_t octave_count);

  /// <summary> 
  /// Compute the histogram for the given keypoints in the image.
  /// </summary>
  /// <param name="gradient"> IN: DOG pyramid. </param>
  /// <param name="rotation"> IN: Rotations?. </param>
  /// <param name="keypoint"> IN: Detected Keypoints.  </param>
  /// <param name="histogram"> OUT: Histogram of the detected keypoints. </param> 
  /// <param name="max_histval"> OUT: Maximum value in the histogram. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_compute_orientation_histogram(struct ethsift_image gradient, struct ethsift_image rotation, struct ethsift_keypoint *keypoint, float *histogram, float *max_histval);

  
  /// <summary> 
  /// Detect the keypoints in the image that SIFT finds interesting.
  /// </summary>
  /// <param name="differences"> IN: DOG pyramid. </param>
  /// <param name="gradients"> IN: Gradients pyramid. </param>
  /// <param name="rotations"> IN: Rotation pyramid.  </param>
  /// <param name="octaves"> IN: Number of Octaves. </param> 
  /// <param name="layers"> IN: Number of layers. </param> 
  /// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
  /// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
  ///                               OUT: Number of keypoints found. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count);

  


  /// <summary> 
  /// Refine the location of the keypoints to be sub-pixel accurate.
  /// </summary>
  /// <param name="differences"> IN: DOG pyramid. </param>
  /// <param name="octaves"> IN: Number of Octaves. </param> 
  /// <param name="layers"> IN: Number of layers. </param> 
  /// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint *keypoint);

  

  /// <summary> 
  /// Extract the keypoint descriptors.
  /// </summary>
  /// <param name="gradients"> IN: Gradients pyramid. </param>
  /// <param name="rotations"> IN: Rotation pyramid.  </param>
  /// <param name="octave_count"> IN: Number of Octaves. </param> 
  /// <param name="gaussian_count"> IN: Number of gaussian layers. </param> 
  /// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
  /// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
  ///                               OUT: Number of keypoints found. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_extract_descriptor(struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint keypoints[], uint32_t keypoint_count);

  
  /// <summary> 
  /// Perform SIFT and compute all known keypoints.
  /// </summary>
  /// <param name="image"> IN: Image to compute the SIFT descriptors of. </param>
  /// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
  /// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
  ///                               OUT: Number of keypoints found. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_compute_keypoints(struct ethsift_image image, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count);


  /// <summary> 
  /// Match up the common keypoints between two sets.
  /// </summary>
  /// <param name="a"> IN: Keypoints of the first image to match. </param>
  /// <param name="a_count"> IN: Number of keypoints in first image.  </param>
  /// <param name="b"> IN: Keypoints of the second image to match. </param> 
  /// <param name="b_count"> IN:  Number of keypoints in second image. </param> 
  /// <param name="matches"> OUT: Matched keypoints found. </param> 
  /// <param name="match_count"> IN: How many matches we can store at most (allocated size of memory).
  ///                            OUT: Number of matches found. </param> 
  /// <returns> 1 IF computation was successful, ELSE 0. </returns>
  int ethsift_match_keypoints(struct ethsift_keypoint a[], uint32_t a_count, struct ethsift_keypoint b[], uint32_t b_count, struct ethsift_match matches[], uint32_t *match_count);
 
#ifdef __cplusplus
}
#endif 
#endif
