#include "tester.h"

// define_test(Dummy, {
//     return 1;
//   })

define_test(TestCompareImageApprox, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  
  if(ez_img.read_pgm(file) != 0) {
    return 0;
  } 
  if(!convert_image(ez_img, &eth_img)){
    return 0;
  }
  return compare_image_approx(ez_img, eth_img);
})

define_test(TestDownscale, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  
  if(ez_img.read_pgm(file) != 0) {
    return 0;
  } 
  if(!convert_image(ez_img, &eth_img)){
    return 0;
  }  
  //Downscale ETH Image
  int srcW = eth_img.width;
  int srcH = eth_img.height;
  int dstW = srcW >> 1;
  int dstH = srcH >> 1;
  struct ethsift_image eth_img_downscaled = allocate_image(dstW, dstH);
  if(!eth_img_downscaled.pixels) return 0;
  ethsift_downscale_half(eth_img, eth_img_downscaled);

  //Downscale ezSIFT Image
  const ezsift::Image<unsigned char> ez_img_downscaled = ez_img.downsample_2x();
  
  //compare files 
  int res = compare_image_approx(ez_img_downscaled, eth_img_downscaled);
  return res;
  })


define_test(TestConvolution, {  
    char const *file = data_file("lena.pgm");
    // init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;

    // constant variables used
    int w = eth_img.width;
    int h = eth_img.height;

    int kernel_size = 9;
    int kernel_rad = 4;
    float sigma = 4.5;
    
    // Create kernel
    float *kernel = (float*) malloc(kernel_size * sizeof(float)); 
    ethsift_generate_gaussian_kernel(kernel, kernel_size, kernel_rad, sigma);

    // Blur ethsift image
    struct ethsift_image output = allocate_image(w, h);
    ethsift_apply_kernel(eth_img, kernel, kernel_size, kernel_rad, output);

    // // Write blurred ethsift image
    // unsigned char* test_img = (unsigned char *)malloc( w * h *sizeof(unsigned char));
    // for (int i = 0; i < h; ++i) {
    //   for (int j = 0; j < w; ++j) {
    //     test_img[i * w + j] = (unsigned char) (output.pixels[i * w + j]);
    //   }
    // }
    // ezsift::write_pgm("eth.pgm", test_img, w, h );

    // Blur ezsift image
    std::vector<float> ez_kernel;
    for (int i = 0; i < kernel_size; ++i) {
      ez_kernel.push_back(kernel[i]);
    }
    ezsift::Image<float> ez_img_blurred(w, h);
    ezsift::gaussian_blur(ez_img.to_float(), ez_img_blurred, ez_kernel);

    // // Write blurred ezsift image
    // ez_img_blurred.write_pgm("ez.pgm");
    
    int res = compare_image_approx(ez_img_blurred, output);
    return res;
  })

define_test(TestOctaves, {
  
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;     
    if(!convert_image(ez_img, &eth_img)) return 0;

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;
    eth_octaves[0] = allocate_image(dstW, dstH);
    for(int i = 1; i < OCTAVE_COUNT; ++i){
      eth_octaves[i] = {0};
      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
      eth_octaves[i] = allocate_image(dstW, dstH);
    }

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    //Create Octaves for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    int res = 0;
    //Compare obtained Octaves in a loop
    for(int i = 0; i < OCTAVE_COUNT; ++i){
      res += compare_image_approx(ez_octaves[i], eth_octaves[i]);
    }
    //printf("TEST OCTAVE RETURN: %d\n", res);
    if(res == OCTAVE_COUNT)
      return 1;
    else return 0;
  })


define_test(TestGaussianKernelGeneration, {
    //Create Kernels for ethSift  
    
    int layers_count = GAUSSIAN_COUNT - 3;
    
    
    float* kernel_ptrs[GAUSSIAN_COUNT]; 
    int kernel_rads[GAUSSIAN_COUNT];
    int kernel_sizes[GAUSSIAN_COUNT];  
    ethsift_generate_all_kernels(layers_count, GAUSSIAN_COUNT, kernel_ptrs, kernel_rads, kernel_sizes);

    //Create Kernels for ezSift    

    std::vector<std::vector<float>> ez_kernels = ezsift::compute_gaussian_coefs(OCTAVE_COUNT, GAUSSIAN_COUNT);

    if(ez_kernels.size() != GAUSSIAN_COUNT){
      printf("TestGaussianKernelGeneration Error: gaussian_count %d and ez_kernels size %d do not match", GAUSSIAN_COUNT, (int)ez_kernels.size());
    }
    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < GAUSSIAN_COUNT; ++i) {
      //printf("Iteration %d; Kernel size = %d\n",i,(int)kernel_sizes[i]);
      res += compare_kernel( ez_kernels[i], kernel_ptrs[i], kernel_sizes[i]);
    }

    ethsift_free_kernels(kernel_ptrs, GAUSSIAN_COUNT);
    printf("Resulted in : %d", res);
    if(res == GAUSSIAN_COUNT) return 1;
    else return 0;
  })


define_test(TestGaussianPyramid, {

    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;
    eth_octaves[0] = allocate_image(dstW, dstH);
    for(int i = 1; i < OCTAVE_COUNT; ++i){
      eth_octaves[i] = {0};
      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
      eth_octaves[i] = allocate_image(dstW, dstH);
    }

    srcW = eth_img.width; 
    srcH = eth_img.height;
    dstW = srcW;
    dstH = srcH;
    // Allocate the gaussian pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        eth_gaussians[i*GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    ethsift_generate_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    //Create Octaves for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);


    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {        
        res += compare_image_approx(ez_gaussians[i*OCTAVE_COUNT + j], eth_gaussians[i*OCTAVE_COUNT + j]);
      }
    }

    if(res == OCTAVE_COUNT*GAUSSIAN_COUNT) return 1;
    else return 0;
  })


  define_test(TestDOGPyramid, {
    
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;
    
    eth_octaves[0] = allocate_image(dstW, dstH);
    for(int i = 1; i < OCTAVE_COUNT; ++i){
      eth_octaves[i] = {0};
      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
      eth_octaves[i] = allocate_image(dstW, dstH);
    }

    srcW = eth_img.width; 
    srcH = eth_img.height;
    dstW = srcW;
    dstH = srcH;
    // Allocate the gaussian pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        eth_gaussians[i*GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    srcW = eth_img.width; 
    srcH = eth_img.height;
    dstW = srcW;
    dstH = srcH;
    
    // Allocate the gaussian pyramids!
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_LAYERS];
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_LAYERS; ++j) {
        eth_differences[i * DOG_LAYERS + j] = allocate_image(dstW, dstH);
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    //Create DOG for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    ethsift_generate_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_LAYERS, OCTAVE_COUNT);

    //Create DOG for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_LAYERS);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_LAYERS);

    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_LAYERS; ++j) {        
        res += compare_image_approx(ez_differences[i * DOG_LAYERS + j], eth_differences[i * DOG_LAYERS + j]);
      }
    }

    if(res == OCTAVE_COUNT * DOG_LAYERS) return 1;
    else return 0;


  })