#include "tester.h"

struct test{
  const char *title;
  int (*func)();
};

std::chrono::time_point<std::chrono::high_resolution_clock> start;
size_t duration = 0;
bool measurement_pending = false;
int test_count = 0;
struct test tests[1024] = {0};

int register_test(const char *title, int (*func)()){
  tests[test_count].title = title;
  tests[test_count].func = func;
  return ++test_count;
}

struct ethsift_image allocate_image(uint32_t width, uint32_t height){
  struct ethsift_image output = {0};
  output.width = width;
  output.height = height;
  output.pixels = (float*) calloc(sizeof(float), width*height);
  return output;
}

struct ethsift_keypoint convert_keypoint(ezsift::SiftKeypoint *k) {
    struct ethsift_keypoint ret = {0};

    ret.layer = k->layer;
    ret.magnitude = k->mag;
    ret.octave = k->octave;
    ret.global_pos.scale = k->scale;
    ret.global_pos.x = k->r;
    ret.global_pos.y = k->c;

    ret.layer_pos.scale = k->layer_scale;
    ret.layer_pos.x = k->ri;
    ret.layer_pos.y = k->ci;

    for (int i = 0; i < DESCRIPTORS; ++i) {
        ret.descriptors[i] = k->descriptors[i];
    }

    return ret;
}

int convert_image(const ezsift::Image<unsigned char> &input,
                  struct ethsift_image *output){
  output->width = input.w;
  output->height = input.h;
  output->pixels = (float*)calloc(sizeof(float), input.w*input.h);
  if(output->pixels == 0) return 0;
  for(int y=0; y<input.h; ++y){
    for(int x=0; x<input.w; ++x){
      size_t idx = y*input.w+x;
      output->pixels[idx] = (float) input.data[idx];
    }
  }
  return 1;
}

int convert_image(const ezsift::Image<float> &input,
                  struct ethsift_image *output){
  output->width = input.w;
  output->height = input.h;
  output->pixels = (float*)calloc(sizeof(float), input.w*input.h);
  if(output->pixels == 0) return 0;
  for(int y=0; y<input.h; ++y){
    for(int x=0; x<input.w; ++x){
      size_t idx = y*input.w+x;
      output->pixels[idx] = (float) input.data[idx];
    }
  }
  return 1;
}

int load_image(const char *file, struct ethsift_image &image){
  ezsift::Image<unsigned char> img;
  if(img.read_pgm(file) != 0) return 0;
  if(!convert_image(img, &image)) return 0;
  return 1;
}

int compare_image(const ezsift::Image<unsigned char> &ez_img,
                  struct ethsift_image &eth_img){
               
  struct ethsift_image conv_ez_img = {0};     
  convert_image(ez_img, &conv_ez_img);
  return compare_image(conv_ez_img, eth_img);
}

int compare_image(struct ethsift_image a, struct ethsift_image b){
  return (a.width == b.width)
    && (a.height == b.height)
    && (memcmp(a.pixels, b.pixels, a.width*a.height*sizeof(float)) == 0);
}

int compare_image_approx(const ezsift::Image<unsigned char> &ez_img,
                  struct ethsift_image &eth_img){
               
  struct ethsift_image conv_ez_img = {0};     
  convert_image(ez_img, &conv_ez_img);
  return compare_image_approx(conv_ez_img, eth_img, EPS);
}

int compare_image_approx(const ezsift::Image<float> &ez_img,
                  struct ethsift_image &eth_img){
               
  struct ethsift_image conv_ez_img = {0};     
  convert_image(ez_img, &conv_ez_img);
  return compare_image_approx(conv_ez_img, eth_img, EPS);
}


int compare_image_approx(struct ethsift_image a, struct ethsift_image b, float eps){
  if(a.width != b.width){
    printf("COMPARE_IMAGE_APPROX WIDTH: a.w = %d ; b.w = %d\n", a.width, b.width);
    return 0;
  }
  if(a.height != b.height){
    printf("COMPARE_IMAGE_APPROX HEIGHT: a.h = %d ; b.h = %d\n", a.height, b.height);
    return 0;
  }
  for(size_t i=0; i<a.width*a.height; ++i){
    float diff = a.pixels[i] - b.pixels[i];
    if(diff < 0.0) diff *= -1;
    if(eps < diff) {
      printf("COMPARE_IMAGE_APPROX PIXEL: index = %d ; val a = %f ; val b = %f\n", (int)i, a.pixels[i], b.pixels[i]);
      return 0;
    }
  }
  return 1;
}

int compare_kernel(std::vector<float> ez_kernel, float* eth_kernel, int eth_kernel_size){
  if((int)ez_kernel.size() != eth_kernel_size)
  {
    printf("Kernel sizes do not match %d != %d", (int)ez_kernel.size(), eth_kernel_size);
    return 0;
  }
  for(int i = 0; i < eth_kernel_size; ++i){
    float diff = ez_kernel[i] - eth_kernel[i];
    if(diff < 0.0) diff *= -1;
    if(EPS < diff) {
      printf("COMPARE_KERNEL: index = %d ; val a = %f ; val b = %f\n", i, ez_kernel[i], eth_kernel[i]);
      return 0;
    }
  }
  return 1;
}

int write_image(struct ethsift_image image){

    unsigned char* pixels_to_write = (unsigned char *)malloc( image.width * image.height *sizeof(unsigned char));
    for (int i = 0; i < image.height; ++i) {
      for (int j = 0; j < image.width; ++j) {
        pixels_to_write[i * image.width + j] = (unsigned char) (image.pixels[i * image.width + j]);
      }
    }
    ezsift::write_pgm("eth.pgm", pixels_to_write, image.width, image.height );
}


void fail(const char *message){
  fprintf(stderr, "\033[1;31m[ERROR]\033[0;0m %s\n", message);
  exit(1);
}

int run_test(struct test test){
  fprintf(stderr, "Running %-32s \033[0;90m...\033[0;0m ", test.title);
  duration = 0;
  // Start measurement without the pending check to allow user override
  start = std::chrono::high_resolution_clock::now();
  // Run the check.
  int ret = test.func();
  // If we had no other measurement so far, force our start by setting the pending now.
  if(duration == 0) measurement_pending = true;
  // End any possible measurement that might still be going on now.
  // We have to do this here in order to allow correct measurement even in the face
  // of early returns from within measurement blocks. We do not have an unwind-protect
  // operator in C after all.
  end_measurement();
  fprintf(stderr, " %10liµs ", duration);
  fprintf(stderr, (ret==0)?"\033[1;31m[FAIL]":"\033[0;32m[OK  ]");
  fprintf(stderr, "\033[0;0m\n");
  return ret;
}

int run_tests(struct test *tests, uint32_t count){
  const char *failed[count];
  int failures = 0;
  int passes = 0;

  fprintf(stderr, "\033[1;33m --> \033[0;0mRunning %i tests\n", count);
  for(uint32_t i=0; i<count; ++i){
    if(run_test(tests[i])){
      passes++;
    }else{
      failed[failures] = tests[i].title;
      failures++;
    }
  }
  fprintf(stderr, "\nPassed: %3i", passes);
  fprintf(stderr, "\nFailed: %3i\n", failures);
  if(failures){
    fprintf(stderr, "\033[1;33m --> \033[0;0mThe following tests failed:\n");
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
    return (run_tests(tests, test_count) == 0)? 1 : 0;
  }else{
    char *file1 = (1 < argc)? argv[1] : 0;
    char *file2 = (2 < argc)? argv[2] : 0;
    complete_run(file1, file2);
  }
  return 0;
}
