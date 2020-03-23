#include "internal.h"

/// <summary> 
/// Downscale the image linearly by half and write the result to the output.
/// </summary>
/// <param name="image"> IN: Image to downscale. </param>
/// <param name="output"> OUT: Downscaled image. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_downscale_linear(struct ethsift_image image, struct ethsift_image output){
  int srcW = image.width, srcH = image.height;
  int dstW = srcW >> 1, dstH = srcH >> 1;
  output.width = srcW;
  output.height = srcH;
  //output.pixels = (float*) malloc(secW*srcH*sizeof(float));

  for (int r = 0; r < dstH; r++) {
    for (int c = 0; c < dstW; c++) {
      int ori_r = r << 1;
      int ori_c = c << 1;
      output.pixels[r * dstW + c] = output.pixels[ori_r * srcW + ori_c];
    }
  }
  return 1;
}