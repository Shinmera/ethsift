
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
    // extract the very first octave in the function "ethsift_generate_pyramid", I suggest to kill 
    // the octave generation as a whole and simply just allocate space for the gaussian pyramid by 
    // "downsampling" the width and height of the image iteratively and simply pass the input image
    // to "ethsift_generate_pyramid" instead of some octaves.
    
    // DIFF NOTE to EZSift: 
    // Original implementation in EZSift contained some upsampling cases which 
    // were used if it was enabled in the settings. Our implementation does not support upsampling
    // of the original image before processing.
    // Also some (seemingly) irrelevant divisions were left out in our implementation. 

    for (int i = 1; i < octave_count; i++) {
        if (i == 0) {
            octaves[i] = image;
        }
        else {
            ethsift_downscale_linear(octaves[i-1], octaves[i]);
            // Zsombors optimization (since downscaled image is not needed in any case):
            // octaves[i].width = octaves[i-1].width * 0.5f;
            // octaves[i].height = octaves[i-1].height * 0.5f;
        }
    }
    return 1;
}