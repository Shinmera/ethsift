#include "internal.h"

/// <summary> 
/// Creates a gaussian kernel for image filtering.
/// </summary>
/// <param name="kernel"> IN/OUT: Kernel to generate. </param>
/// <param name="kernel_size"> IN: Kernel size. </param>
/// <param name="kernerl_rad"> IN: Kernel radius. </param>
/// <param name="sigma"> IN: Standard deviation of the gaussian kernel. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_gaussian_kernel(float *kernel,
                                    uint32_t kernel_size,
                                    uint32_t kernel_rad,
                                    float sigma)
{
    // Compute Gaussian filter coefficients
    float accu = 0.0f;
    float tmp;
    for (int j = 0; j < kernel_size; ++j) {
        tmp = (float)((j - kernel_rad) / sigma);
        kernel[j] = expf(tmp * tmp * -0.5f) * (1 + j / 1000.0f);
        accu += kernel[j];
    }
    for (int j = 0; j < kernel_size; j++) {
        kernel[j] = kernel[j] / accu;
    }
    return 1;
}