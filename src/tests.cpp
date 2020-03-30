#include "tester.h"

define_test(Dummy, {
    return 1;
  })

define_test(SimpleAllocation, {
    const int pyramid_size = 5;
    struct ethsift_image pyramid[pyramid_size] = {0};
    pyramid[0].width = 1024;
    pyramid[0].height = 512;
    with_measurement({
        if(!ethsift_allocate_pyramid(pyramid, pyramid_size))
          fail("Allocation failed");
      });

    for(int i=1; i<pyramid_size; ++i){
      uint32_t width = pyramid[0].width / (2 << (i-1));
      uint32_t height = pyramid[0].height / (2 << (i-1));
      if(pyramid[i].pixels == 0)
        fail("Pixel array unset");
      if(pyramid[i].width != width)
        fail("Unexpected width: %i expected %i", pyramid[i].width, width);
      if(pyramid[i].height != height)
        fail("Unexpected height: %i expected %i", pyramid[i].height, height);
      if(pyramid[i].pixels[width*height-1] != 0.0)
        fail("Unexpected value at end of array: %f", pyramid[i].pixels[width*height-1]);
    }
  })

define_test(RandomAllocation, {
    // Perform a bunch of randomised runs.
    for(int r=0; r<100; ++r){
      int pyramid_size = rand() % 10;
      struct ethsift_image pyramid[pyramid_size] = {0};
      pyramid[0].width = 2 << (rand()%4 + 1 + pyramid_size);
      pyramid[0].height = 2 << (rand()%4 + 1 + pyramid_size);
      with_measurement({
          if(!ethsift_allocate_pyramid(pyramid, pyramid_size))
            fail("Allocation failed");
        });

      for(int i=1; i<pyramid_size; ++i){
        uint32_t width = pyramid[0].width / (2 << (i-1));
        uint32_t height = pyramid[0].height / (2 << (i-1));
        if(pyramid[i].pixels == 0)
          fail("Pixel array unset");
        if(pyramid[i].width != width)
          fail("Unexpected width: %i expected %i", pyramid[i].width, width);
        if(pyramid[i].height != height)
          fail("Unexpected height: %i expected %i", pyramid[i].height, height);
        if(pyramid[i].pixels[width*height-1] != 0.0)
          fail("Unexpected value at end of array: %f", pyramid[i].pixels[width*height-1]);
      }
    }
  })
