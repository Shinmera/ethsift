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
                                        uint32_t layers)
{
    int images_in_gaussian_layer = layers + 1;
    uint32_t width, height;
    int row_index, index;

    float * pixels = (float*) calloc(sizeof(float), width*height);
    float * source_1 = (float*) calloc(sizeof(float), width*height);
    float * source_2 = (float*) calloc(sizeof(float), width*height);;

    for(int i = 0; i < gaussian_count; i++){
        
        row_index = i * images_in_gaussian_layer;

        width = gaussians[row_index].width;
        height = gaussians[row_index].height;

        for(int j = 0; j < layers; j++){

            // TODO: general method to allocate memory for struct?
            differences[i * layers + j].width = width; 
            differences[i * layers + j].height = height; 

            source_1 = gaussians[row_index + j].pixels;
            source_2 = gaussians[row_index + j + 1].pixels;
            
            // TODO: "Accelerate" framework can be used to subtract two floating point arrays
            for(index = 0; index < (width * height); index++){
                pixels[index] = source_2[index] - source_1[index]; 
            }

            differences[i * layers + j].pixels = pixels;

        } 
    }

    free(pixels);
    free(source_1);
    free(source_2);

    return 0;

}
