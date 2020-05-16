#include "internal.h"

float** g_kernel_ptrs;
int* g_kernel_rads;
int* g_kernel_sizes;
float *row_buf;
float *img_buf;

/// <summary> 
/// Initialize Gaussian Kernels globally.
/// </summary>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
/// <remarks> 0 flops </remarks>
int ethsift_init(){
  // Precompute gaussian kernels

  // Number of layers in one octave; same as s in the paper.
  const int layers_count = ETHSIFT_INTVLS;
  // Number of Gaussian images in one octave.
  const int gaussian_count = layers_count + 3;

  g_kernel_ptrs = (float**) malloc(sizeof(float*) * gaussian_count);
  g_kernel_rads = (int*) malloc(sizeof(int) * gaussian_count);
  g_kernel_sizes = (int*) malloc(sizeof(int) * gaussian_count);

  ethsift_generate_all_kernels(layers_count, gaussian_count, g_kernel_ptrs, g_kernel_rads, g_kernel_sizes);

  // Make sure we fit up to 4K size images, with max kernel size 64.
  if(posix_memalign(&row_buf, ETHSIFT_MEMALIGN, (7680+64)*sizeof(float))
     || posix_memalign(&img_buf, ETHSIFT_MEMALIGN, 7680*4320*sizeof(float)))
    return 0;
  
  return 1;
}
