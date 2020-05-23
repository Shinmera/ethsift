#include "internal.h"

/// <summary> 
/// Creates a gaussian kernel for image filtering.
/// </summary>
/// <param name="kernel"> OUT: Kernel to generate. </param>
/// <param name="kernel_size"> IN: Kernel size. </param>
/// <param name="kernerl_rad"> IN: Kernel radius. </param>
/// <param name="sigma"> IN: Standard deviation of the gaussian kernel. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
/// <remarks>ggk =  kernel_size * (7 + EXP) + 1 flops </remarks>
int ethsift_generate_gaussian_kernel(float *kernel,
                                    int kernel_size,
                                    int kernel_rad,
                                    float sigma)
{
    // Compute Gaussian filter coefficients
    float accu = 0.0f;
    float tmp;
    for (int j = 0; j < kernel_size; ++j) {
        tmp = (float)((j - kernel_rad) / sigma); //Float conversion 2 FLOP?
        kernel[j] = expf(tmp * tmp * -0.5f) * (1 + j / 1000.0f); // 3 MULs + 1 ADDs + 1DIVs + 1EXPs = 
        accu += kernel[j];
        inc_adds(2);
        inc_mults(3);
        inc_write(1, float);
        inc_read(1, float);
        inc_div(1);
    }
    float mul = 1.0f / accu; // 1 DIV
    inc_div(1);
    for (int j = 0; j < kernel_size; j++) {
        kernel[j] = kernel[j]*mul; // 1 MUL
        inc_mults(1);
        inc_write(1, float);
        inc_read(1, float);
    }
    return 1;
}


/// <summary> 
/// Creates a gaussian kernel for image filtering.
/// </summary>
/// <param name="layers_count"> IN: Amount of layers. </param>
/// <param name="gaussian_count"> IN: Amount of gaussian kernels to generate. </param>
/// <param name="kernel_ptrs"> OUT: Pointers to the kernels </param>
/// <param name="kernel_rads"> OUT: The radii of all the kernels stored in an array. </param> 
/// <param name="kernel_sizes"> OUT: The sizes of all the kernels stored in an array. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
/// <remarks> gaussian_count * (powf + sqrt + ceilf + 7 + ggk) </remarks>
int ethsift_generate_all_kernels(int layers_count, 
                                uint32_t gaussian_count, 
                                float **kernel_ptrs, 
                                int kernel_rads[], 
                                int kernel_sizes[]){

    
    // Compute all sigmas, kernel sizes, kernel radii and kernels for different layers
    float sigma, sigma_pre;
    float sigma0 = ETHSIFT_SIGMA;
    float k = powf(2.0f, 1.0f / layers_count); //<==> root(2, 1/3) 1FLOP
    inc_div(1); // Since div is something like a worst case
    float sigma_i;

    // Init first sigma
    sigma_pre = ETHSIFT_INIT_SIGMA;
    sigma_i = sqrtf(sigma0 * sigma0 - sigma_pre * sigma_pre); //1 FLOPf for sqrt 2 for MULs 1 for SUB = 4
    inc_adds(1);
    inc_mults(2);
    inc_div(1);     // For the sqrtf
    kernel_rads[0] = (sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS > 1.0f) // 1MUL
                ? (int)ceilf(sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS) : 1; //1 MUL and 1 CEIL
    kernel_sizes[0] = kernel_rads[0] * 2 + 1; // 1MUL and 1ADD  // These seem to be integer operations
    inc_mults(2); // Worst case
    
    inc_read(1, int);
    inc_write(2, int);

    // Create first kernel.
    // NOTE: Could not come up with a better solution for storing the kernels, due to the 
    // sequential dependencies in the section where we calculate the gaussian pyramids.
    // TEST-NOTE: Test and remove memory allocation in case stack is able to handle all the kernels.
    kernel_ptrs[0] = (float*) calloc(kernel_sizes[0], sizeof(float));
    ethsift_generate_gaussian_kernel(kernel_ptrs[0], kernel_sizes[0], kernel_rads[0], sigma_i); // ggk FLOPs
    
    inc_read(3, int);
    inc_read(1, float);
    inc_write(1, float);

    //Calculate all other sigmas and create the according kernel
    for (int i = 1; i < gaussian_count; ++i) {
        // Calculate sigma_i
        sigma_pre = powf(k, (float)(i - 1)) * sigma0; // Wouldn't this be 1 extra MULT as well?
        sigma = sigma_pre * k; // 1MUL
        sigma_i = sqrtf(sigma * sigma - sigma_pre * sigma_pre); //1 FLOPf for sqrt 2 for MULs 1 for SUB = 4
        inc_adds(1);
        inc_mults(4);

        // Calculate radii and sizes needed for apply_kernel
        kernel_rads[i] = (sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS > 1.0f) // 1MUL
                ? (int)ceilf(sigma_i * ETHSIFT_GAUSSIAN_FILTER_RADIUS) : 1; //1 MUL and 1 CEIL
        kernel_sizes[i] = kernel_rads[i] * 2 + 1;
        
        inc_read(1, int);
        inc_write(2, int);
        inc_mults(2);

        // Create kernel and store it in kernels for next step.
        // TEST-NOTE: Test and remove memory allocation in case stack is able to handle all the kernels.
        kernel_ptrs[i] = (float*) calloc(kernel_sizes[i], sizeof(float)); 
        ethsift_generate_gaussian_kernel(kernel_ptrs[i], kernel_sizes[i], kernel_rads[i], sigma_i); // ggk FLOPs
        
        inc_read(3, int);
        inc_read(1, float);
        inc_write(1, float);
    }
    return 1;
}

/// <summary> 
/// Frees up the allocated memory of the kernels
/// </summary>
/// <param name="kernel_ptrs"> IN: Pointers to the kernels for freeing up</param>
/// <param name="gaussian_count"> IN: Amount of gaussian kernels. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_free_kernels(float** kernel_ptrs, uint32_t gaussian_count){
    //free kernels!
    for (int i = 1; i < gaussian_count; ++i) {
        free(kernel_ptrs[i]);
    }
    return 1;
}