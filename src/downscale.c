#include "internal.h"

/// <summary> 
/// Downscale the image by half and write the result to the output.
/// </summary>
/// <param name="image"> IN: Image to downscale. </param>
/// <param name="output"> OUT: Downscaled image. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_downscale_half(struct ethsift_image image, struct ethsift_image output){
  int srcW = image.width, srcH = image.height;
  int dstW = output.width, dstH = output.height;

  for (int r = 0; r < dstH; r++) {
    for (int c = 0; c < dstW; c++) {
      int ori_r = r << 1;
      int ori_c = c << 1;
      output.pixels[r * dstW + c] = image.pixels[ori_r * srcW + ori_c];

      inc_mem(2); // 1 write / 1 read
    }
  }
  return 1;
}