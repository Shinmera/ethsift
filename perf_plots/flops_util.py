import numpy as np
import math

convolution_kernel_size = 9

calc_layers = 3 # Default value set in settings.c
calc_gaussian_count = calc_layers + 3
calc_octave_count = lambda w, h: (int) (math.log2(min([w, h])) - 3)
bin_count = 36
approx_keypoint_scale = 1
histogram_window_size = (2 * 4.5 * approx_keypoint_scale + 1)
calc_fast_atan2_f = lambda y: 10 if (y < 0) else 9 
calc_fast_ata2_f_worst_case = 10
calc_get_pixel_f = 0

exp_flops = 1
floor_flops = 1
ceilf_flops = 1
sqrt_flops = 1
powf_flops = 1
sinf_flops = 1
cosf_flops = 1
minf_flops = 1

keypoint_count = 1000
descriptor_width = 4
descriptor_bin = 8
descriptor_size = 3.0
approx_kpt_scale = 1

flops_util = dict()

flops_util['eth'] = dict()
flops_util['eth']['Downscale'] = lambda  w, h: 0 # Conducts only memcpy
flops_util['eth']['Convolution'] = lambda w, h: 4 * w * h * convolution_kernel_size # 2 * (h * w (2* kernel_size))
flops_util['eth']['Octaves'] = lambda w, h: 0 # Conducts only memcpy
flops_util['eth']['GaussianKernelGeneration'] = lambda w, h: calc_gaussian_count * (powf_flops + sqrt_flops + ceilf_flops + 7 + (get_kernel_sizes(calc_gaussian_count)*(6.0+exp_flops) + calc_gaussian_count))
flops_util['eth']['GaussianPyramid'] = lambda w, h: flops_util['eth']['GaussianKernelGeneration'](w,h) +  ((calc_gaussian_count-1) * calc_octave_count + 1) * flops_util['eth']['Convolution'](w,h)
flops_util['eth']['DOGPyramid'] = lambda w, h: diff_pyr_ops(w, h)
flops_util['eth']['GradientAndRotationPyramids'] = lambda w, h: gr_pyr_ops(w, h)
flops_util['eth']['Histogram'] = lambda w, h: 11 + histogram_window_size * histogram_window_size * (18.0 + exp_flops) + bin_count * 10
flops_util['eth']['ExtremaRefinement'] = lambda w, h: 477 + 2* powf_flops
flops_util['eth']['KeypointDetection'] = lambda w, h:     w*h
flops_util['eth']['ExtractDescriptor'] = lambda w, h:     2 + keypoint_count * (13 + sinf_flops + cosf_flops + descriptor_ops())

flops_util['ez'] = dict()
flops_util['ez']['Downscale'] = lambda w, h: 0 # Conducts only memcpy
flops_util['ez']['Convolution'] = lambda w, h: 4 * w * h * convolution_kernel_size
flops_util['ez']['Octaves'] = lambda w, h: 0 # Conducts only memcpy
flops_util['ez']['GaussianKernelGeneration'] = lambda w, h: calc_gaussian_count * (powf_flops + sqrt_flops + ceilf_flops + 7 + (get_kernel_sizes(calc_gaussian_count)*(6.0+exp_flops) + calc_gaussian_count))
flops_util['ez']['GaussianPyramid'] = lambda w, h: flops_util['ez']['GaussianKernelGeneration'](w,h) +  ((calc_gaussian_count-1) * calc_octave_count + 1) * flops_util['ez']['Convolution'](w,h)
flops_util['ez']['DOGPyramid'] = lambda w, h: diff_pyr_ops(w,h)
flops_util['ez']['GradientAndRotationPyramids'] = lambda w, h: gr_pyr_ops(w, h)
flops_util['ez']['Histogram'] = lambda w, h:  11 + histogram_window_size * histogram_window_size * (18.0 + exp_flops) + bin_count * 10
flops_util['ez']['ExtremaRefinement'] = lambda w, h: 477 + 2* powf_flops
flops_util['ez']['KeypointDetection'] = lambda w, h:     w*h
flops_util['ez']['ExtractDescriptor'] = lambda w, h:     2 + keypoint_count * (13 + sinf_flops + cosf_flops + descriptor_ops())

def get_kernel_sizes(gaussian_count):
    kernel_sizes = np.zeros(gaussian_count)
    k = pow(2.0, 1.0 / calc_layers)

    sigma_pre = 0.5 # Default value set in settings.c
    sigma0 = 1.6 # Default value set in settings.c

    sigma_i = np.sqrt(sigma0 * sigma0 - sigma_pre * sigma_pre)
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

def diff_pyr_ops(w, h):
    width = w
    height = h
    ops = 0
    for _x in range(calc_octave_count):
        ops += (5 * width * height)
        width /= 2
        height /= 2
    return ops

def gr_pyr_ops(w, h):
    width = w
    height = h
    ops = 0
    for _x in range(calc_octave_count):
        ops += (3 * width * height * (2 + (4 * calc_get_pixel_f) + sqrt_flops +  calc_fast_ata2_f_worst_case))
        width /= 2
        height /= 2
    return ops

def descriptor_ops():

    nSubregion = descriptor_width
    subregion_width = descriptor_size * approx_kpt_scale
    nBins = nSubregion * nSubregion * descriptor_bin
    win_size = 1.414213562373095 * subregion_width * (nSubregion + 1) * 0.5 + 0.5
        
    left = -win_size
    right = win_size
    top = -win_size
    bottom = win_size

    ops = 0

    for _x in range(top,1,bottom):
        for _y in range(left,1,right):
                ops += 14
                ops += (3 * floor_flops)
                ops += 7
                ops += exp_flops
                ops += 23

    ops += nBins
    ops += nBins(minf_flops + 2)
    ops += nBins

    return ops

                