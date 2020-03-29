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


// define_test(TestConvolution, {  
//     char const *file = data_file("lena.pgm");
//     init files 
//     ezsift::Image<unsigned char> ez_img;
//     struct ethsift_image eth_img = {0};
//     if(ez_img.read_pgm(file) != 0) return 0;  
//     if(!convert_image(ez_img, &eth_img)) return 0;
//     //TODO: implement TEST

//     return 1;
//   })

define_test(TestOctaves, {
  
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;     
    if(!convert_image(ez_img, &eth_img)) return 0;

    //Init Octaves
    const int nOctaves = (int)std::log2((float)fmin(ez_img.w, ez_img.h)) - 3; // 2 or 3, need further research    

    std::vector<ezsift::Image<unsigned char > > ez_octaves(nOctaves);

    struct ethsift_image eth_octaves[nOctaves];
    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;
    eth_octaves[0] = allocate_image(dstW, dstH);
    for(int i = 1; i < nOctaves; ++i){
      eth_octaves[i] = {0};
      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
      eth_octaves[i] = allocate_image(dstW, dstH);
    }

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, nOctaves);

    //Create Octaves for ezSift    
    build_octaves(ez_img, ez_octaves, 0, nOctaves);

    int res = 0;
    //Compare obtained Octaves in a loop
    for(int i = 0; i < nOctaves; ++i){
      res += compare_image_approx(ez_octaves[i], eth_octaves[i]);
    }
    //printf("TEST OCTAVE RETURN: %d\n", res);
    if(res == nOctaves)
      return 1;
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
    const int nGpyrLayers = 6;
    const int nOctaves = (int)std::log2((float)fmin(ez_img.w, ez_img.h)) - 3; // 2 or 3, need further research    

    std::vector<ezsift::Image<unsigned char > > ez_octaves(nOctaves);

    struct ethsift_image eth_octaves[nOctaves];
    int srcW = eth_img.width; 
    int srcH = eth_img.height;
    int dstW = srcW;
    int dstH = srcH;
    eth_octaves[0] = allocate_image(dstW, dstH);
    for(int i = 1; i < nOctaves; ++i){
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
    struct ethsift_image eth_gaussians[nOctaves*nGpyrLayers];
    for (int i = 0; i < nOctaves; ++i) {
      for (int j = 0; j < nGpyrLayers; ++j) {
        eth_gaussians[i*nOctaves + j] = allocate_image(dstW, dstH);
      }

      srcW = dstW;
      srcH = dstH;
      dstW = srcW >> 1;
      dstH = srcH >> 1;
    }

    //Create Octaves for ethSift    
    ethsift_generate_octaves(eth_img, eth_octaves, nOctaves);

    ethsift_generate_pyramid(eth_octaves, nOctaves, eth_gaussians, nGpyrLayers);

    //Create Octaves for ezSift    
    build_octaves(ez_img, ez_octaves, 0, nOctaves);

    std::vector<ezsift::Image<float>> ez_gaussians(nOctaves * nGpyrLayers);
    build_gaussian_pyramid(ez_octaves, ez_gaussians, nOctaves, nGpyrLayers);


    // Compare the gaussian outputs!
    int res = 0;
    for (int i = 0; i < nOctaves; ++i) {
      for (int j = 0; j < nGpyrLayers; ++j) {        
        res += compare_image_approx(ez_gaussians[i*nOctaves + j], eth_gaussians[i*nOctaves + j]);
      }
    }

    if(res == nOctaves*nGpyrLayers) return 1;
    else return 0;
  })
