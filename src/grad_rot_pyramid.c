#include "internal.h"

/// <summary> 
/// Build the gradient and rotation pyramids
/// NOTE: Size of Pyramids = octave_count * gaussian_count with empty entries!
/// </summary>
/// <param name="gaussians"> IN: The octaves of the input image. </param>
/// <param name="gaussian_count"> IN: Number of octaves. </param>
/// <param name="gradients"> OUT: Struct of gradients to compute. 
/// <param name="rotations"> OUT: Struct of rotations to compute. 
/// <param name="layers"> IN: Number of layers in the gradients and rotation pyramids. 
/// <param name="octave_count"> IN: Number of octaves.  </param>
/// <param name="gaussian_count"> IN: Number of gaussian blurred images per layer. </param> 
/// <returns> 1 IF generation was successful, ELSE 0. </returns>
int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], 
                                      uint32_t gaussian_count, 
                                      struct ethsift_image gradients[], 
                                      struct ethsift_image rotations[], 
                                      uint32_t layers,
                                      uint32_t octave_count){
    int width, height;
    int idx;
    float d_row, d_column;
    float d_row1, d_column1;
    float d_row2, d_column2;

    float * in_gaussian;
    float * out_grads;
    float * out_rots;
    
    float * in_gaussian1;
    float * out_grads1;
    float * out_rots1;

    float * in_gaussian2;
    float * out_grads2;
    float * out_rots2;

    for(int i = 0; i < octave_count; i++){
        
        width = (int) gaussians[i * gaussian_count].width;
        height = (int) gaussians[i * gaussian_count].height;
        inc_mem(2);

        idx = i * gaussian_count + 1;    

        in_gaussian = gaussians[idx].pixels;
        out_grads = gradients[idx].pixels;
        out_rots = rotations[idx].pixels;
        
        in_gaussian1 = gaussians[idx + 1].pixels;
        out_grads1 = gradients[idx + 1].pixels;
        out_rots1 = rotations[idx + 1].pixels;
        
        in_gaussian2 = gaussians[idx + 2].pixels;
        out_grads2 = gradients[idx + 2].pixels;
        out_rots2 = rotations[idx + 2].pixels;

        inc_mem(9);

        for(int row = 0; row < height; row++){
            for(int column = 0; column < width; column++){

                int row_plus_one = internal_min(internal_max(row + 1, 0), height - 1);
                int row_minus_one = internal_min(internal_max(row - 1, 0), height - 1);
                
                int col_plus_one = internal_min(internal_max(column + 1, 0), width - 1);
                int col_minus_one = internal_min(internal_max(column - 1, 0), width - 1);

                d_row = in_gaussian[row_plus_one * width + column] - in_gaussian[row_minus_one * width + column];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width + col_minus_one];
                
                d_row1 = in_gaussian1[row_plus_one * width + column] - in_gaussian1[row_minus_one * width + column];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width + col_minus_one];

                d_row2 = in_gaussian2[row_plus_one * width + column] - in_gaussian2[row_minus_one * width + column];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width + col_minus_one];
                    
                inc_adds(6); // 2 Subtractions
                inc_mem(12); // Maybe?
                    
                out_grads[row * width + column] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width + column] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width + column] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width + column] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width + column] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width + column] = fast_atan2_f(d_row2, d_column2); 
                inc_mem(6); // At least two writes
            }
        }     
    }

    return 1;
}
