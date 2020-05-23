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
  // Outer and inner loop are unrolled. With -O0 optimization level needs about 90 Mio cycles, -O3 -fno-tree-vectorize about 22Mio to convolve lena.pgm (AMD Zen).
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float partialSum = 0.0f;
  float partialSum1 = 0.0f;
  float partialSum2 = 0.0f;
  float partialSum3 = 0.0f;
  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_read(w, float);
    inc_write(w, float);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_read(2, float); // 2 reads
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_write(2, float); // 2 writes
    }

    dst_ind = r;
    buf_ind = 0;

    int lim = w - 3;
    int c;
    for (c = 0; c < lim; c += 4) {
      partialSum = 0;
      partialSum1 = 0;
      partialSum2 = 0;
      partialSum3 = 0;

      float t11 = 0;
      float t21 = 0; 
      float t31 = 0;
      float t41 = 0;

      float t12 = 0;
      float t22 = 0; 
      float t32 = 0;
      float t42 = 0;
      
      float t13 = 0;
      float t23 = 0; 
      float t33 = 0;
      float t43 = 0;
      
      float t14 = 0;
      float t24 = 0; 
      float t34 = 0;
      float t44 = 0;

      float val1, val2, val3, val4, val5, val6, val7;
      float ker1, ker2, ker3, ker4;

      int kernel_lim = kernel_size - 4;
      int i;
      for (i = 0; i < kernel_lim; i += 4) {

        val1 = row_buf[buf_ind];
        val2 = row_buf[buf_ind + 1];
        val3 = row_buf[buf_ind + 2];
        val4 = row_buf[buf_ind + 3];
        val5 = row_buf[buf_ind + 4];
        val6 = row_buf[buf_ind + 5];
        val7 = row_buf[buf_ind + 6];

        ker1 = kernel[i];
        ker2 = kernel[i + 1];
        ker3 = kernel[i + 2];
        ker4 = kernel[i + 3];
        
        t11 = ker1 * val1;
        t21 = ker2 * val2;
        t31 = ker3 * val3;
        t41 = ker4 * val4;
        
        t12 = ker1 * val2;
        t22 = ker2 * val3;
        t32 = ker3 * val4;
        t42 = ker4 * val5;
        
        t13 = ker1 * val3;
        t23 = ker2 * val4;
        t33 = ker3 * val5;
        t43 = ker4 * val6;

        t14 = ker1 * val4;
        t24 = ker2 * val5;
        t34 = ker3 * val6;
        t44 = ker4 * val7;

        t11 += t21;
        t31 += t41;

        t12 += t22;
        t32 += t42;

        t13 += t23;
        t33 += t43;

        t14 += t24;
        t34 += t44;
        
        partialSum += t11 + t31;
        partialSum1 += t12 + t32;
        partialSum2 += t13 + t33;
        partialSum3 += t14 + t34;

        inc_adds(16);
        inc_mults(16);
        inc_read(11, float);

        buf_ind += 4;
      }

      for (; i < kernel_size; ++i) {
        ker1 = kernel[i];

        partialSum += ker1 * row_buf[buf_ind];
        partialSum1 += ker1 * row_buf[buf_ind + 1];
        partialSum2 += ker1 * row_buf[buf_ind + 2];
        partialSum3 += ker1 * row_buf[buf_ind + 3];
        ++buf_ind;

        inc_adds(4);
        inc_mults(4);
        inc_read(5, float);
      }
      
      buf_ind -= 2 * kernel_rad - 3;

      output[dst_ind] = partialSum;
      dst_ind += h;
      output[dst_ind] = partialSum1;
      dst_ind += h;
      output[dst_ind] = partialSum2;
      dst_ind += h;
      output[dst_ind] = partialSum3;
      dst_ind += h;

      inc_write(4, float);
    }

    for (; c < w; ++c) {
      partialSum = 0;

      for (int i = 0; i < kernel_size; i++) {
        partialSum += kernel[i] * row_buf[buf_ind];
        inc_adds(1);
        inc_mults(1);
        inc_read(2, float);
        ++buf_ind;
      }

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      dst_ind += h;

      inc_write(1, float);
    }

    row_ind += w;
  }
  return 1;
}

// In this version of row_filter_unroll only the inner loop is unrolled. When using -O0 optimization level needs about 111 Mio cycles to convolve lena.pgm (AMD Zen) 
int row_filter_transpose_single_unroll(float * restrict pixels, float * restrict output, int w, int h, float * restrict kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float partialSum = 0.0f;
  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_read(w, float);
    inc_write(w, float);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_read(2, float); // 2 reads
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_write(2, float); // 2 writes
    }

    dst_ind = r;
    buf_ind = 0;
    for (int c = 0; c < w; c++) {
      partialSum = 0.0f;   

      // 8-times inner loop unroll: This could give a potential speed-up when using an Intel CPU (AMD avg. 125'978'544 cycles)
      float t1 = 0;
      float t2 = 0; 
      float t3 = 0;
      float t4 = 0; 
      float t5 = 0;
      float t6 = 0;
      float t7 = 0;
      float t8 = 0;
      int j;
      for (j = 0; j < kernel_size; j+=8) {
        if (kernel_size - j < 8) {
          break;
        } 
        t1 += kernel[j] * row_buf[buf_ind];
        t2 += kernel[j + 1] * row_buf[buf_ind + 1];
        t3 += kernel[j + 2] * row_buf[buf_ind + 2];
        t4 += kernel[j + 3] * row_buf[buf_ind + 3];
        t5 += kernel[j + 4] * row_buf[buf_ind + 4];
        t6 += kernel[j + 5] * row_buf[buf_ind + 5];
        t7 += kernel[j + 6] * row_buf[buf_ind + 6];
        t8 += kernel[j + 7] * row_buf[buf_ind + 7];
        buf_ind += 8;
    
        inc_adds(8);
        inc_mults(8);
        inc_read(16, float);
      }

      for (; j < kernel_size; ++j) {
        t1 += kernel[j] * row_buf[buf_ind];
        ++buf_ind;

        inc_adds(1);
        inc_mults(1);
        inc_read(2, float);
      }

      t1 += t2;
      t3 += t4;
      t5 += t6;
      t7 += t8;
      t1 += t5;
      t3 += t7;
      partialSum += t1 + t3;

      inc_adds(8);

      // // 4 times inner loop unroll: This version completed in avg. 112'452'012 cycles on AMD (atm best version on AMD)
      // float t1 = 0;
      // float t2 = 0; 
      // float t3 = 0;
      // float t4 = 0;
      // int j;
      // for (j = 0; j < kernel_size; j+=4) {
      //   if (kernel_size - j < 4) {
      //     break;
      //   } 
      //   t1 += kernel[j] * row_buf[buf_ind];
      //   t2 += kernel[j + 1] * row_buf[buf_ind + 1];
      //   t3 += kernel[j + 2] * row_buf[buf_ind + 2];
      //   t4 += kernel[j + 3] * row_buf[buf_ind + 3];
      //   buf_ind += 4;
    
      //   inc_adds(4);
      //   inc_mults(4);
      //   inc_mem(8);
      // }

      // for (; j < kernel_size; ++j) {
      //   t1 += kernel[j] * row_buf[buf_ind];
      //   ++buf_ind;

      //   inc_adds(1);
      //   inc_mults(1);
      //   inc_mem(2);
      // }

      // t1 += t2;
      // t3 += t4;
      // partialSum += t1 + t3;

      // inc_adds(4);

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      inc_write(1, float);
      dst_ind += h;
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
