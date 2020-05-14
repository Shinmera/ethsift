#include "count_flops.h"

// Number of defined tests
int test_count;

// To save counts for the log file
size_t add_counts[1024];
size_t mult_counts[1024];
size_t mem_counts[1024];
size_t div_counts[1024];

// Input image
char * img = "lena.pgm";
struct ethsift_image input_img;

/// Helper functions and fields
struct ethsift_image eth_octaves[OCTAVE_COUNT];
struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];

int init_gaussian() {
  ethsift_allocate_pyramid(eth_octaves, input_img.width, input_img.height, OCTAVE_COUNT, 1);
  ethsift_allocate_pyramid(eth_gaussians, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
  
  // Generate gaussian pyramid for input picture
  ethsift_generate_octaves(input_img, eth_octaves, OCTAVE_COUNT);
  ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);
}


////////////////////////////////////  Add tests here  /////////////////////////////////////// 
int test_downscale() {
  int dstW = input_img.width >> 1;
  int dstH = input_img.height >> 1;
  struct ethsift_image out;
  out.height = input_img.height >> 1;
  out.width = input_img.width >> 1;
  out.pixels = (float*)malloc(sizeof(float) * out.height * out.width);
  ethsift_downscale_half(input_img, out);
  return 1;
}

int test_convolution() {
  int kernel_size = 9;
  int kernel_rad = 4;
  float sigma = 4.5;
  
  // Create kernel
  float *kernel = (float*) malloc(kernel_size * sizeof(float)); 
  ethsift_generate_gaussian_kernel(kernel, kernel_size, kernel_rad, sigma);

  struct ethsift_image out;
  out.height = input_img.height;
  out.width = input_img.width;
  out.pixels = (float*)malloc(sizeof(float) * out.height * out.width);

  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_apply_kernel(input_img, kernel, kernel_size, kernel_rad, out);

  free(kernel);

  return 1;
}

int test_gaussian_pyramid() {
  // Allocate the pyramids!
  struct ethsift_image eth_octaves[OCTAVE_COUNT];
  ethsift_allocate_pyramid(eth_octaves, input_img.width, input_img.height, OCTAVE_COUNT, 1);

  struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gaussians, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  //Create Octaves for ethSift    
  ethsift_generate_octaves(input_img, eth_octaves, OCTAVE_COUNT);
  // Create gaussians for ethSift
  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);
  
  ethsift_free_pyramid(eth_octaves);
  ethsift_free_pyramid(eth_gaussians);
  
  return 1;
}

int test_dog() {
  // Allocate the pyramids!  
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
  ethsift_allocate_pyramid(eth_differences, input_img.width, input_img.height, OCTAVE_COUNT, DOG_COUNT);
  
  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
  
  ethsift_free_pyramid(eth_differences);
  
  return 1;
}

int test_grad_rot() {
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_rotations, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
  
  ethsift_free_pyramid(eth_rotations);
  ethsift_free_pyramid(eth_gradients);
  
  return 1;
}

int test_histogram() {
  // Allocate the pyramids!  
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
  ethsift_allocate_pyramid(eth_differences, input_img.width, input_img.height, OCTAVE_COUNT, DOG_COUNT);
  
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_rotations, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
  ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);

  struct ethsift_keypoint eth_kpt_list[100];
  uint32_t nKeypoints = 100;
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);

  size_t adds_temp = 0;
  size_t mults_temp = 0;
  size_t mem_temp = 0;
  size_t div_temp = 0;

  int lim = (nKeypoints < 100)? nKeypoints : 100;
  float eth_hist[ETHSIFT_ORI_HIST_BINS];
  for (int i = 0; i < lim; ++i) {
    struct ethsift_keypoint kpt = eth_kpt_list[i];
    float max_mag = 0.f;
    
    #ifdef IS_COUNTING
    reset_counters();
    #endif
    ethsift_compute_orientation_histogram(eth_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                          eth_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                          &kpt, eth_hist, &max_mag);

    #ifdef IS_COUNTING
    adds_temp += add_count;
    mults_temp += mult_count;
    mem_temp += mem_count;
    div_temp += div_count;
    #endif
  }

  // Average the counts
  #ifdef IS_COUNTING
  add_count = (size_t) floor(adds_temp / lim);
  mult_count = (size_t) floor(mults_temp / lim);
  mem_count = (size_t) floor(mem_temp / lim);
  div_count = (size_t) floor(div_temp / lim);
  #endif

  ethsift_free_pyramid(eth_differences);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);

  return 1;
}

int test_refinement() {
  // Allocate the pyramids!  
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
  ethsift_allocate_pyramid(eth_differences, input_img.width, input_img.height, OCTAVE_COUNT, DOG_COUNT);
  
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_rotations, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
  ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);

  struct ethsift_keypoint eth_kpt_list[100];
  uint32_t nKeypoints = 100;
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);

  size_t adds_temp = 0;
  size_t mults_temp = 0;
  size_t mem_temp = 0;
  size_t div_temp = 0;

  int lim = (nKeypoints < 100)? nKeypoints : 100;
  for (int i = 0; i < lim; ++i) {
    struct ethsift_keypoint kpt = eth_kpt_list[i];

    #ifdef IS_COUNTING
    reset_counters();
    #endif
    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &kpt);
    
    #ifdef IS_COUNTING
    adds_temp += add_count;
    mults_temp += mult_count;
    mem_temp += mem_count;
    div_temp += div_count;
    #endif
  }

  // Average the counts
  #ifdef IS_COUNTING
  add_count = (size_t) floor(adds_temp / lim);
  mult_count = (size_t) floor(mults_temp / lim);
  mem_count = (size_t) floor(mem_temp / lim);
  div_count = (size_t) floor(div_temp / lim);
  #endif
  
  ethsift_free_pyramid(eth_differences);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);
  
  return 1;
}

int test_keypoint_detection() {
// Allocate the pyramids!  
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
  ethsift_allocate_pyramid(eth_differences, input_img.width, input_img.height, OCTAVE_COUNT, DOG_COUNT);
  
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_rotations, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
  ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);

  struct ethsift_keypoint eth_kpt_list[100];
  uint32_t nKeypoints = 100;

  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);
  
  ethsift_free_pyramid(eth_differences);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);
  
  return 1;
}

int test_extract_descriptor() {
  // Allocate the pyramids!  
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
  ethsift_allocate_pyramid(eth_differences, input_img.width, input_img.height, OCTAVE_COUNT, DOG_COUNT);
  
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_rotations, input_img.width, input_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
  ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);

  struct ethsift_keypoint eth_kpt_list[100];
  uint32_t nKeypoints = 100;
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);
  
  nKeypoints = (nKeypoints < 100) ? nKeypoints : 100;
  #ifdef IS_COUNTING
  reset_counters();
  #endif
  ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, nKeypoints);
  
  ethsift_free_pyramid(eth_differences);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);

  return 1;
}

int test_compute_keypoints() {  
  uint32_t nKeypoints = 1000;
  struct ethsift_keypoint eth_kpt_list[1000];
  ethsift_compute_keypoints(input_img, eth_kpt_list, &nKeypoints);
  return 1;
}
////////////////////////////////////  End of tests  ///////////////////////////////////////

int init_tests() {
  // Specify number of tests
  test_count = 9;

  tests = (test *)malloc(test_count * sizeof(test));
  // Add tests here
  tests[0] = (test){"Downscale", 1, test_downscale};
  tests[1] = (test){"Convolution", 1, test_convolution};
  tests[2] = (test){"GaussianPyramid", 1, test_gaussian_pyramid};
  tests[3] = (test){"DOGPyramid", 1, test_dog};
  tests[4] = (test){"GradientAndRotationPyramids", 1, test_grad_rot};
  tests[5] = (test){"Histogram", 1, test_histogram};
  tests[6] = (test){"ExtremaRefinement", 1, test_refinement};
  tests[7] = (test){"KeypointDetection", 1, test_keypoint_detection};
  tests[8] = (test){"ExtractDescriptor", 1, test_extract_descriptor};
  //tests[9] = (test){"CountSIFTComplete1000", 1, test_compute_keypoints};

  return 1;
}

int main(int argc, const char* argv[]){
  if (argc > 1) {
    img = (char *)argv[1];
  }
  fprintf(stderr, "Count flops and memory accesses using %s\n", img);

  // Load input img from pgm file
  if (!read_pgm(img, &input_img)) {
    return 0;
  }
  fprintf(stderr, "Image with %d x %d pixels\n", input_img.width, input_img.height);

  // Precompute gaussian pyramid
  init_gaussian();

  init_tests();
  for (int i = 0; i < test_count; ++i) {
    if (tests[i].enabled) {
      run_test(i, tests[i]);
    }
  }
  
  // Free gaussian and octaves
  ethsift_free_pyramid(eth_gaussians);
  ethsift_free_pyramid(eth_octaves);

  write_log();
  return 0;
}
