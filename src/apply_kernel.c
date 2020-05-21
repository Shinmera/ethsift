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

  // const int align = 32;
  // void *p = alloca(8*sizeof(float) + align - 1);
  // float *partialSum = (float*)((((intptr_t)p + align - 1) / align) * align);

  float partialSum[8];
  
  __m256 d_partialSum;
  __m256 d_kernel, d_rowbuf;

  float firstData, lastData;
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_read(w, float);
    inc_write(w, float);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_read(2, float);
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_write(2, float);
    }

    dst_ind = r;
    buf_ind = 0;
    
    int w_lim = w - 7;
    int c;
    for (c = 0; c < w_lim; c += 8) {
      d_partialSum = _mm256_setzero_ps();    

      for (int i = 0; i < kernel_size; ++i) {
        d_kernel = _mm256_broadcast_ss(kernel + i);
        d_rowbuf = _mm256_load_ps(row_buf + buf_ind);
        inc_read(1+8, float);

        d_partialSum = _mm256_fmadd_ps(d_kernel, d_rowbuf, d_partialSum);

        ++buf_ind;
          
        inc_adds(8);
        inc_mults(8);
      }

      _mm256_store_ps(partialSum, d_partialSum);
      inc_write(8, float);

      buf_ind -= 2 * kernel_rad;
      buf_ind += 7;

      for (int i = 0; i < 8; ++i) {
        output[dst_ind] = partialSum[i];
        inc_write(1, float);
        inc_read(1, float);
        dst_ind += h;
      }
    }

    for (; c < w; ++c) {
      float s_partialSum = 0.0f;       

      for (int i = 0; i < kernel_size; i++) {
        s_partialSum += kernel[i] * row_buf[buf_ind];
        inc_adds(1);
        inc_mults(1);
        inc_read(2, float);
        ++buf_ind;
      }

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = s_partialSum;
      inc_write(1, float);
      dst_ind += h;
    }

    row_ind += w;
  }

  return 1;
}

int fft_1D(float* fft_out, float* vector, int inp_size, int out_size) {
  // Maybe can be removed later, but to test if the fft does work in general
  fft_out = (float*)calloc(out_size, sizeof(float));
  


  return 1;
}

int ifft_1D(float* ifft_out, float* vector, int inp_size, int out_size) {
  // Maybe can be removed later, but to test if the ifft does work in general
  ifft_out = (float*)calloc(out_size, sizeof(float));
  
  

  return 1;
}

// TODO 
int row_filter_transpose_fft(float * restrict pixels, float * restrict output, int w, int h, float * restrict kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  // Using 1D FFT
  // for each row in image ifft_1D(fft_1D(row_buf, w + 2 * kernel_size) .* fft_1D(kernel, w + 2 * kernel_size)));
  // And transpose the result
  // precompute only once fft_1D(kernel, w + 2 * kernel_size)



  return 1;
}

// First prototype, to show each optimization step
int row_filter_transpose_first(float * restrict pixels, float * restrict output, int w, int h, float * restrict kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float partialSum = 0.0f;
  float firstData, lastData;

  __m256 t;
  __m256 d_kernel;
  __m256 d_row_buf;

  float t_temp[8];
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_read(w, float);
    inc_write(w, float);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_read(2, float);
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_write(2, float);
    }

    dst_ind = r;
    buf_ind = 0;

    for (int c = 0; c < w; c++) {
      partialSum = 0.0f;   

      t = _mm256_setzero_ps();

      int j;
      int k_lim = kernel_size - 7;
      for (j = 0; j < k_lim; j += 8) {
        d_kernel = _mm256_loadu_ps(kernel + j);
        d_row_buf = _mm256_loadu_ps(row_buf + buf_ind);
        inc_read(16, float);

        t = _mm256_fmadd_ps(d_kernel, d_row_buf, t);
        
        buf_ind += 8;
    
        inc_adds(8);
        inc_mults(8);
      }

      for (; j < kernel_size; ++j) {
        partialSum += kernel[j] * row_buf[buf_ind];
        ++buf_ind;

        inc_adds(1);
        inc_mults(1);
        inc_read(2, float);
      }

      t = _mm256_hadd_ps(t, t);
      _mm256_storeu_ps(t_temp, t);
      inc_write(8, float);

      partialSum += t_temp[0];
      partialSum += t_temp[1];
      partialSum += t_temp[4];
      partialSum += t_temp[5];
      partialSum = partialSum;

      inc_adds(8);


      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = partialSum;
      inc_write(1, float);
      dst_ind += h;
    }

    row_ind += w;
  }
  return 1;
}

// Another AVX version, that should decrease the amount of split_loads
int row_filter_transpose_useing_shuffles(float * restrict pixels, float * restrict output, int w, int h, float * restrict kernel, uint32_t kernel_size, uint32_t kernel_rad) {
  
  int elemSize = sizeof(float);

  int buf_ind = 0;
  int dst_ind = 0;
  int row_ind = 0;

  float partialSum[8];
  __m256 d_partialSum;
  __m256 d_kernel, d_rowbuf;
  __m256 d_rowbuf1;

  float firstData, lastData;

  __m256i permute = _mm256_set_epi32(0,7,6,5,4,3,2,1);
  __m256i permute1 = _mm256_set_epi32(0,1,2,3,4,5,6,7);
  __m256i permute2 = _mm256_set_epi32(6,5,4,3,2,1,0,7);
  
  for (int r = 0; r < h; r++) {
    memcpy(&row_buf[kernel_rad], &pixels[row_ind], elemSize * w);
    inc_read(w, float);
    inc_write(w, float);
    firstData = pixels[row_ind];
    lastData = pixels[row_ind + w - 1];
    inc_read(2, float);
    for (int i = 0; i < kernel_rad; i++) {
      row_buf[i] = firstData;
      row_buf[i + w + kernel_rad] = lastData;
      inc_write(2, float);
    }

    dst_ind = r;
    buf_ind = 0;
    
    int w_lim = w - 7;
    int c;
    for (c = 0; c < w_lim; c += 8) {
      d_partialSum = _mm256_setzero_ps();    

      int k_lim = kernel_size - 15;
      int i;
      for (i = 0; i < k_lim; i += 8) {

        d_kernel = _mm256_broadcast_ss(kernel + i);
        
        d_rowbuf = _mm256_load_ps(row_buf + buf_ind);
        
        d_rowbuf1 = _mm256_load_ps(row_buf + buf_ind + 8);
        d_rowbuf1 = _mm256_permutevar8x32_ps(d_rowbuf1, permute1);

        d_partialSum = _mm256_fmadd_ps(d_kernel, d_rowbuf, d_partialSum);
 
        inc_read(1+8+8, float);
        inc_adds(8);     
        inc_mults(8);

        for (int j = 1; j < 8; ++j) {
          d_kernel = _mm256_broadcast_ss(kernel + i + j);
          inc_read(1, float);
          
          d_rowbuf = _mm256_permutevar8x32_ps(d_rowbuf, permute);
          d_rowbuf = _mm256_blend_ps(d_rowbuf, d_rowbuf1, 0b10000000);
          d_rowbuf1 = _mm256_permutevar8x32_ps(d_rowbuf1, permute2);

          d_partialSum = _mm256_fmadd_ps(d_kernel, d_rowbuf, d_partialSum);

          inc_adds(8);
          inc_mults(8);
        }

        buf_ind += 8;  
      }

      // Do the remaining kernels
      for (; i < kernel_size; ++i) {
        d_kernel = _mm256_broadcast_ss(kernel + i);
        d_rowbuf = _mm256_load_ps(row_buf + buf_ind);

        d_partialSum = _mm256_fmadd_ps(d_kernel, d_rowbuf, d_partialSum);

        ++buf_ind;
          
        inc_adds(8);
        inc_mults(8);
        inc_read(1+8, float);   
      }

      _mm256_store_ps(partialSum, d_partialSum);
      inc_write(8, float);

      buf_ind -= 2 * kernel_rad;
      buf_ind += 7;

      for (i = 0; i < 8; ++i) {
        output[dst_ind] = partialSum[i];
        inc_write(1, float);
        dst_ind += h;
      }
    }

    for (; c < w; ++c) {
      float s_partialSum = 0.0f;       

      for (int i = 0; i < kernel_size; i++) {
        s_partialSum += kernel[i] * row_buf[buf_ind];
        inc_adds(1);
        inc_mults(1);
        inc_read(2, float);
        ++buf_ind;
      }

      buf_ind -= 2 * kernel_rad;
      output[dst_ind] = s_partialSum;
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
