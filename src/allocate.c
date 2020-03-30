#include "internal.h"
#include <stdlib.h>

int ethsift_allocate_pyramid(struct ethsift_image pyramid[], uint32_t pyramid_count){
  if(pyramid_count == 0) return 1;

  uint32_t width = pyramid[0].width;
  uint32_t height = pyramid[0].height;

  // Analytical integer formula to compute total number
  // of pixels across all pyramid levels:
  // sum(i=0..n-1) w/(2^i)*y/(2^i)
  // = 1/3*w*h*(4-4^(1-n))
  // = (4*w*h-2*w*h/(2^(2*(n-1))))/3
  uint32_t total_size = (4*width*height - 2*width*height / (2 << 2*(pyramid_count-1))) / 3;

  float *pixels = calloc(sizeof(float), total_size);
  if(pixels == 0) return 0;

  pyramid[0].pixels = pixels;
  for(int i=1; i<pyramid_count; ++i){
    pixels += width*height;
    width /= 2;
    height /= 2;
    pyramid[i].pixels = pixels;
    pyramid[i].width = width;
    pyramid[i].height = height;
  }
  
  return 1;
}
