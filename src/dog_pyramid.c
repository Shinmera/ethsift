#include "internal.h"

/// <summary> 
/// Generate a pyramid consisting of the difference between consecutive blur amounts of the input image.
/// </summary>
/// <param name="gaussians"> IN: Struct of gaussians. </param>
/// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param>
/// <param name="differences"> IN/OUT: Struct of differences to compute. 
/// <param name="layers"> IN: Number of layers in dog pyramid (shoudl be layers in Gaussian - 1) </param> 
/// <returns> 0 </returns>

int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], 
                                        uint32_t gaussian_count, 
                                        struct ethsift_image differences[], 
                                        uint32_t layers,
                                        uint32_t octave_count)
{
    uint32_t width, height;
    int row_index;

    for(int i = 0; i < octave_count; i++){
        
        row_index = i * (int) (layers + 1);

        width = gaussians[row_index].width;
        height = gaussians[row_index].height;

        

        for(int j = 0; j < layers; j++){

            differences[i * layers + j].width = width;
            differences[i * layers + j].height = height;
            differences[i * layers + j].pixels = (float*) calloc(sizeof(float), width*height);
            
            // TODO: "Accelerate" framework can be used to subtract two floating point arrays
            for(int idx = 0; idx < (width * height); idx++){
                 differences[i * layers + j].pixels[idx] = gaussians[row_index + j + 1].pixels[idx] - gaussians[row_index + j].pixels[idx]; 
            }
        } 
    }

    return 1;

}
