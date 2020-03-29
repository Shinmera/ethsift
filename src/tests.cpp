#include "tester.h"

// define_test(Dummy, {
//     return 1;
//   })

<<<<<<< HEAD

define_test(TestCompareImageApprox, {
  char const *file = "../data/lena.pgm";
=======
define_test(TestDownscale, {
    char const *file = data_file("lena.pgm");
>>>>>>> 62219b9add21dc14ba0c841f979badcc60a3dc68
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
  char const *file = "../data/lena.pgm";
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

<<<<<<< HEAD
// define_test(TestConvolution, {  
//     char const *file = "./data/lena.pgm";
//     //init files 
//     ezsift::Image<unsigned char> ez_img;
//     struct ethsift_image eth_img = {0};
//     if(ez_img.read_pgm(file) != 0) return 0;  
//     if(!convert_image(ez_img, &eth_img)) return 0;
=======
define_test(TestConvolution, {  
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;
>>>>>>> 62219b9add21dc14ba0c841f979badcc60a3dc68

//     //TODO: implement TEST

//     return 1;
//   })

// define_test(TestOctaves, {
  
<<<<<<< HEAD
//     char const *file = "./data/lena.pgm";
//     //init files 
//     ezsift::Image<unsigned char> ez_img;
//     struct ethsift_image eth_img = {0};
//     if(ez_img.read_pgm(file) != 0) return 0;  
=======
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
>>>>>>> 62219b9add21dc14ba0c841f979badcc60a3dc68

//     //Init Octaves
//     int nOctaves = (int)std::log2((float)fmin(ez_img.w, ez_img.h)) - 3; // 2 or 3, need further research    

//     std::vector<ezsift::Image<unsigned char > > ez_octaves(nOctaves);

//     struct ethsift_image eth_octaves[nOctaves];
//     for(int i = 1; i < nOctaves; ++i){
//       eth_octaves[i] = {0};
      
//       int srcW = eth_octaves[i-1].width; 
//       int srcH = eth_octaves[i-1].height;
//       int dstW = srcW >> 1;
//       int dstH = srcH >> 1;
//       eth_octaves[i].width = dstW;
//       eth_octaves[i].height = dstH;
//     }

//     //Create Octaves for ethSift    
//     ethsift_generate_octaves(eth_img, eth_octaves, nOctaves);

//     //Create Octaves for ezSift    
//     build_octaves(ez_img, ez_octaves, 0, nOctaves);

//     int res = 0;
//     //Compare obtained Octaves in a loop
//     for(int i = 1; i < nOctaves; ++i){
//       res += compare_image_approx(ez_octaves[i], eth_octaves[i]);
//     }
//     if(res == nOctaves)
//       return 1;
//     else return 0;
//   })

// define_test(TestGaussianPyramid, {
  
<<<<<<< HEAD
//     char const *file = "./data/lena.pgm";
//     //init files 
//     ezsift::Image<unsigned char> ez_img;
//     struct ethsift_image eth_img = {0};
//     if(ez_img.read_pgm(file) != 0) return 0;  
//     if(!convert_image(ez_img, &eth_img)) return 0;

//     //TODO: implement TEST
=======
    char const *file = data_file("lena.pgm");
    //init files 
    ezsift::Image<unsigned char> ez_img;
    struct ethsift_image eth_img = {0};
    if(ez_img.read_pgm(file) != 0) return 0;  
    if(!convert_image(ez_img, &eth_img)) return 0;

    //TODO: implement TEST
>>>>>>> 62219b9add21dc14ba0c841f979badcc60a3dc68
    
//     return 1;
//   })
