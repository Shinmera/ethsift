#include "tester.h"

#if RUN_ETHSIFT_MEASUREMENTS

define_test(eth_Downscale, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");
    
    //Downscale ETH Image
    struct ethsift_image eth_img_downscaled = allocate_image(eth_img.width >> 1, eth_img.height >> 1);
    if(!eth_img_downscaled.pixels)
      fail("Failed to allocate downscaled image");

    with_repeating(if (!ethsift_downscale_half(eth_img, eth_img_downscaled))
                     fail("Failed to downscale image!"));
  })

define_test(eth_Convolution, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");

    // constant variables used
    int kernel_size = 9;
    int kernel_rad = 4;
    float sigma = 4.5;
    
    // Create kernel
    float *kernel = (float*) malloc(kernel_size * sizeof(float)); 
    ethsift_generate_gaussian_kernel(kernel, kernel_size, kernel_rad, sigma);
    
    // Blur ethsift image
    struct ethsift_image output = allocate_image(eth_img.width, eth_img.height);

    with_repeating(ethsift_apply_kernel(eth_img, kernel, kernel_size, kernel_rad, output));
  })

define_test(eth_Octaves, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    struct ethsift_image eth_octaves[OCTAVE_COUNT];
    ethsift_allocate_pyramid(eth_octaves, eth_img.width, eth_img.height, OCTAVE_COUNT, 1);

    with_repeating(ethsift_generate_octaves(eth_img, eth_octaves, OCTAVE_COUNT));

    ethsift_free_pyramid(eth_octaves);
  })

define_test(eth_GaussianKernelGeneration, 1, {
    //Create Kernels for ethSift
    int layers_count = GAUSSIAN_COUNT - 3;
    
    float* kernel_ptrs[GAUSSIAN_COUNT]; 
    int kernel_rads[GAUSSIAN_COUNT];
    int kernel_sizes[GAUSSIAN_COUNT];

    with_repeating(ethsift_generate_all_kernels(layers_count, GAUSSIAN_COUNT, kernel_ptrs, kernel_rads, kernel_sizes));
  })

define_test(eth_GaussianPyramid, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");
    
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    //Create Octaves for ethSift
    with_repeating(ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT));
    
    ethsift_free_pyramid(eth_gaussians);
  })

define_test(eth_DOGPyramid, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

    //Create DOG for ethSift    
    ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    with_repeating(ethsift_generate_difference_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_differences, DOG_COUNT, OCTAVE_COUNT));
    
    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_differences);
  })

define_test(eth_GradientAndRotationPyramids, 1, {
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img))
      fail("Failed to load image");
       
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Calculate all pyramid contents
    ethsift_generate_gaussian_pyramid(eth_img, OCTAVE_COUNT, eth_gaussians, GAUSSIAN_COUNT);

    with_repeating(ethsift_generate_gradient_pyramid(eth_gaussians, GAUSSIAN_COUNT, eth_gradients, eth_rotations, GRAD_ROT_LAYERS, OCTAVE_COUNT));

    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })


define_test(eth_Histogram, 1, {
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img, &ez_img))
      fail("Failed to load image");
  
    // Allocate the pyramids!
    struct ethsift_image eth_gaussians[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gaussians, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_gradients[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT * GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

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

    with_repeating(ethsift_compute_orientation_histogram(eth_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                                         eth_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                                         &eth_kpt, eth_hist, &eth_max_mag));

    ethsift_free_pyramid(eth_gaussians);
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })

define_test(eth_ExtremaRefinement, 1, {
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img, &ez_img))
      fail("Failed to load image");

    // Allocate the gaussian pyramids!
    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

  
 
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);
    struct ethsift_keypoint eth_kpt;

    eth_kpt = convert_keypoint(&ez_kpt_list.front());

    // Convert ezsift images to ethsift images:
    for (int i = 0; i < OCTAVE_COUNT; ++i) {
      for (int j = 0; j < DOG_COUNT; ++j) {
        convert_image(ez_differences[i * DOG_COUNT + j], &eth_differences[i * DOG_COUNT + j]);
      }
    }

    with_repeating(ethsift_refine_local_extrema(eth_differences, OCTAVE_COUNT, GAUSSIAN_COUNT, &eth_kpt));
    
    ethsift_free_pyramid(eth_differences);
  })

define_test(eth_KeypointDetection, 1, {
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img, &ez_img))
      fail("Failed to load image");

    // Allocate the pyramids!
    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_differences[OCTAVE_COUNT*DOG_COUNT];
    ethsift_allocate_pyramid(eth_differences, eth_img.width, eth_img.height, OCTAVE_COUNT, DOG_COUNT);

    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

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
    uint32_t nKeypoints = ETHSIFT_MAX_TRACKABLE_KEYPOINTS;
    struct ethsift_keypoint eth_kpt_list[nKeypoints];

    with_repeating({
        nKeypoints = ETHSIFT_MAX_TRACKABLE_KEYPOINTS;
        if (!ethsift_detect_keypoints(eth_differences, eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, &nKeypoints))
          fail("Computation failed");
      });
    
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
    ethsift_free_pyramid(eth_differences);
  })

define_test(eth_ExtractDescriptor, 1, {
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img, &ez_img))
      fail("Failed to load image");
        
    // Allocate the pyramids!
    struct ethsift_image eth_gradients[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_gradients, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);

    struct ethsift_image eth_rotations[OCTAVE_COUNT*GAUSSIAN_COUNT];
    ethsift_allocate_pyramid(eth_rotations, eth_img.width, eth_img.height, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);

    // Ethsift descriptor extraction:
    const uint32_t keypoint_count = (uint32_t)ez_kpt_list.size();
    struct ethsift_keypoint eth_kpt_list[ETHSIFT_MAX_TRACKABLE_KEYPOINTS];

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

    with_repeating(ethsift_extract_descriptor(eth_gradients, eth_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, eth_kpt_list, keypoint_count));
    
    ethsift_free_pyramid(eth_gradients);
    ethsift_free_pyramid(eth_rotations);
  })

define_test(eth_MeasureFull, 1, {
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(!load_image(get_testimg_path(), eth_img, &ez_img))
      fail("Failed to load image");
    
    uint32_t keypoint_count = 2048;
    struct ethsift_keypoint keypoints[keypoint_count] = {0};

    with_repeating(ethsift_compute_keypoints(eth_img, keypoints, &keypoint_count))
  })
  
#endif
