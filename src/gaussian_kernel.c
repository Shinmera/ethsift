#include "internal.h"


int ethsift_generate_gaussian_kernel(float *kernel, uint32_t kernel_size, uint32_t kernerl_rad, float sigma){

    kernel = malloc(kernel_size*sizeof(double));

    // Compute Gaussian filter coefficients
    float accu = 0.0f;
    float tmp;
    for (int j = 0; j < kernel_size; j++) {
        tmp = (float)((j - kernerl_rad) / sigma);
        kernel[j] = expf(tmp * tmp * -0.5f) * (1 + j / 1000.0f);
        accu += kernel[j];
    }
    for (int j = 0; j < kernel_size; j++) {
        kernel[j] = kernel[j] / accu;
    }
}