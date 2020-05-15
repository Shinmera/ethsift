#include "internal.h"

int ethsift_init(){
  // Precompute gaussian kernels
  int layers_count = ETHSIFT_INTVLS;
  uint32_t gaussian_count = ETHSIFT_INTVLS + 3;
  g_kernel_ptrs = malloc(sizeof(float*) * gaussian_count);
  g_kernel_rads = malloc(sizeof(int) * gaussian_count);
  g_kernel_sizes = malloc(sizeof(int) * gaussian_count);

  ethsift_generate_all_kernels(layers_count, gaussian_count, g_kernel_ptrs, g_kernel_rads, g_kernel_sizes);

  return 1;
}