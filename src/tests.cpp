#include "tester.h"

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
        
    // Blur ezsift image
    std::vector<float> ez_kernel;
    for (int i = 0; i < kernel_size; ++i) {
      ez_kernel.push_back(kernel[i]);
    }
    ezsift::Image<float> ez_img_blurred(w, h);
    ezsift::gaussian_blur(ez_img.to_float(), ez_img_blurred, ez_kernel);
        
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


    struct ethsift_image eth_octaves[OCTAVE_COUNT];

    uint32_t srcW = eth_img.width; 
    uint32_t srcH = eth_img.height;
    uint32_t dstW = srcW;
    uint32_t dstH = srcH;
    // Allocate the gaussian pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      eth_octaves[i] = allocate_image(dstW, dstH);
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
    // Create gaussians for ethSift
    ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    //Init ezSIFT Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);
    //Init ezSIFT Gaussians
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

    ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

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

define_test(TestGradientPyramids, {
    
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;


    // Allocate the gaussian pyramids!
    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];


    uint32_t srcW = eth_img.width; 
    uint32_t srcH = eth_img.height;
    uint32_t dstW = srcW;
    uint32_t dstH = srcH;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      eth_octaves[i] = allocate_image(dstW, dstH);
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        eth_gaussians[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
        eth_gradients[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
        eth_rotations[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);      
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
    
    // Init EZSIFT Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    // Init EZSIFT Gaussians
    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Init EZSIFT Gradients and Rotations
    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

    // Compare the Gradient outputs!
    int res_g = 0;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 1; j <= GRAD_ROT_LAYERS; ++j) {        
        res_g += compare_image_approx(ez_gradients[i * GAUSSIAN_COUNT + j], eth_gradients[i * GAUSSIAN_COUNT + j]);
      }
    }

    if(res_g ==  OCTAVE_COUNT * GRAD_ROT_LAYERS) return 1;
    return 0;


  })

define_test(TestRotationPyramids, {
    
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;

    // Allocate the gaussian pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];

    uint32_t srcW = eth_img.width; 
    uint32_t srcH = eth_img.height;
    uint32_t dstW = srcW;
    uint32_t dstH = srcH;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        eth_gaussians[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
        eth_gradients[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
        eth_rotations[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);      
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);   
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    //Conver ezSIFT Gaussians to ethSIFT gaussians
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        convert_image(ez_gaussians[i * GAUSSIAN_COUNT + j], &(eth_gaussians[i * GAUSSIAN_COUNT + j]));
      }
    }
    //Calculate Rotations using results from ezSIFT
    ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
    

    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

    // Compare the gaussian outputs!
    int res_r = 0;
    int res_r_old;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 1; j <= GRAD_ROT_LAYERS; ++j) {        
        res_r_old = res_r;
        res_r += compare_image_approx(ez_rotations[i * GAUSSIAN_COUNT + j], eth_rotations[i * GAUSSIAN_COUNT + j]);
        if(res_r_old == res_r) printf("FAILED ON INSTANCE: %d\n",i * GAUSSIAN_COUNT + j);
      }
    }

    if(res_r ==  OCTAVE_COUNT * GRAD_ROT_LAYERS) return 1;   
    return 0;

  })


define_test(TestHistograms, {

  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  if (ez_img.read_pgm(file) != 0) return 0;
  if (!convert_image(ez_img, &eth_img)) return 0;

  // Allocate the gaussian pyramids!
  struct ethsift_image eth_gaussians[OCTAVE_COUNT*GAUSSIAN_COUNT];
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];

  uint32_t srcW = eth_img.width;
  uint32_t srcH = eth_img.height;
  uint32_t dstW = srcW;
  uint32_t dstH = srcH;
  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      eth_gaussians[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      eth_gradients[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      eth_rotations[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
    }

    srcW = dstW;
    srcH = dstH;
    dstW = srcW >> 1;
    dstH = srcH >> 1;
  }

  //Init Octaves
  std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
  build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

  std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);
 
  std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_LAYERS);
  build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_LAYERS);

  std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
  std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

  //Convert ezSIFT Gaussians, Gradients and Rotations to ethSIFT format
  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &(eth_gradients[i * GAUSSIAN_COUNT + j]));
      convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &(eth_rotations[i * GAUSSIAN_COUNT + j]));
    }
  }
   
  // Detect keypoints
  std::list<ezsift::SiftKeypoint> kpt_list;
  detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_LAYERS, kpt_list);

  // Init histogram bins
  float *ez_hist = new float[ETHSIFT_ORI_HIST_BINS];
  float eth_hist[ETHSIFT_ORI_HIST_BINS];

  // Test histograms
  for (auto kpt : kpt_list) {
    
    // Calculate histograms
    float ez_max_mag = 0.f;
    //ez_max_mag = ezsift::compute_orientation_hist(ez_img.to_float(), kpt, ez_hist);
   ez_max_mag = ezsift::compute_orientation_hist_with_gradient(
        ez_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
        ez_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer], kpt, ez_hist);
    float eth_max_mag = 0.f;
    struct ethsift_keypoint eth_kpt = convert_keypoint(&kpt);
    ethsift_compute_orientation_histogram(eth_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                          eth_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                          &eth_kpt, eth_hist, &eth_max_mag);

    // Compare resulted histograms
    float epsilon = 0.001f;
    if (abs(ez_max_mag - eth_max_mag) < epsilon) {
        for (int i = 0; i < ETHSIFT_ORI_HIST_BINS; ++i) {
            if (abs(eth_hist[i] - ez_hist[i]) < epsilon) continue;
            else {
                return 0;
            }
        }
    }
    else {
        return 0;
    }
  }
  return 1;
  })


define_test(TestExtremaRefinement, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  if(ez_img.read_pgm(file) != 0) return 0;  
  if(!convert_image(ez_img, &eth_img)) return 0;

  int srcW = eth_img.width; 
  int srcH = eth_img.height;
  int dstW = srcW;
  int dstH = srcH;

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

  // 3 random unrefined Keypoints to test refinement: 
  ezsift::SiftKeypoint kpt1;
  kpt1.octave = 3;
  kpt1.layer = 3;
  kpt1.ri = 56.0f;
  kpt1.ci = 87.0f;
  ezsift::SiftKeypoint kpt2;
  kpt2.octave = 1;
  kpt2.layer = 1;
  kpt2.ri = 109.0f;
  kpt2.ci = 378.0f;
  ezsift::SiftKeypoint kpt3;
  kpt3.octave = 0;
  kpt3.layer = 1;
  kpt3.ri = 405.0f;
  kpt3.ci = 489.0f;

  struct ethsift_keypoint eth_kpt1;
  eth_kpt1.layer = 3;
  eth_kpt1.octave = 3;
  eth_kpt1.layer_pos.x = 56.0f;
  eth_kpt1.layer_pos.y = 87.0f;
  struct ethsift_keypoint eth_kpt2;
  eth_kpt2.layer = 1;
  eth_kpt2.octave = 1;
  eth_kpt2.layer_pos.x = 109.0f;
  eth_kpt2.layer_pos.y = 378.0f;
  struct ethsift_keypoint eth_kpt3;
  eth_kpt3.layer = 1;
  eth_kpt3.octave = 0;
  eth_kpt3.layer_pos.x = 405.0f;
  eth_kpt3.layer_pos.y = 489.0f;
  
  //Init EZSift Octaves
  std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

  //Create DOG for ezSift    
  build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

  std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

  std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_LAYERS);
  build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_LAYERS);

  refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_LAYERS, kpt1);
  refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_LAYERS, kpt2);
  refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_LAYERS, kpt3);

  // Convert ezsift images to ethsift images:
  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < DOG_LAYERS; ++j) {
      convert_image(ez_differences[i * DOG_LAYERS + j], &eth_differences[i * DOG_LAYERS + j]);
    }
  }

  ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt1);
  ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt2);
  ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt3);

  int res = 1;

  res = res && (kpt1.octave == (int) eth_kpt1.octave && 
                kpt1.layer == (int) eth_kpt1.layer &&
                fabs(kpt1.ri - eth_kpt1.layer_pos.x) < EPS &&
                fabs(kpt1.ci - eth_kpt1.layer_pos.y) < EPS);
  
  res = res && (kpt2.octave == (int) eth_kpt2.octave && 
                kpt2.layer == (int) eth_kpt2.layer &&
                fabs(kpt2.ri - eth_kpt2.layer_pos.x) < EPS &&
                fabs(kpt2.ci - eth_kpt2.layer_pos.y) < EPS);

  res = res && (kpt3.octave == (int) eth_kpt3.octave && 
                kpt3.layer == (int) eth_kpt3.layer &&
                fabs(kpt3.ri - eth_kpt3.layer_pos.x) < EPS &&
                fabs(kpt3.ci - eth_kpt3.layer_pos.y) < EPS);

  return res;
})


define_test(TestKeypointDetection, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  if(ez_img.read_pgm(file) != 0) return 0;  
  if(!convert_image(ez_img, &eth_img)) return 0;

  int srcW = eth_img.width; 
  int srcH = eth_img.height;
  int dstW = srcW;
  int dstH = srcH;

  // Allocate the gaussian pyramids!
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
  struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_LAYERS];

  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      eth_gradients[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      eth_rotations[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);      
    }

    for (int j = 0; j < DOG_LAYERS; ++j) {
      eth_differences[i * DOG_LAYERS + j] = allocate_image(dstW, dstH);
    }

    srcW = dstW;
    srcH = dstH;
    dstW = srcW >> 1;
    dstH = srcH >> 1;
  }
  
  //Init EZSift Octaves
  std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

  //Create DOG for ezSift    
  build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

  std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

  std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_LAYERS);
  build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_LAYERS);

  std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
  std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

  // EzSift: Detect keypoints
  std::list<ezsift::SiftKeypoint> ez_kpt_list;
  detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_LAYERS, ez_kpt_list);

  // Convert ezsift images to ethsift images:
  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &eth_gradients[i * GAUSSIAN_COUNT + j]);
      convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &eth_rotations[i * GAUSSIAN_COUNT + j]);   
    }

    for (int j = 0; j < DOG_LAYERS; ++j) {
      convert_image(ez_differences[i * DOG_LAYERS + j], &eth_differences[i * DOG_LAYERS + j]);
    }
  }

  // Ethsift keypoint detection:
  struct ethsift_keypoint eth_kpt_list[100];
  uint32_t nKeypoints = 100;
  ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);

  int res = 1;

  if (ez_kpt_list.size() != nKeypoints) {
    // Test if methods would have found same amount of keypoints
    res = 0;
  } else {
    int i = 0;
    for (ezsift::SiftKeypoint kpt : ez_kpt_list) {
      // Returned values are identical using identical inputs
      res = res && (((int) eth_kpt_list[i].octave) == kpt.octave &&
                    ((int) eth_kpt_list[i].layer) == kpt.layer &&
                    eth_kpt_list[i].layer_pos.x == kpt.ri &&
                    eth_kpt_list[i].layer_pos.y == kpt.ci &&
                    eth_kpt_list[i].layer_pos.scale == kpt.layer_scale &&
                    eth_kpt_list[i].global_pos.x == kpt.r &&
                    eth_kpt_list[i].global_pos.y == kpt.c &&
                    eth_kpt_list[i].global_pos.scale == kpt.scale &&
                    eth_kpt_list[i].orientation == kpt.ori &&
                    eth_kpt_list[i].magnitude == kpt.mag);
      
      ++i;

      if (i == 100) {
        // Because we only collect 100 keypoints
        break;
      }
    }
  } 
  
  return res;
})


define_test(TestExtractDescriptor, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  if (ez_img.read_pgm(file) != 0) return 0;
  if (!convert_image(ez_img, &eth_img)) return 0;

  int srcW = eth_img.width;
  int srcH = eth_img.height;
  int dstW = srcW;
  int dstH = srcH;

  // Allocate the gaussian pyramids!
  struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
  struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];

  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      eth_gradients[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
      eth_rotations[i * GAUSSIAN_COUNT + j] = allocate_image(dstW, dstH);
    }
    srcW = dstW;
    srcH = dstH;
    dstW = srcW >> 1;
    dstH = srcH >> 1;
  }

  //Init EZSift Octaves
  std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

  //Create DOG for ezSift    
  build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

  std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

  std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_LAYERS);
  build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_LAYERS);

  std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
  std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
  build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

  // EzSift: Detect keypoints
  std::list<ezsift::SiftKeypoint> ez_kpt_list;
  detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_LAYERS, ez_kpt_list);

  // EzSift: Extract Descriptors
  extract_descriptor(ez_gradients, ez_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, ez_kpt_list);

  // Convert ezsift images to ethsift images:
  for (int i = 0; i < OCTAVE_COUNT; ++i) {
    for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
      convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &eth_gradients[i * GAUSSIAN_COUNT + j]);
      convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &eth_rotations[i * GAUSSIAN_COUNT + j]);
    }
  }

  // Ethsift descriptor extraction:
  const uint32_t keypoint_count = (uint32_t) ez_kpt_list.size();
  struct ethsift_keypoint eth_kpt_list[150];

  int i = 0;
  // Convert ezsift Keypoints to ethsift
  for (auto ez_kpt : ez_kpt_list) {
    eth_kpt_list[i] = convert_keypoint(&ez_kpt);
    ++i;
  }

  // Compute our descriptors which we want to test!
  ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, keypoint_count);

  i = 0;
  // Compare descriptors for correctness
  for (auto ez_kpt : ez_kpt_list) {
    // Returned values are identical using identical inputs
    if (!compare_descriptor(ez_kpt.descriptors, eth_kpt_list[i].descriptors)) return 0;
    ++i;
  }

  return 1;
  })