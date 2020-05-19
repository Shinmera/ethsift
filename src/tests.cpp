#include "tester.h"

define_test(TestCompareImageApprox, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
  
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
    return compare_image_approx(ez_img, eth_img);
  })

define_test(SimpleAllocation, 0, {
    const int pyramid_size = 5;
    uint32_t ref_w = 1024;
    uint32_t ref_h = 512;
    struct ethsift_image pyramid[pyramid_size];
    with_measurement({
        if(!ethsift_allocate_pyramid(pyramid, ref_w, ref_h, pyramid_size, 1))
          fail("Allocation failed");
      });

    for(int i=1; i<pyramid_size; ++i){
      uint32_t width = ref_w / (2 << (i-1));
      uint32_t height = ref_h / (2 << (i-1));
      if(pyramid[i].pixels == 0)
        fail("Pixel array unset");
      if(pyramid[i].width != width)
        fail("Unexpected width: %i expected %i", pyramid[i].width, width);
      if(pyramid[i].height != height)
        fail("Unexpected height: %i expected %i", pyramid[i].height, height);
      if(pyramid[i].pixels[width*height-1] != 0.0)
        fail("Unexpected value at end of array: %f", pyramid[i].pixels[width*height-1]);
    }
    ethsift_free_pyramid(pyramid);
  })

define_test(RandomAllocation, 0, {
    // Perform a bunch of randomised runs.
    for(int r=0; r<100; ++r){
      int pyramid_size = rand() % 10 + 1;
      struct ethsift_image pyramid[pyramid_size];
      uint32_t ref_w = 2 << (rand()%4 + 1 + pyramid_size);
      uint32_t ref_h = 2 << (rand()%4 + 1 + pyramid_size);
      with_measurement({
          if(!ethsift_allocate_pyramid(pyramid, ref_w, ref_h, pyramid_size, 1))
            fail("Allocation failed");
        });

      for(int i=1; i<pyramid_size; ++i){
        uint32_t width = ref_w / (2 << (i-1));
        uint32_t height = ref_h / (2 << (i-1));
        if(pyramid[i].pixels == 0)
          fail("Pixel array unset");
        if(pyramid[i].width != width)
          fail("Unexpected width: %i expected %i", pyramid[i].width, width);
        if(pyramid[i].height != height)
          fail("Unexpected height: %i expected %i", pyramid[i].height, height);
        if(pyramid[i].pixels[width*height-1] != 0.0)
          fail("Unexpected value at end of array: %f", pyramid[i].pixels[width*height-1]);
      }
      ethsift_free_pyramid(pyramid);
    }
  })


define_test(TestDownscale, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
  
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
    //Downscale ETH Image
    int srcW = eth_img.width;
    int srcH = eth_img.height;
    int dstW = srcW >> 1;
    int dstH = srcH >> 1;
    struct ethsift_image eth_img_downscaled = allocate_image(dstW, dstH);
    if(!eth_img_downscaled.pixels)
      fail("Failed to downscale image");
    ethsift_downscale_half(eth_img, eth_img_downscaled);

    //Downscale ezSIFT Image
    const ezsift::Image<unsigned char> ez_img_downscaled = ez_img.downsample_2x();
    //compare files 
    int res = compare_image_approx(ez_img_downscaled, eth_img_downscaled);
    return res;
  })


define_test(TestConvolution, 0, {
    char const *file = data_file("lena.pgm");
    // init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");

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
        
    return compare_image_approx(ez_img_blurred, output);
  })

define_test(TestOctaves, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    ethsift_allocate_pyramid(eth_octaves, eth_img.width, eth_img.height, OCTAVE_COUNT, 1);

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    //Create Octaves for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    int res = 0;
    //Compare obtained Octaves in a loop
    for(int i = 0; i < OCTAVE_COUNT; ++i){
      res += compare_image_approx(ez_octaves[i], eth_octaves[i]);
    }

    return (res == OCTAVE_COUNT);
  })


define_test(TestGaussianKernelGeneration, 0, {
    //Create Kernels for ethSift
    int layers_count = GAUSSIAN_COUNT - 3;
    
    float* kernel_ptrs[GAUSSIAN_COUNT]; 
    int kernel_rads[GAUSSIAN_COUNT];
    int kernel_sizes[GAUSSIAN_COUNT];  
    ethsift_generate_all_kernels(layers_count, GAUSSIAN_COUNT, kernel_ptrs, kernel_rads, kernel_sizes);

    //Create Kernels for ezSift
    std::vector<std::vector<float>> ez_kernels = ezsift::compute_gaussian_coefs(OCTAVE_COUNT, GAUSSIAN_COUNT);

    if(ez_kernels.size() != GAUSSIAN_COUNT)
      fail("Expected %d kernels, but got %d", GAUSSIAN_COUNT, (int)ez_kernels.size());
    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < GAUSSIAN_COUNT; ++i) {
      res += compare_kernel( ez_kernels[i], kernel_ptrs[i], kernel_sizes[i]);
    }

    ethsift_free_kernels(kernel_ptrs, GAUSSIAN_COUNT);
    return (res == GAUSSIAN_COUNT);
  })

define_test(TestGaussianPyramid, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
    
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Create gaussians for ethSift
    ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

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

    return (res == OCTAVE_COUNT*GAUSSIAN_COUNT);
  })


define_test(TestDOGPyramid, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

    //Create DOG for ethSift
    ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);

    //Create DOG for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_COUNT; ++j) {        
        res += compare_image_approx(ez_differences[i * DOG_COUNT + j], eth_differences[i * DOG_COUNT + j]);
      }
    }

    return (res == OCTAVE_COUNT * DOG_COUNT);
  })

define_test(TestGradientPyramids, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) 
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
       
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Calculate all pyramid contents
    ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

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
         if(!compare_image_approx(ez_gradients[i * GAUSSIAN_COUNT + j], eth_gradients[i * GAUSSIAN_COUNT + j])){
            printf("FAILED ON INSTANCE: %d\n",i * GAUSSIAN_COUNT + j);
         }          
         else {
          ++res_g;
         }
      }
    }

    return (res_g ==  OCTAVE_COUNT * GRAD_ROT_LAYERS);
  })

define_test(TestRotationPyramids, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");

    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

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

    return (res_r ==  OCTAVE_COUNT * GRAD_ROT_LAYERS);
  })

define_test(TestHistograms, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if (ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if (!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
       
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);
 
    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

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
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, kpt_list);

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
      if (epsilon < abs(ez_max_mag - eth_max_mag))
        fail("Maximum magnitude differs by %f at %f,%f,%f", abs(ez_max_mag - eth_max_mag), kpt.rlayer, kpt.ri, kpt.ci);
    
      for (int i = 0; i < ETHSIFT_ORI_HIST_BINS; ++i) {
        if (epsilon < abs(eth_hist[i] - ez_hist[i]))
          fail("Histograms differ by %f in bin %i at %f,%f,%f", abs(eth_hist[i] - ez_hist[i]), i, kpt.rlayer, kpt.ri, kpt.ci);
      }
    }
  })


define_test(TestExtremaRefinement, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");

    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;

    // Allocate the gaussian pyramids!
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);


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
    eth_kpt1.layer_pos.y = 56.0f;
    eth_kpt1.layer_pos.x = 87.0f;
    struct ethsift_keypoint eth_kpt2;
    eth_kpt2.layer = 1;
    eth_kpt2.octave = 1;
    eth_kpt2.layer_pos.y = 109.0f;
    eth_kpt2.layer_pos.x = 378.0f;
    struct ethsift_keypoint eth_kpt3;
    eth_kpt3.layer = 1;
    eth_kpt3.octave = 0;
    eth_kpt3.layer_pos.y = 405.0f;
    eth_kpt3.layer_pos.x = 489.0f;
  
    //Init EZSift Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    //Create DOG for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

    refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt1);
    refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt2);
    refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt3);

    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_COUNT; ++j) {
        convert_image(ez_differences[i * DOG_COUNT + j], &eth_differences[i * DOG_COUNT + j]);
      }
    }

    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt1);
    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt2);
    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt3);

    int res = 1;

    res = res && (kpt1.octave == (int) eth_kpt1.octave && 
                  kpt1.layer == (int) eth_kpt1.layer &&
                  fabs(kpt1.ri - eth_kpt1.layer_pos.y) < EPS &&
                  fabs(kpt1.ci - eth_kpt1.layer_pos.x) < EPS);
  
    res = res && (kpt2.octave == (int) eth_kpt2.octave && 
                  kpt2.layer == (int) eth_kpt2.layer &&
                  fabs(kpt2.ri - eth_kpt2.layer_pos.y) < EPS &&
                  fabs(kpt2.ci - eth_kpt2.layer_pos.x) < EPS);

    res = res && (kpt3.octave == (int) eth_kpt3.octave && 
                  kpt3.layer == (int) eth_kpt3.layer &&
                  fabs(kpt3.ri - eth_kpt3.layer_pos.y) < EPS &&
                  fabs(kpt3.ci - eth_kpt3.layer_pos.x) < EPS);

    return res;
  })


define_test(TestKeypointDetection, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
    

    // Allocate the pyramids!
    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    ethsift_allocate_pyramid(eth_octaves, eth_img.width, eth_img.height, OCTAVE_COUNT, 1);

    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

  
    //Init EZSift Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    //Create DOG for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);

    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &eth_gradients[i * GAUSSIAN_COUNT + j]);
        convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &eth_rotations[i * GAUSSIAN_COUNT + j]);   
      }

      for (int j = 0; j < DOG_COUNT; ++j) {
        convert_image(ez_differences[i * DOG_COUNT + j], &eth_differences[i * DOG_COUNT + j]);
      }
    }

    // Ethsift keypoint detection:
    struct ethsift_keypoint eth_kpt_list[100];
    uint32_t nKeypoints = 100;
    ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);

    if (ez_kpt_list.size() != nKeypoints)
      fail("No keypoints found.");
  
    int i = 0;
    for (ezsift::SiftKeypoint kpt : ez_kpt_list) {
      // Returned values are identical using identical inputs
      if(!(((int) eth_kpt_list[i].octave) == kpt.octave &&
           ((int) eth_kpt_list[i].layer) == kpt.layer &&
           abs(eth_kpt_list[i].layer_pos.y - kpt.ri) < EPS &&
           abs(eth_kpt_list[i].layer_pos.x - kpt.ci) < EPS &&
           abs(eth_kpt_list[i].layer_pos.scale - kpt.layer_scale) < EPS &&
           abs(eth_kpt_list[i].global_pos.y - kpt.r) < EPS &&
           abs(eth_kpt_list[i].global_pos.x - kpt.c) < EPS &&
           abs(eth_kpt_list[i].global_pos.scale - kpt.scale) < EPS &&
           abs(eth_kpt_list[i].orientation - kpt.ori < EPS) &&
           abs(eth_kpt_list[i].magnitude - kpt.mag < EPS)))
        fail("Keypoint mismatch at %f,%f,%f", kpt.rlayer, kpt.ri, kpt.ci);
    
      ++i;
      if (i == 100) {
        // Because we only collect 100 keypoints
        break;
      }
    }
  })

define_test(TestExtractDescriptor, 0, {
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0)
      fail("Failed to read image");
    if(!convert_image(ez_img, &eth_img))
      fail("Failed to convert image");
        
    // Allocate the pyramids!
    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    //Init EZSift Objects
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);
    build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT);

    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);

    // Ethsift descriptor extraction:
    const uint32_t keypoint_count = (uint32_t)ez_kpt_list.size();
    struct ethsift_keypoint eth_kpt_list[150];

    int i = 0;
    // Convert ezsift Keypoints to ethsift
    for (auto ez_kpt : ez_kpt_list) {
      eth_kpt_list[i] = convert_keypoint(&ez_kpt);
      ++i;
    }

    // EzSift: Extract Descriptors
    extract_descriptor(ez_gradients, ez_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, ez_kpt_list);

    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &eth_gradients[i * GAUSSIAN_COUNT + j]);
        convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &eth_rotations[i * GAUSSIAN_COUNT + j]);
      }
    }

    // Compute our descriptors which we want to test!
    ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, keypoint_count);

    i = 0;
    // Compare descriptors for correctness
    for (auto ez_kpt : ez_kpt_list) {
      // Returned values are identical using identical inputs
      if (!compare_descriptor(ez_kpt.descriptors, eth_kpt_list[i].descriptors))
        fail("Descriptor %i mismatched at %f,%f,%f", i, ez_kpt.rlayer, ez_kpt.ri, ez_kpt.ci);
      ++i;
    }
  })

define_test(TestComputeKeypoints, 0, {
  char const *file = data_file("lena.pgm");
  //init files 
  ezsift::Image<unsigned char> ez_img;
  struct ethsift_image eth_img = {0};
  if (ez_img.read_pgm(file) != 0)
    fail("Failed to read image");
  if (!convert_image(ez_img, &eth_img))
    fail("Failed to convert image");


  struct ethsift_keypoint eth_kpt_list[ETHSIFT_MAX_TRACKABLE_KEYPOINTS];
  uint32_t keypoints_tracked = ETHSIFT_MAX_TRACKABLE_KEYPOINTS;
  ethsift_compute_keypoints(eth_img, eth_kpt_list, &keypoints_tracked);

  if(keypoints_tracked != LENA_KEYPOINTS) fail("Keypoints tracked mismatched: %d != %d", keypoints_tracked, ETHSIFT_MAX_TRACKABLE_KEYPOINTS);
  })
