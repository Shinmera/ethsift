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
    int row_index, index;

    

    for(int i = 0; i < octave_count; i++){
        
        row_index = i * gaussian_count;

        width = gaussians[row_index].width;
        height = gaussians[row_index].height;

        

        float *source_1 = (float*) calloc(sizeof(float), width*height);
        float *source_2 = (float*) calloc(sizeof(float), width*height);

        float *temp = (float*) calloc(sizeof(float), width*height);

        

        for(int j = 0; j < layers; j++){
            
            memcpy(source_1, gaussians[row_index + j].pixels, width*height*sizeof(float));
            memcpy(source_2, gaussians[row_index + j+1].pixels, width*height*sizeof(float));
            
            // TODO: "Accelerate" framework can be used to subtract two floating point arrays
            for(index = 0; index < (width * height); index++){
                 temp[index] = source_2[index] - source_1[index]; 
            }

            differences[i * layers + j].pixels = temp;
            

        } 
        

        free(source_1);
        free(source_2);
        free(temp);

        
    }
    

    

    return 1;

}
