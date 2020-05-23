#include "internal.h"

/// <summary> 
/// Creates a pyramid of images containing blurred versions of the input image.
/// </summary>
/// <param name="octaves"> IN: The octaves of the input image. </param>
/// <param name="octave_count"> IN: Number of octaves. </param>
/// <param name="gaussians"> IN/OUT: Struct of gaussians to compute. 
/// NOTE: Size = octave_count * gaussian_count. </param>
/// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
/// <remarks> ggk + ((gaussian_count-1)*octave_count + 1) * ak </remarks>
int ethsift_generate_gaussian_pyramid(struct ethsift_image image,
                            uint32_t octave_count, 
                            struct ethsift_image gaussians[], 
                            uint32_t gaussian_count){
    int layers_count = gaussian_count - 3;
    
    // Calculate the gaussian pyramids!
    ethsift_apply_kernel(image, g_kernel_ptrs[0], g_kernel_sizes[0], g_kernel_rads[0], 
                         gaussians[0]);
   
    inc_read(1, float*);
    inc_read(2, int);
    inc_read(1, struct ethsift_image);

    for(int j = 1; j < gaussian_count; ++j){
      ethsift_apply_kernel(gaussians[j - 1], g_kernel_ptrs[j], g_kernel_sizes[j], 
                           g_kernel_rads[j], gaussians[j]);
      inc_read(1, float*);
      inc_read(2, int);
      inc_read(1, struct ethsift_image);
    }
    for (int i = 1; i < octave_count; ++i) {
      ethsift_downscale_half(gaussians[(i - 1) * gaussian_count + layers_count],
                             gaussians[i * gaussian_count]);
      inc_read(2, struct ethsift_image);
      for (int j = 1; j < gaussian_count; ++j) {
        ethsift_apply_kernel(gaussians[i * gaussian_count + j - 1], g_kernel_ptrs[j], g_kernel_sizes[j], 
                             g_kernel_rads[j], gaussians[i * gaussian_count + j]);
        inc_read(1, float*);
        inc_read(2, int);
        inc_read(1, struct ethsift_image);
      }
    }

    return 1;
}
