#include "tester.h"

struct test{
  const char *title;
  const char *reason;
  int has_measurement_comp;
  int (*func)();
};

std::chrono::time_point<std::chrono::high_resolution_clock> start;
std::vector<size_t> durations;
std::vector<LogTuple> test_logs;
bool measurement_pending = false;
int test_count = 0;
struct test tests[1024] = {0};

int register_test(const char *title, int has_measurement_comp, int (*func)()){
  tests[test_count].title = title;
  tests[test_count].reason = 0;
  tests[test_count].has_measurement_comp = has_measurement_comp;
  tests[test_count].func = func;
  return test_count++;
}

static int _debugger_present = -1;
static void _sigtrap_handler(int signum){
  _debugger_present = 0;
  signal(SIGTRAP, SIG_DFL);
}

void debug_break(void){
  if (-1 == _debugger_present) {
    _debugger_present = 1;
    signal(SIGTRAP, _sigtrap_handler);
    raise(SIGTRAP);
  }
}

int register_failure(int test, const char *reason){
  if(test_count <= test) return 0;
  tests[test].reason = reason;
  debug_break();
  return 1;
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
    ret.orientation = k->ori;
    ret.octave = k->octave;
    ret.global_pos.scale = k->scale;
    ret.global_pos.y = k->r;
    ret.global_pos.x = k->c;

    ret.layer_pos.scale = k->layer_scale;
    ret.layer_pos.y = k->ri;
    ret.layer_pos.x = k->ci;

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

// Compare an ezsift kernel with an ethsift kernel for correctness
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

// Compare an ezsift descriptor with an ethsift descriptor for correctness
int compare_descriptor(float* ez_descriptors, float* eth_descriptors) {
  for (int i = 0; i < (int) DESCRIPTORS; ++i) {
    float diff = ez_descriptors[i] - eth_descriptors[i];
    if (EPS < abs(diff)) {
      printf("COMPARE_DESCRIPTORS: index = %d ; val a = %f ; val b = %f\n", i, ez_descriptors[i], eth_descriptors[i]);
      return 0;
    }
  }
  return 1;
}

int write_image(struct ethsift_image image, const char* filename){

    unsigned char* pixels_to_write = (unsigned char *)malloc( image.width * image.height *sizeof(unsigned char));
    for (int i = 0; i < (int) image.height; ++i) {
      for (int j = 0; j < (int) image.width; ++j) {
        pixels_to_write[i * image.width + j] = (unsigned char) (image.pixels[i * image.width + j]);
      }
    }
    ezsift::write_pgm(filename, pixels_to_write, (int) image.width, (int) image.height);
    return 1;
}

void abort(const char *message){
  fprintf(stderr, "\033[1;31m[ERROR]\033[0;0m %s\n", message);
  exit(1);
}

template<typename T, typename Func>
T reduce(std::vector<T> &arg, Func func){
  switch(arg.size()){
  case 0: return 0.0;
  case 1: return arg[0];
  default: {
    T result = arg[0];
    for(size_t i=1; i<arg.size(); ++i)
      result = func(result, arg[1]);
    return result;
  }
  }
}

template<typename U, typename V, typename Func>
void map(std::vector<U> &in, std::vector<V> &out, Func func){
  for(size_t i=0; i<in.size() && i<out.size(); ++i)
    out[i] = func(in[i]);
}

void write_logfile() {
    time_t curr_time;
    tm* curr_tm;
    char date_string[100];
    char time_string[100];

    time(&curr_time);
    curr_tm = localtime(&curr_time);

    strftime(date_string, 50, "%B_%d_%Y", curr_tm);
    strftime(time_string, 50, "_%T", curr_tm);
    char filename[100] = "../logs/logfile_";
    strcat(filename, date_string);
    strcat(filename, time_string);
    strcat(filename, ".csv");

    std::ofstream myfile;
    myfile.open(filename);
    for (auto t : test_logs) {
        myfile << std::get<0>(t) << ", " << std::get<1>(t)
            << ", " << std::get<2>(t) << ", " << std::get<3>(t) << std::endl;
    }
    myfile.close();
}

int run_test(struct test test){
  fprintf(stderr, "Running %-32s \033[0;90m...\033[0;0m ", test.title);
  durations.clear();
  // Start measurement without the pending check to allow user override
  start = std::chrono::high_resolution_clock::now();
  // Run the check.
  int ret = test.func();
  // If we had no other measurement so far, force our start by setting the pending now.
  if(durations.size() == 0) measurement_pending = true;
  // End any possible measurement that might still be going on now.
  // We have to do this here in order to allow correct measurement even in the face
  // of early returns from within measurement blocks. We do not have an unwind-protect
  // operator in C after all.
  end_measurement();
  std::vector<size_t> durations_ethsift;
  if (test.has_measurement_comp) {
      // If we are doing runtime comparison with the ezsift library, split the measurements into two, since second half were ezSIFT measurements.
      std::size_t const half_size = durations.size() / 2;
      durations_ethsift = std::vector<size_t>(durations.begin(), durations.begin() + half_size);
      std::vector<size_t> durations_ezsift(durations.begin() + half_size, durations.end()-1);

      // Compute statistics for ezsift
      size_t cumulative_ezsift = reduce(durations_ezsift, [](auto a, auto b) {return a + b; });
      double average_ezsift = ((double)cumulative_ezsift) / durations_ezsift.size();
      std::vector<double> variances_ezsift(durations_ezsift.size());
      map(durations_ezsift, variances_ezsift, [&](auto d) {return (d - average_ezsift) * (d - average_ezsift); });
      double stddev_ezsift = reduce(variances_ezsift, [](auto a, auto b) {return a + b; }) / durations_ezsift.size();
      std::sort(durations_ezsift.begin(), durations_ezsift.end());
      size_t median_ezsift = durations_ezsift[durations_ezsift.size() / 2];
      LogTuple t = { test.title, "ezSIFT", median_ezsift, stddev_ezsift };
      test_logs.push_back(t);
  }
  else {
      // If we are not doing any runtime comparisons, continue as before.
      durations_ethsift = durations;
  }

  // Compute statistics for ethsift
  size_t cumulative_ethsift = reduce(durations_ethsift, [](auto a,auto b){return a+b;});
  double average_ethsift = ((double)cumulative_ethsift) / durations_ethsift.size();
  std::vector<double> variances_ethsift(durations_ethsift.size());
  map(durations_ethsift, variances_ethsift, [&](auto d){return (d- average_ethsift)*(d- average_ethsift);});
  double stddev_ethsift = reduce(variances_ethsift, [](auto a,auto b){return a+b;}) / durations_ethsift.size();
  std::sort(durations_ethsift.begin(), durations_ethsift.end());
  size_t median_ethsfit = durations_ethsift[durations_ethsift.size()/2];
  LogTuple t = { test.title, "ETHSIFT", median_ethsfit, stddev_ethsift };
  test_logs.push_back(t);


  // Show
  fprintf(stderr, " %10liµs ±%3.3f", median_ethsfit, stddev_ethsift);
  fprintf(stderr, (ret==0)?"\033[1;31m[FAIL]":"\033[0;32m[OK  ]");
  fprintf(stderr, "\033[0;0m\n");
  return ret;
}

int run_tests(struct test *tests, uint32_t count){
  int failed[count];
  int failures = 0;
  int passes = 0;

  fprintf(stderr, "\033[1;33m --> \033[0;0mRunning %i tests\n", count);
  for(uint32_t i=0; i<count; ++i){
    if(run_test(tests[i])){
      passes++;
    }else{
      failed[failures] = i;
      failures++;
    }
  }
  write_logfile();

  fprintf(stderr, "\nPassed: %3i", passes);
  fprintf(stderr, "\nFailed: %3i\n", failures);
  if(failures){
    fprintf(stderr, "\033[1;33m --> \033[0;0mThe following tests failed:\n");
    for(int i=0; i<failures; ++i){
      struct test test = tests[failed[i]];
      fprintf(stderr, "%s: %s\n", test.title, test.reason);
    }
  }
  return (failures == 0);
}


void compute_keypoints(char *file, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  ezsift::Image<unsigned char> img;
  if(img.read_pgm(file) != 0)
    abort("Failed to read image!");

  struct ethsift_image image = {0};
  if(!convert_image(img, &image))
    abort("Could not convert image");

  if(!ethsift_compute_keypoints(image, keypoints, keypoint_count))
    abort("Failed to compute keypoints");
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
      abort("Failed to match up keypoints.");

    // TODO: Show matches
  }else{
    // TODO: Show keypoints
  }
}

int main(int argc, char *argv[]){
  if(!ethsift_init())
    abort("Failed to initialise ETHSIFT");
  srand(time(NULL));

  if(argc <= 1){
    return (run_tests(tests, test_count) == 0)? 1 : 0;
  }else{
    char *file1 = (1 < argc)? argv[1] : 0;
    char *file2 = (2 < argc)? argv[2] : 0;
    complete_run(file1, file2);
  }
  return 0;
}
