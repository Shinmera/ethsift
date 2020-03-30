#include "internal.h"


void print_kernel(float* kernel, 
                uint32_t kernel_size, 
                float sigma){
    printf("Kernel %f:\n[",sigma);
    for(int i = 0; i < kernel_size; ++i){
        printf("%f, ", kernel[i]);
    }
    printf("]\n\n");
}

/// <summary> 
/// Creates a gaussian kernel for image filtering.
/// </summary>
/// <param name="kernel"> IN/OUT: Kernel to generate. </param>
/// <param name="kernel_size"> IN: Kernel size. </param>
/// <param name="kernerl_rad"> IN: Kernel radius. </param>
/// <param name="sigma"> IN: Standard deviation of the gaussian kernel. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_gaussian_kernel(float *kernel,
                                    int kernel_size,
                                    int kernel_rad,
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
    float mul = 1.0f / accu;
    for (int j = 0; j < kernel_size; j++) {
        kernel[j] = kernel[j]*mul;
    }
    //print_kernel(kernel, kernel_size, sigma);
    return 1;
}



int ethsift_generate_all_kernels(int layers_count, 
                                uint32_t gaussian_count, 
                                float **kernel_ptrs, 
                                int kernel_rads[], 
                                int kernel_sizes[]){

    
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
    // TEST-NOTE: Test and remove memory allocation in case stack is able to handle all the kernels.
    kernel_ptrs[0] = (float*) calloc(kernel_sizes[0], sizeof(float)); 
    ethsift_generate_gaussian_kernel(kernel_ptrs[0], kernel_sizes[0], kernel_rads[0], sigma_i);
    //print_kernel(kernel_ptrs[0], kernel_sizes[0], sigma_i);

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
        // TEST-NOTE: Test and remove memory allocation in case stack is able to handle all the kernels.
        kernel_ptrs[i] = (float*) calloc(kernel_sizes[i], sizeof(float)); 
        ethsift_generate_gaussian_kernel(kernel_ptrs[i], kernel_sizes[i], kernel_rads[i], sigma_i);

        //print_kernel(kernel_ptrs[i], kernel_sizes[i], sigma_i);
    }
    return 1;
}


int ethsift_free_kernels(float** kernel_ptrs, uint32_t gaussian_count){
    //free kernels!
    for (int i = 1; i < gaussian_count; ++i) {
        free(kernel_ptrs[i]);
    }
    return 1;
}