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
int ethsift_generate_gaussian_pyramid(struct ethsift_image octaves[],
                            uint32_t octave_count, 
                            struct ethsift_image gaussians[], 
                            uint32_t gaussian_count
                            )
{
    // ZSOMBORS OPTIMIZATION:
    // Since EZSift (and therefore our implementation) only uses octaves[0], which corresponds
    // to the initial input image, I suggest that we simply pass the imput image instead of the 
    // octaves as first input argument.

    int layers_count = gaussian_count - 3;

    // Calculate the gaussian pyramids!
    for (int i = 0; i < octave_count; ++i) {
        for (int j = 0; j < gaussian_count; ++j) {
            // OPT TODO: Optimize these checks, expensive and optimization potential is there !  
            if (i == 0 && j == 0) {
                ethsift_apply_kernel(octaves[0], g_kernel_ptrs[0], g_kernel_sizes[0], g_kernel_rads[0], 
                                    gaussians[0]);
            }
            else if (i > 0 && j == 0) {
                ethsift_downscale_half(gaussians[(i - 1) * gaussian_count + layers_count],
                                         gaussians[i * gaussian_count]);
            }
            else {
                ethsift_apply_kernel(gaussians[i * gaussian_count + j - 1], g_kernel_ptrs[j], g_kernel_sizes[j], 
                                    g_kernel_rads[j], gaussians[i * gaussian_count + j]);
            }
        }
    }
    // TODO: Release octaves memory? EZSift did free memory here.

    

    return 1;
}
