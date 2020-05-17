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
    for (int c = 0; c < w; c++) {
      partialSum = 0.0f;   

      /// The commented stuff is intentional   
      /// Runtime Measurements done on Debug Build and RDTSC, using lena.pgm (-O0 to measure influence of ILP)

      // // Unchanged version (avg. 139'331'880 cycles on AMD Zen)
      // for (int i = 0; i < kernel_size; i++) {
      //   partialSum += kernel[i] * row_buf[buf_ind];
      //   inc_adds(1);
      //   inc_mults(1);
      //   inc_mem(2);
      //   ++buf_ind;
      // }


      // // This could give a potential speed-up when using an Intel CPU (AMD avg. 125'978'544 cycles)
      // float t1 = 0;
      // float t2 = 0; 
      // float t3 = 0;
      // float t4 = 0; 
      // float t5 = 0;
      // float t6 = 0;
      // float t7 = 0;
      // float t8 = 0;
      // int j;
      // for (j = 0; j < kernel_size; j+=8) {
      //   if (kernel_size - j < 8) {
      //     break;
      //   } 
      //   t1 += kernel[j] * row_buf[buf_ind];
      //   t2 += kernel[j + 1] * row_buf[buf_ind + 1];
      //   t3 += kernel[j + 2] * row_buf[buf_ind + 2];
      //   t4 += kernel[j + 3] * row_buf[buf_ind + 3];
      //   t5 += kernel[j + 4] * row_buf[buf_ind + 4];
      //   t6 += kernel[j + 5] * row_buf[buf_ind + 5];
      //   t7 += kernel[j + 6] * row_buf[buf_ind + 6];
      //   t8 += kernel[j + 7] * row_buf[buf_ind + 7];
      //   buf_ind += 8;
    
      //   inc_adds(8);
      //   inc_mults(8);
      //   inc_mem(16);
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
      // t5 += t6;
      // t7 += t8;
      // t1 += t5;
      // t3 += t7;
      // partialSum += t1 + t3;

      // inc_adds(8);


      // This version completed in avg. 112'452'012 cycles on AMD (atm best version on AMD)
      float t1 = 0;
      float t2 = 0; 
      float t3 = 0;
      float t4 = 0;
      int j;
      for (j = 0; j < kernel_size; j+=4) {
        if (kernel_size - j < 4) {
          break;
        } 
        t1 += kernel[j] * row_buf[buf_ind];
        t2 += kernel[j + 1] * row_buf[buf_ind + 1];
        t3 += kernel[j + 2] * row_buf[buf_ind + 2];
        t4 += kernel[j + 3] * row_buf[buf_ind + 3];
        buf_ind += 4;
    
        inc_adds(4);
        inc_mults(4);
        inc_mem(8);
      }

      for (; j < kernel_size; ++j) {
        t1 += kernel[j] * row_buf[buf_ind];
        ++buf_ind;

        inc_adds(1);
        inc_mults(1);
        inc_mem(2);
      }

      t1 += t2;
      t3 += t4;
      partialSum += t1 + t3;

      inc_adds(4);


      // // This version completed in avg. 113'821'596 cycles on AMD 
      // float t1 = 0;
      // float t2 = 0; 
      // float t3 = 0;
      // float t4 = 0;
      // int j;
      // for (j = 0; j < kernel_size; j+=4) {
      //   if (kernel_size - j < 4) {
      //     break;
      //   } 
      //   t1 = kernel[j] * row_buf[buf_ind];
      //   t2 = kernel[j + 1] * row_buf[buf_ind + 1];
      //   t3 = kernel[j + 2] * row_buf[buf_ind + 2];
      //   t4 = kernel[j + 3] * row_buf[buf_ind + 3];

      //   t1 += t2;
      //   t3 += t4;
      //   partialSum += t1 + t3;

      //   buf_ind += 4;
    
      //   inc_adds(4);
      //   inc_mults(4);
      //   inc_mem(8);
      // }

      // for (; j < kernel_size; ++j) {
      //   partialSum += kernel[j] * row_buf[buf_ind];
      //   ++buf_ind;

      //   inc_adds(1);
      //   inc_mults(1);
      //   inc_mem(2);
      // }


      // // This could give a potential speed-up when using an Intel CPU (AMD avg. 117'139'356 cycles)
      // float t1 = 0;
      // float t2 = 0; 
      // float t3 = 0;
      // float t4 = 0; 
      // float t5 = 0;
      // float t6 = 0;
      // float t7 = 0;
      // float t8 = 0;
      // int j;
      // for (j = 0; j < kernel_size; j+=8) {
      //   if (kernel_size - j < 8) {
      //     break;
      //   } 
      //   t1 = kernel[j] * row_buf[buf_ind];
      //   t2 = kernel[j + 1] * row_buf[buf_ind + 1];
      //   t3 = kernel[j + 2] * row_buf[buf_ind + 2];
      //   t4 = kernel[j + 3] * row_buf[buf_ind + 3];
      //   t5 = kernel[j + 4] * row_buf[buf_ind + 4];
      //   t6 = kernel[j + 5] * row_buf[buf_ind + 5];
      //   t7 = kernel[j + 6] * row_buf[buf_ind + 6];
      //   t8 = kernel[j + 7] * row_buf[buf_ind + 7];

      //   t1 += t2;
      //   t3 += t4;
      //   t5 += t6;
      //   t7 += t8;
      //   t1 += t5;
      //   t3 += t7;
      //   partialSum += t1 + t3;

      //   buf_ind += 8;
    
      //   inc_adds(8);
      //   inc_mults(8);
      //   inc_mem(16);
      // }

      // for (; j < kernel_size; ++j) {
      //   partialSum += kernel[j] * row_buf[buf_ind];
      //   ++buf_ind;

      //   inc_adds(1);
      //   inc_mults(1);
      //   inc_mem(2);
      // }


      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      inc_mem(1);
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
