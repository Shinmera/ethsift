#include "count_flops.h"

// Number of defined tests
int test_count = 10;

// To save counts for the log file
size_t add_counts[1024];
size_t mult_counts[1024];
size_t mem_counts[1024];

// Input image
char * img = "lena.pgm";
struct ethsift_image input_img;

/// Add tests here
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
  return 1;
}

int test_gaussian_pyramid() {

  return 1;
}

int test_dog() {

  return 1;
}

int test_grad_rot() {

  return 1;
}

int test_histogram() {

  return 1;
}

int test_refinement() {

  return 1;
}

int test_keypoint_detection() {

  return 1;
}

int test_extract_descriptor() {

  return 1;
}

int test_compute_keypoints() {

  return 1;
}

/// End of tests

int init_tests() {
  tests = (test *)malloc(test_count * sizeof(test));
  // Add tests here
  tests[0] = (test){"CountDownscale", 1, test_downscale};
  tests[1] = (test){"CountConvolution", 1, test_convolution};
  tests[2] = (test){"CountGaussianPyramid", 1, test_gaussian_pyramid};
  tests[3] = (test){"CountDoG", 1, test_dog};
  tests[4] = (test){"CountGradRot", 1, test_grad_rot};
  tests[5] = (test){"CountHistogram", 1, test_histogram};
  tests[6] = (test){"CountRefinement", 1, test_refinement};
  tests[7] = (test){"CountKeypointDetection", 1, test_keypoint_detection};
  tests[8] = (test){"CountExtractDescriptor", 1, test_extract_descriptor};
  tests[9] = (test){"CountComputeKeypoints", 1, test_compute_keypoints};

  return 1;
}

int main(int argc, const char* argv[]){
  if (argc > 1) {
    img = (char *)argv[1];
  }
  fprintf(stderr, "Count flops and memory accesses using %s\n", img);
  char * path = data_file(img);
  fprintf(stderr, "Load image from %s\n", path);

  // Load input img from pgm file
  if (!read_pgm(path, &input_img)) {
    return 0;
  }
  fprintf(stderr, "Image with %d x %d pixels\n", input_img.width, input_img.height);

  init_tests();
  for (int i = 0; i < test_count; ++i) {
    if (tests[i].enabled) {
      run_test(i, tests[i]);
    }
  }
  
  return 0;
}