#include "internal.h"

/// <summary> 
/// Smartly allocate the image pyramid contents (allocate pixels, set sizes).
/// </summary>
/// <param name="pyramid"> OUT: The pyramid we want to allocate. </param>
/// <param name="ref_width"> IN: Reference width from the image we analyze. </param>
/// <param name="ref_height"> IN: Reference height from the image we analyze. </param>
/// <param name="layer_count"> IN: Number of layers the pyramid has. </param>
/// <param name="image_per_layer_count"> IN: Number of images the pyramid has per layer. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_allocate_pyramid(struct ethsift_image pyramid[], uint32_t ref_width, uint32_t ref_height, uint32_t layer_count, uint32_t image_per_layer_count){
  if(layer_count == 0) return 1;
  if(image_per_layer_count == 0) return 1;

  uint32_t dim;

  // Ensure the pyramid can even be built at the requested depth.
  if(ref_width / (2 << layer_count) <= 0) return 0;
  if(ref_height / (2 << layer_count) <= 0) return 0;
  if(__builtin_umul_overflow(ref_width, ref_height, &dim)) return 0;
     
  // Analytical integer formula to compute total number
  // of pixels across all pyramid levels:
  // sum(i=0..n-1) w/(2^i)*y/(2^i)
  // = 1/3*w*h*(4-4^(1-n))
  // = (4*w*h-2*w*h/(2^(2*(n-1))))/3
  size_t total_size = (4*dim - 2*dim / (2 << 2*(layer_count-1))) / 3;
  total_size *= image_per_layer_count;

  float *pixels = 0;
  if(posix_memalign(&pixels, ETHSIFT_MEMALIGN, total_size*sizeof(float)))
    return 0;

  uint32_t width = ref_width;
  uint32_t height = ref_height;

  for(int i=0; i<layer_count; ++i){
    for (int j = 0; j < image_per_layer_count; ++j) {

      pyramid[i*image_per_layer_count + j].pixels = pixels;
      pyramid[i*image_per_layer_count + j].width = width;
      pyramid[i*image_per_layer_count + j].height = height;
      pixels += width * height;
    }
    width /= 2;
    height /= 2;
  }
  
  return 1;
}

/// <summary> 
/// Free up the pyramids allocated memory.
/// </summary>
/// <param name="pyramid"> IN: The pyramid we want to free up. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_free_pyramid(struct ethsift_image pyramid[]) {
  if(pyramid[0].pixels != 0){
    free(pyramid[0].pixels);
    pyramid[0].pixels = 0;
  }
  return 1;
}
