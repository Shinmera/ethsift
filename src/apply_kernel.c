#include "internal.h"


// Apply Gaussian row filter to image and then transpose the image
int row_filter_transpose(float *pixels, float *output, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, uint32_t w, uint32_t h) {
  // TODO
  float row_buf[w + kernel_rad * 2];
  

  return 1;
}


// Apply the gaussian kernel to the image and write the result to the output.
int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, struct ethsift_image output) {
  uint32_t kernel_rad = kernel_size / 2;
  uint32_t w = image.width;
  uint32_t h = image.height;

  output.height = w;
  output.width = h;
  
  float* temp = (float*)malloc(w * h * sizeof(float));
  row_filter_transpose(image.pixels, temp, kernel, kernel_size, kernel_rad, w, h);
  row_filter_transpose(temp, output.pixels, kernel, kernel_size, kernel_rad, h, w);
  
  return 1;
}