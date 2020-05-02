flops_util = dict()

flops_util['eth'] = dict()
flops_util['eth']['Downscale'] = lambda  w, h:     1 # Conducts only memcpy
flops_util['eth']['Convolution'] = lambda w, h:     w*h
flops_util['eth']['Octaves'] = lambda w, h:     w*h
flops_util['eth']['GaussianKernelGeneration'] = lambda w, h:     w*h
flops_util['eth']['GaussianPyramid'] = lambda w, h:     w*h
flops_util['eth']['DOGPyramid'] = lambda w, h:     w*h
flops_util['eth']['GradientAndRotationPyramids'] = lambda w, h:     w*h
flops_util['eth']['Histogram'] = lambda w, h:     w*h
flops_util['eth']['ExtremaRefinement'] = lambda w, h:     w*h
flops_util['eth']['KeypointDetection'] = lambda w, h:     w*h
flops_util['eth']['ExtractDescriptor'] = lambda w, h:     w*h

flops_util['ez'] = dict()
flops_util['ez']['Downscale'] = lambda w, h:      1 
flops_util['ez']['Convolution'] = lambda w, h:     w*h
flops_util['ez']['Octaves'] = lambda w, h:     w*h
flops_util['ez']['GaussianKernelGeneration'] = lambda w, h:     w*h
flops_util['ez']['GaussianPyramid'] = lambda w, h:     w*h
flops_util['ez']['DOGPyramid'] = lambda w, h:     w*h
flops_util['ez']['GradientAndRotationPyramids'] = lambda w, h:     w*h
flops_util['ez']['Histogram'] = lambda w, h:     w*h
flops_util['ez']['ExtremaRefinement'] = lambda w, h:     w*h
flops_util['ez']['KeypointDetection'] = lambda w, h:     w*h
flops_util['ez']['ExtractDescriptor'] = lambda w, h:     w*h