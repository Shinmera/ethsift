flops_util = dict()

flops_util['eth'] = dict()
flops_util['eth']['Downscale'] = lambda  n:     1 # Conducts only memcpy
flops_util['eth']['Convolution'] = lambda  n:      n
flops_util['eth']['Octaves'] = lambda  n:      n
flops_util['eth']['GaussianKernelGeneration'] = lambda  n:      n
flops_util['eth']['GaussianPyramid'] = lambda  n:      n
flops_util['eth']['DOGPyramid'] = lambda  n:      n
flops_util['eth']['GradientAndRotationPyramids'] = lambda  n:      n
flops_util['eth']['Histogram'] = lambda  n:      n
flops_util['eth']['ExtremaRefinement'] = lambda  n:      n
flops_util['eth']['KeypointDetection'] = lambda  n:      n
flops_util['eth']['ExtractDescriptor'] = lambda  n:      n

flops_util['ez'] = dict()
flops_util['ez']['Downscale'] = lambda  n:      1 
flops_util['ez']['Convolution'] = lambda  n:      n
flops_util['ez']['Octaves'] = lambda  n:      n
flops_util['ez']['GaussianKernelGeneration'] = lambda  n:      n
flops_util['ez']['GaussianPyramid'] = lambda  n:      n
flops_util['ez']['DOGPyramid'] = lambda  n:      n
flops_util['ez']['GradientAndRotationPyramids'] = lambda  n:      n
flops_util['ez']['Histogram'] = lambda  n:      n
flops_util['ez']['ExtremaRefinement'] = lambda  n:      n
flops_util['ez']['KeypointDetection'] = lambda  n:      n
flops_util['ez']['ExtractDescriptor'] = lambda  n:      n