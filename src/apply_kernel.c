#include "internal.h"

/// <summary> 
/// Apply Gaussian row filter to image and then transpose the image.
/// </summary>
/// <param name="pixels"> IN: Pixels to filter. </param>
/// <param name="output"> OUT: Filtered image. </param>
/// <param name="w"> IN: Width of image to filter. </param>
/// <param name="h"> IN: Height of image to filter. </param>
/// <param name="kernel"> IN: Kernel to filter with. </param>
/// <param name="kernel_size"> IN: Size of the kernel. </param>
/// <param name="kernel_rad"> IN: Radius of the kernel. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
 /// <remarks> (h * w * (2* kernel_size)) flops </remarks>
int row_filter_transpose(float * restrict pixels, float * restrict output, int w, int h, float * restrict kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  
  // ==========================================================================
  // TODO Work in progress
  //      - Rewrite to make it in place
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float partialSum = 0.0f;
  float partialSum1 = 0.0f;
  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_mem(w); // memcpy 1 read / 1 write
    inc_mem(w);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_mem(2); // 2 reads
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_mem(2); // 2 writes
    }

    dst_ind = r;
    buf_ind = 0;

    // AMD avg of 101'586'060 cycles (lena.pgm)
    int lim = w - 1;
    int c;
    for (c = 0; c < lim; c += 2) {
      partialSum = 0;
      partialSum1 = 0;

      float t11 = 0;
      float t21 = 0; 
      float t31 = 0;
      float t41 = 0;

      float t12 = 0;
      float t22 = 0; 
      float t32 = 0;
      float t42 = 0;

      float val1, val2, val3, val4, val5;
      float ker1, ker2, ker3, ker4;

      int kernel_lim = kernel_size - 4;
      int i;
      for (i = 0; i < kernel_lim; i += 4) {

        val1 = row_buf[buf_ind];
        val2 = row_buf[buf_ind + 1];
        val3 = row_buf[buf_ind + 2];
        val4 = row_buf[buf_ind + 3];
        val5 = row_buf[buf_ind + 4];

        ker1 = kernel[i];
        ker2 = kernel[i + 1];
        ker3 = kernel[i + 2];
        ker4 = kernel[i + 3];
        
        t11 += ker1 * val1;
        t21 += ker2 * val2;
        t31 += ker3 * val3;
        t41 += ker4 * val4;
        
        t12 += ker1 * val2;
        t22 += ker2 * val3;
        t32 += ker3 * val4;
        t42 += ker4 * val5;

        inc_adds(8);
        inc_mults(8);
        inc_mem(9);

        buf_ind += 4;
      }

      for (; i < kernel_size; ++i) {
        ker1 = kernel[i];

        t11 += ker1 * row_buf[buf_ind];
        t12 += ker1 * row_buf[buf_ind + 1];
        ++buf_ind;

        inc_adds(2);
        inc_mults(2);
        inc_mem(4);
      }

      
      t11 += t21;
      t31 += t41;
      t12 += t22;
      t32 += t42;
      partialSum += t11 + t31;
      partialSum1 += t12 + t32;

      inc_adds(8);
      
      buf_ind -= 2 * kernel_rad - 1;
      output[dst_ind] = partialSum;
      dst_ind += h;
      output[dst_ind] = partialSum1;
      dst_ind += h;

      inc_mem(2);
    }

    for (; c < w; ++c) {
      partialSum = 0;

      for (int i = 0; i < kernel_size; i++) {
        partialSum += kernel[i] * row_buf[buf_ind];
        inc_adds(1);
        inc_mults(1);
        inc_mem(2);
        ++buf_ind;
      }

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      dst_ind += h;

      inc_mem(1)
    }

    row_ind += w;
  }
  return 1;
}

/// <summary> 
/// Apply the gaussian kernel to the image and write the result to the output.
/// </summary>
/// <param name="image"> IN: Input image to blur. </param>
/// <param name="kernel"> IN: The gaussian kernel/filter we use for blurring. </param>
/// <param name="kernel_size"> IN: Size of gaussian kernels. </param>
/// <param name="kernel_rad"> IN: Radius of the kernel. </param>
/// <param name="output"> OUT: Blurred output image. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_apply_kernel(struct ethsift_image image, float *kernel, uint32_t kernel_size, uint32_t kernel_rad, struct ethsift_image output) {
  uint32_t w = image.width;
  uint32_t h = image.height;
  
  row_filter_transpose(image.pixels, img_buf, w, h, kernel, kernel_size, kernel_rad);
  row_filter_transpose(img_buf, output.pixels, h, w, kernel, kernel_size, kernel_rad);
  return 1;
}
