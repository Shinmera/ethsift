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
int ethsift_generate_pyramid(struct ethsift_image octaves[],
                            uint32_t octave_count, 
                            struct ethsift_image gaussians[], 
                            uint32_t gaussian_count)
{
    // ZSOMBORS OPTIMIZATION:
    // Since EZSift (and therefore our implementation) only uses octaves[0], which corresponds
    // to the initial input image, I suggest that we simply pass the imput image instead of the 
    // octaves as first input argument.

    int layers_count = gaussian_count - 3;
    int w, h;
    
    float* kernel_ptrs[gaussian_count]; 
    uint32_t kernel_rads[gaussian_count];
    uint32_t kernel_sizes[gaussian_count];

    // Compute all sigmas, kernel sizes, kernel radii and kernels for different layers
    float sigma, sigma_pre;
    float sigma0 = ETHSIFT_SIGMA;
    float k = powf(2.0f, 1.0f / layers_count);
    float sigma_i;

    // Init first sigma
    sigma_pre = ETHSIFT_INIT_SIGMA;
    sigma_i = sqrtf(sigma0 * sigma0 - sigma_pre * sigma_pre);
    kernel_rads[0] = (sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS > 1.0f) 
                ? (int)ceilf(sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS) : 1;
    kernel_sizes[0] = kernel_rads[0] * 2 + 1;

    // Create first kernel.
    // NOTE: Could not come up with a better solution for storing the kernels, due to the 
    // sequential dependencies in the section where we calculate the gaussian pyramids.
    float *kernel = (float*) malloc(kernel_sizes[0]*sizeof(float)); 
    ethsift_generate_gaussian_kernel(kernel, kernel_sizes[0], kernel_rads[0], sigma_i);
    kernel_ptrs[0] = kernel;

    //Calculate all other sigmas and create the according kernel
    for (int i = 1; i < gaussian_count; ++i) {
        // Calculate sigma_i
        sigma_pre = powf(k, (float)(i - 1)) * sigma0;
        sigma = sigma_pre * k;
        sigma_i = sqrtf(sigma * sigma - sigma_pre * sigma_pre);

        // Calculate radii and sizes needed for apply_kernel
        kernel_rads[i] = (sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS > 1.0f) 
                ? (int)ceilf(sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS) : 1;
        kernel_sizes[i] = kernel_rads[i] * 2 + 1;

        // Create kernel and store it in kernels for next step.
        kernel = (float*) malloc(kernel_sizes[i]*sizeof(float)); 
        ethsift_generate_gaussian_kernel(kernel, kernel_sizes[i], kernel_rads[i], sigma_i);
        kernel_ptrs[i] = kernel;
    }


    // Calculate the gaussian pyramids!
    for (int i = 0; i < octave_count; ++i) {
        for (int j = 0; j < gaussian_count; ++j) {
            // OPT TODO: Optimize these checks, expensive and optimization potential is there !  
            if (i == 0 && j == 0) {
                ethsift_apply_kernel(octaves[0], kernel_ptrs[0], kernel_sizes[0], kernel_rads[0], 
                                    gaussians[0]);
            }
            else if (i > 0 && j == 0) {
                ethsift_downscale_linear(gaussians[(i - 1) * gaussian_count + layers_count],
                                         gaussians[i * gaussian_count]);
            }
            else {
                ethsift_apply_kernel(gaussians[i * gaussian_count + j - 1], kernel_ptrs[j], kernel_sizes[j], 
                                    kernel_rads[j], gaussians[i * gaussian_count + j]);
            }
        }
    }
    // TODO: Release octaves memory? EZSift did free memory here.

    //free kernels!
    for (int i = 1; i < gaussian_count; ++i) {
        free(kernel_ptrs[i]);
    }

    return 1;
}
