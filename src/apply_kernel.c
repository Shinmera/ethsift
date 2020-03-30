#include "internal.h"


// Apply Gaussian row filter to image and then transpose the image
int row_filter_transpose(float *pixels, float *output, int w, int h, float *kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  
  // ==========================================================================
  // TODO Work in progress
  //      - Remove any memory allocations
  //      - Rewrite to make it in place
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float *row_buf = malloc((w + kernel_rad * 2) * elemSize);

  float partialSum = 0.0f;
  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
    }

    dst_ind = r;
    buf_ind = 0;
    for (int c = 0; c < w; c++) {
      partialSum = 0.0f;       

      for (int i = 0; i < kernel_size; i++) {
        partialSum += kernel[i] * row_buf[buf_ind];
        ++buf_ind;
      }

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      dst_ind += h;
    }

    row_ind += w;
  }

  free(row_buf);

  return 1;
}


// Apply the gaussian kernel to the image and write the result to the output.
int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, struct ethsift_image output) {
  uint32_t w = image.width;
  uint32_t h = image.height;

  float* temp = (float*)malloc(w * h * sizeof(float));
  
  row_filter_transpose(image.pixels, temp, w, h, kernel, kernel_size, kernel_rad);
  row_filter_transpose(temp, output.pixels, h, w, kernel, kernel_size, kernel_rad);
  
  free(temp);

  return 1;
}