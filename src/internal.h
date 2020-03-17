#include <stdio.h>
#include "ethsift.h"

// Wrap image pixel access. Note this does not handle border conditions!
static inline float pixel(struct ethsift_image image, uint32_t x, uint32_t y){
  return image.pixels[image.width*y+x];
};
