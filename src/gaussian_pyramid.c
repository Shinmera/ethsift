#include "internal.h"

/// <summary> 
/// Creates a pyramid of images containing blurred versions of the input image.
/// </summary>
/// <param name="octaves"> IN: The octaves of the input image. </param>
/// <param name="octave_count"> IN: Number of octaves. </param>
/// <param name="gaussians"> IN/OUT: Struct of gaussians to compute. </param>
/// <param name="gaussian_count"> IN: Number of gaussian blurred images. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_pyramid(struct ethsift_image octaves[], 
                            uint32_t octave_count, 
                            struct ethsift_image gaussians[], 
                            uint32_t gaussian_count)
{


    return 1;
}
