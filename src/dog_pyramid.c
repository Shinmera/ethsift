#include "internal.h"

/// <summary> 
/// Generate a pyramid consisting of the difference between consecutive blur amounts of the input image.
/// </summary>
/// <param name="gaussians"> IN: Struct of gaussians. </param>
/// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param>
/// <param name="differences"> IN/OUT: Struct of differences to compute. 
/// <param name="layers"> IN: Number of layers in dog pyramid (shoudl be layers in Gaussian - 1) </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], 
                                        uint32_t gaussian_count, 
                                        struct ethsift_image differences[], 
                                        uint32_t layers,
                                        uint32_t octave_count){
    uint32_t width, height;
    int row_index;

    for(int i = 0; i < octave_count; i++){
        
        row_index = i * gaussian_count;

        width = gaussians[row_index].width;
        height = gaussians[row_index].height;
        inc_read(2, uint32_t);

        for(int idx = 0; idx < (width * height); idx++){
            differences[i * layers].pixels[idx] = gaussians[row_index + 1].pixels[idx] - gaussians[row_index].pixels[idx]; 
            differences[i * layers + 1].pixels[idx] = gaussians[row_index + 2].pixels[idx] - gaussians[row_index + 1].pixels[idx];
            differences[i * layers + 2].pixels[idx] = gaussians[row_index + 3].pixels[idx] - gaussians[row_index + 2].pixels[idx];  
            differences[i * layers + 3].pixels[idx] = gaussians[row_index + 4].pixels[idx] - gaussians[row_index + 3].pixels[idx];
            differences[i * layers + 4].pixels[idx] = gaussians[row_index + 5].pixels[idx] - gaussians[row_index + 4].pixels[idx]; 
            
            inc_adds(5);
            inc_write(5, float);
            inc_read(10, float);
        }

    }

    return 1;

}
