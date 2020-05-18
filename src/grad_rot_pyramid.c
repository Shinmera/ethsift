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
    int idx1, idx2, idx3, base_idx;
    float d_row1, d_column1, d_row2, d_column2, d_row3, d_column3;

    float *in_gaussian1, *in_gaussian2, *in_gaussian3;
    float * out_grads1, *out_grads2, *out_grads3;
    float * out_rots1, *out_rots2, *out_rots3;

    for(int i = 0; i < octave_count; i++){
        
        width = (int) gaussians[i * gaussian_count].width;
        height = (int) gaussians[i * gaussian_count].height;
        inc_mem(2);

        base_idx = i * gaussian_count;
        idx1 = base_idx + 1;         
        idx2 = base_idx + 2;
        idx3 = base_idx + 3;

        in_gaussian1 = gaussians[idx1].pixels;
        out_grads1 = gradients[idx1].pixels;
        out_rots1 = rotations[idx1].pixels;

        
        in_gaussian2 = gaussians[idx2].pixels;
        out_grads2 = gradients[idx2].pixels;
        out_rots2 = rotations[idx2].pixels;

        
        in_gaussian3 = gaussians[idx3].pixels;
        out_grads3 = gradients[idx3].pixels;
        out_rots3 = rotations[idx3].pixels;

        inc_mem(9);

        for(int row = 0; row < height; row++){
            for(int column = 0; column < width; column++){

                int row_plus_one1 = internal_min(internal_max(row + 1, 0), height - 1);
                int row_minus_one1 = internal_min(internal_max(row - 1, 0), height - 1);
                
                int col_plus_one1 = internal_min(internal_max(column + 1, 0), width - 1);
                int col_minus_one1 = internal_min(internal_max(column - 1, 0), width - 1);

                d_row1 = in_gaussian1[row_plus_one1 * width + column] - in_gaussian1[row_minus_one1 * width + column];
                    
                d_column1 = in_gaussian1[row * width + col_plus_one1] - in_gaussian1[row * width + col_minus_one1];
                    
                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?
                    
                out_grads1[row * width + column] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width + column] = fast_atan2_f(d_row1, d_column1); 
                inc_mem(2); // At least two writes

                int row_plus_one2 = internal_min(internal_max(row + 1, 0), height - 1);
                int row_minus_one2 = internal_min(internal_max(row - 1, 0), height - 1);
                
                int col_plus_one2 = internal_min(internal_max(column + 1, 0), width - 1);
                int col_minus_one2 = internal_min(internal_max(column - 1, 0), width - 1);

                d_row2 = in_gaussian2[row_plus_one2 * width + column] - in_gaussian2[row_minus_one2 * width + column];
                    
                d_column2 = in_gaussian2[row * width + col_plus_one2] - in_gaussian2[row * width + col_minus_one2];
                    
                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?
                    
                out_grads2[row * width + column] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width + column] = fast_atan2_f(d_row2, d_column2); 
                inc_mem(2); // At least two writes

                
                int row_plus_one3 = internal_min(internal_max(row + 1, 0), height - 1);
                int row_minus_one3 = internal_min(internal_max(row - 1, 0), height - 1);
                
                int col_plus_one3 = internal_min(internal_max(column + 1, 0), width - 1);
                int col_minus_one3 = internal_min(internal_max(column - 1, 0), width - 1);

                d_row3 = in_gaussian3[row_plus_one3 * width + column] - in_gaussian3[row_minus_one3 * width + column];
                    
                d_column3 = in_gaussian3[row * width + col_plus_one3] - in_gaussian3[row * width + col_minus_one3];
                    
                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?
                    
                out_grads3[row * width + column] = sqrtf(d_row3 * d_row3 + d_column3 * d_column3);
                out_rots3[row * width + column] = fast_atan2_f(d_row3, d_column3); 
                inc_mem(2); // At least two writes
            }
        }
     
    }

    return 1;
}
