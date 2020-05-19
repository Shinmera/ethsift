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
        inc_mem(2); // 2 reads (maybe?)
        float * dif_layer0 = differences[i * layers].pixels;
        float * dif_layer1 = differences[i * layers + 1].pixels;
        float * dif_layer2 = differences[i * layers + 2].pixels;
        float * dif_layer3 = differences[i * layers + 3].pixels;
        float * dif_layer4 = differences[i * layers + 4].pixels;

        float * gaussian0 = gaussians[row_index].pixels;
        float * gaussian1 = gaussians[row_index + 1].pixels;
        float * gaussian2 = gaussians[row_index + 2].pixels;
        float * gaussian3 = gaussians[row_index + 3].pixels;
        float * gaussian4 = gaussians[row_index + 4].pixels;
        float * gaussian5 = gaussians[row_index + 5].pixels;
        inc_mem(11);
        
        for(int idx = 0; idx < (width * height); idx++){
            dif_layer0[idx] = gaussian1[idx] - gaussian0[idx]; 
            dif_layer1[idx] = gaussian2[idx] - gaussian1[idx];
            dif_layer2[idx] = gaussian3[idx] - gaussian2[idx];  
            dif_layer3[idx] = gaussian4[idx] - gaussian3[idx];
            dif_layer4[idx] = gaussian5[idx] - gaussian4[idx]; 
            
            inc_adds(5);
            inc_mem(30);
        }

    }

    return 1;

}
