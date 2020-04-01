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
int ethsift_generate_gaussian_pyramid(struct ethsift_image octaves[],
                            uint32_t octave_count, 
                            struct ethsift_image gaussians[], 
                            uint32_t gaussian_count)
{
    // ZSOMBORS OPTIMIZATION:
    // Since EZSift (and therefore our implementation) only uses octaves[0], which corresponds
    // to the initial input image, I suggest that we simply pass the imput image instead of the 
    // octaves as first input argument.

    int layers_count = gaussian_count - 3;
    
    
    float* kernel_ptrs[gaussian_count]; 
    int kernel_rads[gaussian_count];
    int kernel_sizes[gaussian_count];


    ethsift_generate_all_kernels(layers_count, gaussian_count, kernel_ptrs, kernel_rads, kernel_sizes);


    // Calculate the gaussian pyramids!
    for (int i = 0; i < octave_count; ++i) {
        for (int j = 0; j < gaussian_count; ++j) {
            // OPT TODO: Optimize these checks, expensive and optimization potential is there !  
            if (i == 0 && j == 0) {
                ethsift_apply_kernel(octaves[0], kernel_ptrs[0], kernel_sizes[0], kernel_rads[0], 
                                    gaussians[0]);
            }
            else if (i > 0 && j == 0) {
                ethsift_downscale_half(gaussians[(i - 1) * gaussian_count + layers_count],
                                         gaussians[i * gaussian_count]);
            }
            else {
                ethsift_apply_kernel(gaussians[i * gaussian_count + j - 1], kernel_ptrs[j], kernel_sizes[j], 
                                    kernel_rads[j], gaussians[i * gaussian_count + j]);
            }
        }
    }
    // TODO: Release octaves memory? EZSift did free memory here.

    ethsift_free_kernels(kernel_ptrs, gaussian_count);

    return 1;
}
