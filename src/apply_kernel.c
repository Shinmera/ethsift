#include "internal.h"


// Apply Gaussian row filter to image and then transpose the image
int row_filter_transpose(float *pixels, float *output, int w, int h, float *kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  
  // ==========================================================================
  // TODO Work in progress
  //      - Remove any memory allocations
  //      - Rewrite to make it in place
  int elemSize = sizeof(float);

  float *row_start;
  float *coef = kernel;
  float *prow;

  float *srcData = pixels;
  float *dstData = output + w * h - 1;
  float *row_buf = malloc((w + kernel_rad * 2) * elemSize);

  float partialSum = 0.0f;
  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    row_start = srcData + r * w;
    memcpy(row_buf + kernel_rad, row_start, elemSize * w);
    firstData = *(row_start);
    lastData = *(row_start + w - 1);
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
    }

    prow = row_buf;
    dstData = dstData - w * h + 1;
    for (int c = 0; c < w; c++) {
      partialSum = 0.0f;
      coef = kernel;

      for (int i = -kernel_rad; i <= kernel_rad; i++) {
        partialSum += (*coef++) * (*prow++);
      }

      prow -= 2 * kernel_rad;
      *dstData = partialSum;
      dstData += h;
    }
  }

  free(row_buf);

  return 1;
}


// Apply the gaussian kernel to the image and write the result to the output.
int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, struct ethsift_image output) {
  uint32_t w = image.width;
  uint32_t h = image.height;

  //output.height = h;
  //output.width = w;

  float *out = output.pixels;

  float* temp = (float*)malloc(w * h * sizeof(float));
  
  row_filter_transpose(image.pixels, temp, w, h, kernel, kernel_size, kernel_rad);
  row_filter_transpose(temp, out, w, h, kernel, kernel_size, kernel_rad);
  
  free(temp);

  return 1;
}