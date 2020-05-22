#include "tester.h"

#if RUN_EZSIFT_MEASUREMENTS

define_test(ez_Downscale, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    //Downscale ezSIFT Image
    ezsift::Image<unsigned char> ez_img_downscaled;
    with_repeating(ez_img_downscaled = ez_img.downsample_2x());
  })

define_test(ez_Convolution, 1, {  
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    // constant variables used
    int kernel_size = 9;
    int kernel_rad = 4;
    float sigma = 4.5;

    // Create kernel
    float* kernel = (float*)malloc(kernel_size * sizeof(float));
    ethsift_generate_gaussian_kernel(kernel, kernel_size, kernel_rad, sigma);

    // Blur ezsift image
    std::vector<float> ez_kernel;
    for (int i = 0; i < kernel_size; ++i) {
      ez_kernel.push_back(kernel[i]);
    }
    ezsift::Image<float> ez_img_blurred(ez_img.w, ez_img.h);
    with_repeating(ezsift::gaussian_blur(ez_img.to_float(), ez_img_blurred, ez_kernel));
    free(kernel);
  })

define_test(ez_Octaves, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    with_repeating(build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT));
  })

define_test(ez_GaussianPyramid, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    std::vector<ezsift::Image<unsigned char>> ez_octaves(OCTAVE_COUNT, ezsift::Image<unsigned char>());
    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());
    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    with_repeating(build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT));
  })

define_test(ez_DOGPyramid, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    std::vector<ezsift::Image<unsigned char>> ez_octaves(OCTAVE_COUNT, ezsift::Image<unsigned char>());
    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());
    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());

    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);
    
    with_repeating(build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT));
  })

define_test(ez_GradientAndRotationPyramids, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0) 
      fail("Failed to read image");

    std::vector<ezsift::Image<unsigned char>> ez_octaves(OCTAVE_COUNT, ezsift::Image<unsigned char>());
    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());
    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT, ezsift::Image<float>());
    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    with_repeating(build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS));
  })

define_test(ez_Histogram, 1, {
    //init files 
    ezsift::Image<unsigned char> ez_img;
    if (ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // Detect keypoints
    std::list<ezsift::SiftKeypoint> kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, kpt_list);

    // Init histogram bins
    float* ez_hist = new float[ETHSIFT_ORI_HIST_BINS];

    // Measurement histograms: keypoint for measurement (Maybe randomize?)
    ezsift::SiftKeypoint kpt = kpt_list.front();
    // Calculate histograms
    with_repeating(ezsift::compute_orientation_hist_with_gradient(ez_gradients[kpt.octave * GAUSSIAN_COUNT + kpt.layer],
                                                                  ez_rotations[kpt.octave * GAUSSIAN_COUNT + kpt.layer], kpt, ez_hist));
    delete[] ez_hist;
  })

define_test(ez_ExtremaRefinement, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // Detect keypoints
    std::list<ezsift::SiftKeypoint> kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, kpt_list);
    ezsift::SiftKeypoint kpt = kpt_list.front();

    with_repeating(refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt));
  })

define_test(ez_KeypointDetection, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;

    // std::list is O(n) to clear, meaning it will actually impact measurement, so we can't
    // simply use with_repeating here. That sucks.
    for (int i = 0; i < NR_RUNS; ++i) {
      ez_kpt_list.clear();
      detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);
    }

    for (int i = 0; i < NR_RUNS; ++i) {
      ez_kpt_list.clear();
      with_measurement({
          detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);
        });
    }
  })

define_test(ez_ExtractDescriptor, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");
    
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);

    with_repeating(extract_descriptor(ez_gradients, ez_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, ez_kpt_list));
  })

define_test(ez_MeasureFull, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(get_testimg_path()) != 0)
      fail("Failed to read image");

    std::list<ezsift::SiftKeypoint> kpt_list;
    with_repeating({
        ezsift::sift_cpu(ez_img, kpt_list, true);
        kpt_list.clear();
      })
  })

#endif
