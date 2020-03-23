#include "internal.h"


int ethsift_generate_gaussian_kernel(float *kernel, uint32_t kernel_size, uint32_t kernerl_rad, float sigma){

        // Compute Gaussian filter coefficients
        int kernel_size = kernerl_rad * 2 + 1;

        kernel = malloc(kernel_size*sizeof(double))
        float accu = 0.0f;
        float tmp;
        for (int j = 0; j < kernel_size; j++) {
            tmp = (float)((j - kernerl_rad) / sigma);
            kernel[j] = expf(tmp * tmp * -0.5f) * (1 + j / 1000.0f);
            accu += kernel[j];
        }
        for (int j = 0; j < kernel_size; j++) {
            kernel[j] = kernel[j] / accu;
        } // End compute Gaussian filter coefs
}