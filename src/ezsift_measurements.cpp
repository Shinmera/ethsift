#include "tester.h"

define_test(ezMeasureDownscale, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

    //Downscale ezSIFT Image
    ezsift::Image<unsigned char> ez_img_downscaled;
    with_repeating(ez_img_downscaled = ez_img.downsample_2x());
  })

define_test(ezMeasureConvolution, 1, {  
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
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
  })

define_test(ezMeasureOctaves, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    with_repeating(build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT));
  })

define_test(ezMeasureGaussianPyramid, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

    //Init ezSIFT Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    //Init ezSIFT Gaussians
    std::vector<ezsift::Image<float> > ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);

    //Warmup cache
    for (int i = 0; i < NR_RUNS; ++i) {
      ez_gaussians.clear();
      build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);
      build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);
    }
    //Measure
    for (int i = 0; i < NR_RUNS; ++i) {
      ez_gaussians.clear();
      build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);
      with_measurement({
          build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);
        });
    }
  })

define_test(ezMeasureDOGPyramid, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

    //Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);

    //Create DOG for ezSift    
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    std::vector<ezsift::Image<float>> ez_differences(OCTAVE_COUNT * DOG_COUNT);

    with_repeating(build_dog_pyr(ez_gaussians, ez_differences, OCTAVE_COUNT, DOG_COUNT));
  })

define_test(ezMeasureGradientPyramids, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0) 
      fail("Failed to read image");

    // Init EZSIFT Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    // Init EZSIFT Gaussians
    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Init EZSIFT Gradients and Rotations
    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);

    with_repeating(build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS));
  })

define_test(ezMeasureRotationPyramids, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

    // Init Octaves
    std::vector<ezsift::Image<unsigned char > > ez_octaves(OCTAVE_COUNT);   
    build_octaves(ez_img, ez_octaves, 0, OCTAVE_COUNT);

    std::vector<ezsift::Image<float>> ez_gaussians(OCTAVE_COUNT * GAUSSIAN_COUNT);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, OCTAVE_COUNT, GAUSSIAN_COUNT);

    // Build Rotations for ezSIFT
    std::vector<ezsift::Image<float>> ez_gradients(OCTAVE_COUNT * GAUSSIAN_COUNT);
    std::vector<ezsift::Image<float>> ez_rotations(OCTAVE_COUNT * GAUSSIAN_COUNT);

    //Calculate Rotations using results from ezSIFT
    with_repeating(build_grd_rot_pyr(ez_gaussians, ez_gradients, ez_rotations, OCTAVE_COUNT, GRAD_ROT_LAYERS));
  })

define_test(ezMeasurementOneHistogram, 1, {
    ezsift::Image<unsigned char> ez_img;
    if (ez_img.read_pgm(data_file("lena.pgm")) != 0)
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
  })

define_test(ezMeasureExtremaRefinement, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");

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
  
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt1);
    refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt2);

    with_repeating(refine_local_extrema(ez_differences, OCTAVE_COUNT, DOG_COUNT, kpt3));
  })

define_test(ezMeasureKeypointDetection, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
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

define_test(ezMeasureExtractDescriptor, 1, {
    ezsift::Image<unsigned char> ez_img;
    if(ez_img.read_pgm(data_file("lena.pgm")) != 0)
      fail("Failed to read image");
    
    // Init pyramids
    std::vector<ezsift::Image<float>> ez_differences, ez_gradients, ez_rotations;
    build_ezsift_pyramids(ez_img, ez_differences, ez_gradients, ez_rotations);

    // EzSift: Detect keypoints
    std::list<ezsift::SiftKeypoint> ez_kpt_list;
    detect_keypoints(ez_differences, ez_gradients, ez_rotations, OCTAVE_COUNT, DOG_COUNT, ez_kpt_list);

    with_repeating(extract_descriptor(ez_gradients, ez_rotations, OCTAVE_COUNT, GAUSSIAN_COUNT, ez_kpt_list));
  })

