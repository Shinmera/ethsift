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

        for(int row = 4; row < height-4; row++){
        
            int row_plus_one = row + 1;
            int row_minus_one = row - 1;
            int row_width = row*width;
            for(int column = 4; column < width-4; column+=8){
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
                    
                inc_adds(6); // 2 Subtractions
                inc_mem(12); // Maybe?
                sqrt_input = _mm256_mul_ps(d_row_m256, d_row_m256);  
                sqrt_input1 = _mm256_mul_ps(d_row1_m256, d_row1_m256);
                sqrt_input2 = _mm256_mul_ps(d_row2_m256, d_row2_m256);

                sqrt_input = _mm256_fmadd_ps(d_column_m256, d_column_m256, sqrt_input);
                sqrt_input1 = _mm256_fmadd_ps(d_column1_m256, d_column1_m256, sqrt_input1);
                sqrt_input2 = _mm256_fmadd_ps(d_column2_m256, d_column2_m256, sqrt_input2);

                grad = _mm256_sqrt_ps(sqrt_input);
                rot = _mm256_atan2_ps(d_row_m256, d_column_m256);
                grad1 = _mm256_sqrt_ps(sqrt_input);
                rot1 = _mm256_atan2_ps(d_row1_m256, d_column1_m256);
                grad2 = _mm256_sqrt_ps(sqrt_input);
                rot2 = _mm256_atan2_ps(d_row2_m256, d_column2_m256);

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
