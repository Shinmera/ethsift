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

        for(int idx = 0; idx < (width * height); idx++){
            differences[i * layers].pixels[idx] = gaussians[row_index + 1].pixels[idx] - gaussians[row_index].pixels[idx]; 
            differences[i * layers + 1].pixels[idx] = gaussians[row_index + 2].pixels[idx] - gaussians[row_index + 1].pixels[idx];
            differences[i * layers + 2].pixels[idx] = gaussians[row_index + 3].pixels[idx] - gaussians[row_index + 2].pixels[idx];  
            differences[i * layers + 3].pixels[idx] = gaussians[row_index + 4].pixels[idx] - gaussians[row_index + 3].pixels[idx];
            differences[i * layers + 4].pixels[idx] = gaussians[row_index + 5].pixels[idx] - gaussians[row_index + 4].pixels[idx]; 
            
            inc_adds(5);
            inc_mem(30);
        }

    }

    return 1;

}



// /// <summary> 
// /// Generate a pyramid consisting of the difference between consecutive blur amounts of the input image.
// /// </summary>
// /// <param name="gaussians"> IN: Struct of gaussians. </param>
// /// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param>
// /// <param name="differences"> IN/OUT: Struct of differences to compute. 
// /// <param name="layers"> IN: Number of layers in dog pyramid (shoudl be layers in Gaussian - 1) </param> 
// /// <returns> 1 IF generation was successful, ELSE 0. </returns>
// int ethsift_generate_difference_pyramid(struct ethsift_image gaussians[], 
//                                         uint32_t gaussian_count, 
//                                         struct ethsift_image differences[], 
//                                         uint32_t layers,
//                                         uint32_t octave_count){
//     uint32_t width, height;
//     int row_index;

//     float *out1_diff;
//     float *out2_diff;
//     float *pre_gaussian;
//     float *gaussian;
//     float *next_gaussian;
    
//     int lim = layers - 1;

//     for(int i = 0; i < octave_count; i++){
        
//         row_index = i * gaussian_count;

//         width = gaussians[row_index].width;
//         height = gaussians[row_index].height;
//         inc_mem(2); // 2 reads (maybe?)
        
//         int img_size = width * height;

//         int j;
//         for(j = 0; j < lim; j+=2){

//             out1_diff = differences[i * layers + j].pixels;
//             out2_diff = differences[i * layers + j + 1].pixels;

//             pre_gaussian = gaussians[row_index + j].pixels;
//             gaussian = gaussians[row_index + j + 1].pixels;
//             next_gaussian = gaussians[row_index + j + 2].pixels;

//             for(int idx = 0; idx < img_size; idx++){
//                 out1_diff[idx] = gaussian[idx] - pre_gaussian[idx]; 
//                 out2_diff[idx] = next_gaussian[idx] - gaussian[idx]; 

//                 inc_adds(1);
//                 inc_mem(6);
//             }
//         } 
        
//         for(; j < layers; j++){

//             out1_diff = differences[i * layers + j].pixels;

//             pre_gaussian = gaussians[row_index + j].pixels;
//             gaussian = gaussians[row_index + j + 1].pixels;

//             for(int idx = 0; idx < img_size; idx++){
//                 out1_diff[idx] = gaussian[idx] - pre_gaussian[idx]; 

//                 inc_adds(1);
//                 inc_mem(6);
//             }
//         } 
//     }

//     return 1;

// }