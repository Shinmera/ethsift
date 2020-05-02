import numpy as np
import math

convolution_kernel_size = 9

calc_layers = 3 # Default value set in settings.c
calc_gaussian_count = calc_layers + 3
calc_octave_count = lambda w, h: (int) (math.log2(min([w, h])) - 3)
calc_fast_atan2_f = lambda y: 10 if (y < 0) else 9 
calc_get_pixel_f = 0

exp_flops = 1
ceilf_flops = 1
sqrt_flops = 1
powf_flops = 1

flops_util = dict()

flops_util['eth'] = dict()
flops_util['eth']['Downscale'] = lambda  w, h: 0 # Conducts only memcpy
flops_util['eth']['Convolution'] = lambda w, h: 4 * w * h * convolution_kernel_size # 2 * (h * w (2* kernel_size))
flops_util['eth']['Octaves'] = lambda w, h: 0 # Conducts only memcpy
flops_util['eth']['GaussianKernelGeneration'] = lambda w, h: calc_gaussian_count * (powf_flops + sqrt_flops + ceilf_flops + 7 + (get_kernel_sizes(calc_gaussian_count)*(6.0+exp_flops) + calc_gaussian_count))
flops_util['eth']['GaussianPyramid'] = lambda w, h: flops_util['eth']['GaussianKernelGeneration'](w,h) +  ((calc_gaussian_count-1) * calc_octave_count + 1) * flops_util['eth']['Convolution'](w,h)
flops_util['eth']['DOGPyramid'] = lambda w, h: calc_octave_count * 5 * w * h
flops_util['eth']['GradientAndRotationPyramids'] = lambda w, h: calc_octave_count * 3 * w * h * (2 + (4 * calc_get_pixel_f) + sqrt + calc_fast_atan2_f(0))
flops_util['eth']['Histogram'] = lambda w, h:     w*h
flops_util['eth']['ExtremaRefinement'] = lambda w, h:     w*h
flops_util['eth']['KeypointDetection'] = lambda w, h:     w*h
flops_util['eth']['ExtractDescriptor'] = lambda w, h:     w*h

flops_util['ez'] = dict()
flops_util['ez']['Downscale'] = lambda w, h: 0 # Conducts only memcpy
flops_util['ez']['Convolution'] = lambda w, h: 4 * w * h * convolution_kernel_size
flops_util['ez']['Octaves'] = lambda w, h: 0 # Conducts only memcpy
flops_util['ez']['GaussianKernelGeneration'] = lambda w, h: calc_gaussian_count * (powf_flops + sqrt_flops + ceilf_flops + 7 + (get_kernel_sizes(calc_gaussian_count)*(6.0+exp_flops) + calc_gaussian_count)
flops_util['ez']['GaussianPyramid'] = lambda w, h: flops_util['ez']['GaussianKernelGeneration'](w,h) +  ((calc_gaussian_count-1) * calc_octave_count + 1) * flops_util['ez']['Convolution'](w,h)
flops_util['ez']['DOGPyramid'] = lambda w, h:     w*h
flops_util['ez']['GradientAndRotationPyramids'] = lambda w, h:     w*h
flops_util['ez']['Histogram'] = lambda w, h:     w*h
flops_util['ez']['ExtremaRefinement'] = lambda w, h:     w*h
flops_util['ez']['KeypointDetection'] = lambda w, h:     w*h
flops_util['ez']['ExtractDescriptor'] = lambda w, h:     w*h

def get_kernel_sizes(gaussian_count):
    kernel_sizes = np.zeros(gaussian_count)
    k = pow(2.0, 1.0 / calc_layers)

    sigma_pre = 0.5 # Default value set in settings.c
    sigma0 = 1.6 # Default value set in settings.c

    sigma_i = np.sqrtf(sigma0 * sigma0 - sigma_pre * sigma_pre)
    curr_rad = sigma_i * 3.0
    if curr_rad > 1.0:
        curr_rad = np.ceil(curr_rad)
    else:
        curr_rad = 1
    
    kernel_sizes[0] = curr_rad * 2 + 1

    for i in range(1, gaussian_count):
        sigma_pre = pow(k, (i-1.0)*sigma0)
        sigma = sigma_pre *k
        sigma_i = np.sqrt(sigma*sigma - sigma_pre-sigma_pre)
        curr_rad = sigma_i * 3.0
        if curr_rad > 1.0:
            curr_rad = np.ceil(curr_rad)
        else:
            curr_rad = 1
        
        kernel_sizes[i]  = curr_rad * 2 + 1


    return kernel_sizes