#include "tester.h"

define_test(ethMeasureDownscale, 1, {
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
      fail("Failed to allocate downscaled image");

    // Warm up cache
    for (int i = 0; i < NR_RUNS; ++i) {
        if (!ethsift_downscale_half(eth_img, eth_img_downscaled))
            fail("Failed to downscale image!");
    }
    // Measure
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
              ethsift_downscale_half(eth_img, eth_img_downscaled);
            });
    }
  })


define_test(ethMeasureConvolution, 1, {  
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

    // Measure performance of ethsift
    for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_apply_kernel(eth_img, kernel, kernel_size, kernel_rad, output);
    }

    // Measure performance of ethsift
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_apply_kernel(eth_img, kernel, kernel_size, kernel_rad, output);
            });
    }
  })

define_test(ethMeasureOctaves, 1, {
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

    //Warmup cache
    for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);
    }

    //Measure   
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);
            });
    }

    ethsift_free_pyramid(eth_octaves);
  })


define_test(ethMeasureGaussianKernelGeneration, 1, {
    //Create Kernels for ethSift
    int layers_count = GAUSSIAN_COUNT - 3;
    
    float* kernel_ptrs[GAUSSIAN_COUNT]; 
    int kernel_rads[GAUSSIAN_COUNT];
    int kernel_sizes[GAUSSIAN_COUNT];


    for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_generate_all_kernels(layers_count, GAUSSIAN_COUNT, kernel_ptrs, kernel_rads, kernel_sizes);
    }

    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_all_kernels(layers_count, GAUSSIAN_COUNT, kernel_ptrs, kernel_rads, kernel_sizes);
            });
    }
  })

define_test(ethMeasureGaussianPyramid, 1, {
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

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    // Create gaussians for ethSift
    for (int i = 0; i < NR_RUNS; ++i) {
         ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);
    }

    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);
            });
    }

    ethsift_free_pyramid(eth_octaves);
    ethsift_free_pyramid(eth_gaussians);
    })


define_test(ethMeasureDOGPyramid, 1, {
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
    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    ethsift_allocate_pyramid(eth_octaves, eth_img.width, eth_img.height, OCTAVE_COUNT, 1);

    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

    //Create DOG for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);

    ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT);
            });
    }
    ethsift_free_pyramid(eth_octaves);
    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_differences);
  })

define_test(ethMeasureGradientPyramids, 1, {
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

    // Calculate all pyramid contents
    ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT);
    ethsift_generate_gaussian_pyramid(eth_octaves, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
            });
    }

    ethsift_free_pyramid(eth_octaves);
    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })

define_test(ethMeasureRotationPyramids, 1, {
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

    //Convert ezSIFT Gaussians to ethSIFT gaussians
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        convert_image(ez_gaussians[i * GAUSSIAN_COUNT + j], &(eth_gaussians[i * GAUSSIAN_COUNT + j]));
      }
    }

    //Warmup cache
    for (int i = 0; i < NR_RUNS; ++i) {
         ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
    }
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT);
            });
    }

    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })


define_test(ethMeasurementHistogram, 1, {
  char const* file = data_file("lena.pgm");
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

  struct ethsift_image eth_gradients[OCTAVE_COUNT * GAUSSIAN_COUNT];
  ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

  struct ethsift_image eth_rotations[OCTAVE_COUNT * GAUSSIAN_COUNT];
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
  ezsift::SiftKeypoint kpt = kpt_list.front();

  // Init histogram bins
  float eth_hist[ETHSIFT_ORI_HIST_BINS];

  float eth_max_mag = 0.f;
  struct ethsift_keypoint eth_kpt = convert_keypoint(&kpt);

  // Warm up cache
  for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_compute_orientation_histogram(eth_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                            eth_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                            &eth_kpt, eth_hist, &eth_max_mag);
  }
  // Measure
  for (int i = 0; i < NR_RUNS; ++i) {
      with_measurement({
          ethsift_compute_orientation_histogram(eth_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                                eth_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                                &eth_kpt, eth_hist, &eth_max_mag);
      });
  }

  ethsift_free_pyramid(eth_gaussians);
  ethsift_free_pyramid(eth_gradients);
  ethsift_free_pyramid(eth_rotations);
})

define_test(ethMeasureExtremaRefinement, 1, {
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


    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_COUNT; ++j) {
        convert_image(ez_differences[i * DOG_COUNT + j], &eth_differences[i * DOG_COUNT + j]);
      }
    }

    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt1);
    ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt2);

    //Warmup cache
    for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt3);
    }

    //Measurement
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt3);
        });
    }
    ethsift_free_pyramid(eth_differences);
  })


define_test(ethMeasureKeypointDetection, 1, {
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

    for (int i = 0; i < NR_RUNS; ++i) {
        nKeypoints = 100;
        if (!ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints))
            fail("Computation failed");
    }

    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            nKeypoints = 100;
            ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints);
            });
    }

    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
    ethsift_free_pyramid(eth_differences);
  })

define_test(ethMeasureExtractDescriptor, 1, {
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

    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < GAUSSIAN_COUNT; ++j) {
        convert_image(ez_gradients[i * GAUSSIAN_COUNT + j], &eth_gradients[i * GAUSSIAN_COUNT + j]);
        convert_image(ez_rotations[i * GAUSSIAN_COUNT + j], &eth_rotations[i * GAUSSIAN_COUNT + j]);
      }
    }

    // Warmup cache
    for (int i = 0; i < NR_RUNS; ++i) {
        ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, keypoint_count);
    }

    // Measure
    for (int i = 0; i < NR_RUNS; ++i) {
        with_measurement({
            ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, keypoint_count);
            });
    }

    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })
