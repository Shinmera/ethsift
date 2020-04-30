#include "tester.h"

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

int load_image(const char *file, struct ethsift_image &image, ezsift::Image<unsigned char> *out){
  ezsift::Image<unsigned char> img;
  if(img.read_pgm(file) != 0) return 0;
  if(!convert_image(img, &image)) return 0;
  if(out != 0){
    out->init(img.w, img.h);
    memcpy(out->data, img.data, img.w * img.h * sizeof(unsigned char));
  }
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

int build_ezsift_pyramids(ezsift::Image<unsigned char> ez_img, std::vector<ezsift::Image<float>> &ez_differences, std::vector<ezsift::Image<float>> &ez_gradients, std::vector<ezsift::Image<float>> &ez_rotations){
  std::vector<ezsift::Image<unsigned char>> ez_octaves(OCTAVE_COUNT);
  build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

  std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

  ez_differences.resize(OCTAVE_COUNT * DOG_COUNT);
  build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

  ez_gradients.resize(OCTAVE_COUNT * GAUSSIAN_COUNT);
  ez_rotations.resize(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);
  return 1;
}
