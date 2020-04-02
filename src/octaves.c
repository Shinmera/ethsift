
#include "internal.h"

/// <summary> 
/// Fill the octave pyramid by downsampling repeatedly.
/// </summary>
/// <param name="image"> IN: The original image we want to process. </param>
/// <param name="octaves"> OUT: Octaves to generate. </param>
/// <param name="kernerl_rad"> IN: Octaves image array size. </param>
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_octaves(struct ethsift_image image, 
                            struct ethsift_image octaves[], 
                            uint32_t octave_count)
{
    // ZSOMBORS ULTIMATE OPTIMIZATION:
    // Since in the implementation of EZSift (as in ours) the only use of the octaves is to pass and
    // extract the very first octave in the function "ethsift_generate_gaussian_pyramid", I suggest to kill 
    // the octave generation as a whole and simply just allocate space for the gaussian pyramid by 
    // "downsampling" the width and height of the image iteratively and simply pass the input image
    // to "ethsift_generate_gaussian_pyramid" instead of some octaves.
    
    // DIFF NOTE to EZSift: 
    // Original implementation in EZSift contained some upsampling cases which 
    // were used if it was enabled in the settings. Our implementation does not support upsampling
    // of the original image before processing.
    // Also some (seemingly) irrelevant divisions were left out in our implementation. 


    for (int i = 0; i < octave_count; i++) {
        if (i == 0) {
            memcpy(octaves[i].pixels, image.pixels, image.width * image.height * sizeof(float));
        }
        else {
            ethsift_downscale_half(octaves[i-1], octaves[i]);
        }
    }
    return 1;
}