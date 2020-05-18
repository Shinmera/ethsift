#include "internal.h"

static inline void calculate_border(int row){
    
}



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
    int col_upper_offset = 8, col_lower_offset = 1;
    int row_upper_offset = 1, row_lower_offset = 1;
    

    float d_row, d_column;
    float d_row1, d_column1;
    float d_row2, d_column2;

    __m256 d_row_m256, d_column_m256;
    __m256 d_row1_m256, d_column1_m256;
    __m256 d_row2_m256, d_column2_m256;

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

        // DO THE THE BORDER OF THE IMAGE AS WE HAVE DONE BEFORE.
        for(int row = 0; row < height; ++row){
            int row_plus_one = internal_min(internal_max(row + 1, 0), height - 1);
            int row_minus_one = internal_min(internal_max(row - 1, 0), height - 1);
            if(row < row_lower_offset || row >= height-row_upper_offset){
                for(int column = 0; column < width; ++column){
                    
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
            else{
                int col_plus_one = 1;
                int col_minus_one = 0;
                d_row = in_gaussian[row_plus_one * width] - in_gaussian[row_minus_one * width ];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width + col_minus_one];
                
                d_row1 = in_gaussian1[row_plus_one * width ] - in_gaussian1[row_minus_one * width ];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width + col_minus_one];

                d_row2 = in_gaussian2[row_plus_one * width ] - in_gaussian2[row_minus_one * width ];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width + col_minus_one];
                    
                inc_adds(6); // 2 Subtractions
                inc_mem(12); // Maybe?
                    
                out_grads[row * width ] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width ] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width] = fast_atan2_f(d_row2, d_column2); 
                inc_mem(6); // At least two writes

                col_plus_one = width-1;
                col_minus_one = width-2;
                d_row = in_gaussian[row_plus_one * width + col_plus_one] - in_gaussian[row_minus_one * width + col_plus_one];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width + col_minus_one];
                
                d_row1 = in_gaussian1[row_plus_one * width + col_plus_one] - in_gaussian1[row_minus_one * width + col_plus_one];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width + col_minus_one];

                d_row2 = in_gaussian2[row_plus_one * width + col_plus_one] - in_gaussian2[row_minus_one * width + col_plus_one];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width + col_minus_one];
                    
                inc_adds(6); // 2 Subtractions
                inc_mem(12); // Maybe?
                    
                out_grads[row * width ] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width ] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width] = fast_atan2_f(d_row2, d_column2); 
                inc_mem(6); // At least two writes
            }
        }
            



        // DO THE MIDDLE OF THE IMAGE WITH AVX
        __m256 gaussian_rpo_cols, gaussian_rmo_cols;        
        __m256 gaussian1_rpo_cols, gaussian1_rmo_cols;
        __m256 gaussian2_rpo_cols, gaussian2_rmo_cols;
        
        __m256 gaussian_cpo_cols, gaussian_cmo_cols;
        __m256 gaussian1_cpo_cols, gaussian1_cmo_cols;
        __m256 gaussian2_cpo_cols, gaussian2_cmo_cols;

        __m256 sqrt_input, sqrt_input1, sqrt_input2;
        __m256 grad, grad1, grad2;
        __m256 rot, rot1, rot2;

        for(int row = row_lower_offset; row < height-row_upper_offset; ++row){
        
            int row_plus_one = row + 1;
            int row_minus_one = row - 1;
            int row_width = row*width;
            for(int column = col_lower_offset; column < width-col_upper_offset; column+=8){
                int write_index = row_width + column;
                int col_plus_one = column + 1;
                int col_minus_one = column - 1;

                int rpo_ind = row_plus_one * width + column;
                int rmo_ind = row_minus_one * width + column;
                int cpo_ind = row * width + col_plus_one;
                int cmo_ind = row * width + col_minus_one;

                gaussian_rpo_cols = _mm256_load_ps(in_gaussian + rpo_ind);
                gaussian_rmo_cols = _mm256_load_ps(in_gaussian + rmo_ind);
                
                gaussian1_rpo_cols = _mm256_load_ps(in_gaussian1 + rpo_ind);
                gaussian1_rmo_cols = _mm256_load_ps(in_gaussian1 + rmo_ind);

                gaussian2_rpo_cols = _mm256_load_ps(in_gaussian2 + rpo_ind);
                gaussian2_rmo_cols = _mm256_load_ps(in_gaussian2 + rmo_ind);

                
                gaussian_cpo_cols = _mm256_load_ps(in_gaussian + cpo_ind);
                gaussian_cmo_cols = _mm256_load_ps(in_gaussian + cmo_ind);
                
                gaussian1_cpo_cols = _mm256_load_ps(in_gaussian1 + cpo_ind);
                gaussian1_cmo_cols = _mm256_load_ps(in_gaussian1 + cmo_ind);

                gaussian2_cpo_cols = _mm256_load_ps(in_gaussian2 + cpo_ind);
                gaussian2_cmo_cols = _mm256_load_ps(in_gaussian2 + cmo_ind);

                d_row_m256 = _mm256_sub_ps(gaussian_rpo_cols, gaussian_rmo_cols);
                d_column_m256 =  _mm256_sub_ps(gaussian_cpo_cols, gaussian_cmo_cols);
                 
                d_row1_m256 = _mm256_sub_ps(gaussian1_rpo_cols, gaussian1_rmo_cols);
                d_column1_m256 =  _mm256_sub_ps(gaussian1_cpo_cols, gaussian1_cmo_cols);

                d_row2_m256 = _mm256_sub_ps(gaussian2_rpo_cols, gaussian2_rmo_cols);
                d_column2_m256 =  _mm256_sub_ps(gaussian2_cpo_cols, gaussian2_cmo_cols);
                    
                inc_adds(48); // 2 Subtractions
                inc_mem(12); // Maybe?
                sqrt_input = _mm256_mul_ps(d_row_m256, d_row_m256);  
                sqrt_input1 = _mm256_mul_ps(d_row1_m256, d_row1_m256);
                sqrt_input2 = _mm256_mul_ps(d_row2_m256, d_row2_m256);

                sqrt_input = _mm256_fmadd_ps(d_column_m256, d_column_m256, sqrt_input);
                sqrt_input1 = _mm256_fmadd_ps(d_column1_m256, d_column1_m256, sqrt_input1);
                sqrt_input2 = _mm256_fmadd_ps(d_column2_m256, d_column2_m256, sqrt_input2);

                grad = _mm256_sqrt_ps(sqrt_input);                
                eth_mm256_atan2_ps(&d_row_m256, &d_column_m256, &rot);

                grad1 = _mm256_sqrt_ps(sqrt_input);
                eth_mm256_atan2_ps(&d_row1_m256, &d_column1_m256, &rot1);

                grad2 = _mm256_sqrt_ps(sqrt_input);
                eth_mm256_atan2_ps(&d_row2_m256, &d_column2_m256, &rot2);

                _mm256_store_ps(out_grads + write_index, grad);
                _mm256_store_ps(out_rots + write_index, rot);
                
                
                _mm256_store_ps(out_grads1 + write_index, grad1);
                _mm256_store_ps(out_rots1 + write_index, rot1);
                
                _mm256_store_ps(out_grads2 + write_index, grad2);
                _mm256_store_ps(out_rots2 + write_index, rot2);
                inc_mem(6); // At least two writes
            }
        }     
    }

    return 1;
}
