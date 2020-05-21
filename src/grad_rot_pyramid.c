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
        inc_read(2, int32_t);

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
        int col_counter = 0;
        for(int row = row_lower_offset; row < height-row_upper_offset; ++row){
        
            int row_plus_one = row + 1;
            int row_minus_one = row - 1;
            int row_width = row*width;
            int column = col_lower_offset;
            for(; column < width-col_upper_offset; column+=8){
                int write_index = row_width + column;
                int col_plus_one = column + 1;
                int col_minus_one = column - 1;
                
                int rpo_ind = row_plus_one * width + column;
                int rmo_ind = row_minus_one * width + column;
                int cpo_ind = row * width + col_plus_one;
                int cmo_ind = row * width + col_minus_one;

                gaussian_rpo_cols = _mm256_loadu_ps(in_gaussian + rpo_ind);
                gaussian_rmo_cols = _mm256_loadu_ps(in_gaussian + rmo_ind);
                
                gaussian1_rpo_cols = _mm256_loadu_ps(in_gaussian1 + rpo_ind);
                gaussian1_rmo_cols = _mm256_loadu_ps(in_gaussian1 + rmo_ind);

                gaussian2_rpo_cols = _mm256_loadu_ps(in_gaussian2 + rpo_ind);
                gaussian2_rmo_cols = _mm256_loadu_ps(in_gaussian2 + rmo_ind);

                
                gaussian_cpo_cols = _mm256_loadu_ps(in_gaussian + cpo_ind);
                gaussian_cmo_cols = _mm256_loadu_ps(in_gaussian + cmo_ind);
                
                gaussian1_cpo_cols = _mm256_loadu_ps(in_gaussian1 + cpo_ind);
                gaussian1_cmo_cols = _mm256_loadu_ps(in_gaussian1 + cmo_ind);

                gaussian2_cpo_cols = _mm256_loadu_ps(in_gaussian2 + cpo_ind);
                gaussian2_cmo_cols = _mm256_loadu_ps(in_gaussian2 + cmo_ind);
                inc_read(2*3*2*8, float);
                

                d_row_m256 = _mm256_sub_ps(gaussian_rpo_cols, gaussian_rmo_cols);
                d_column_m256 =  _mm256_sub_ps(gaussian_cpo_cols, gaussian_cmo_cols);
                 
                d_row1_m256 = _mm256_sub_ps(gaussian1_rpo_cols, gaussian1_rmo_cols);
                d_column1_m256 =  _mm256_sub_ps(gaussian1_cpo_cols, gaussian1_cmo_cols);

                d_row2_m256 = _mm256_sub_ps(gaussian2_rpo_cols, gaussian2_rmo_cols);
                d_column2_m256 =  _mm256_sub_ps(gaussian2_cpo_cols, gaussian2_cmo_cols);
                inc_adds(48); // 2 Subtractions
                
                sqrt_input = _mm256_mul_ps(d_row_m256, d_row_m256);  
                sqrt_input1 = _mm256_mul_ps(d_row1_m256, d_row1_m256);
                sqrt_input2 = _mm256_mul_ps(d_row2_m256, d_row2_m256);

                sqrt_input = _mm256_fmadd_ps(d_column_m256, d_column_m256, sqrt_input);
                sqrt_input1 = _mm256_fmadd_ps(d_column1_m256, d_column1_m256, sqrt_input1);
                sqrt_input2 = _mm256_fmadd_ps(d_column2_m256, d_column2_m256, sqrt_input2);

                grad = _mm256_sqrt_ps(sqrt_input);                
                eth_mm256_atan2_ps(&d_row_m256, &d_column_m256, &rot);

                grad1 = _mm256_sqrt_ps(sqrt_input1);
                eth_mm256_atan2_ps(&d_row1_m256, &d_column1_m256, &rot1);

                grad2 = _mm256_sqrt_ps(sqrt_input2);
                eth_mm256_atan2_ps(&d_row2_m256, &d_column2_m256, &rot2);

                _mm256_storeu_ps(out_grads + write_index, grad);
                _mm256_storeu_ps(out_rots + write_index, rot);
                
                _mm256_storeu_ps(out_grads1 + write_index, grad1);
                _mm256_storeu_ps(out_rots1 + write_index, rot1);
                
                _mm256_storeu_ps(out_grads2 + write_index, grad2);
                _mm256_storeu_ps(out_rots2 + write_index, rot2);
                inc_write(3*2*8, float);
            }
            //DO THE REST UP UNTIL TO THE BORDERS
            for(; column < width-1; ++column){
                    
                int col_plus_one = column + 1;
                int col_minus_one = column - 1;

                d_row = in_gaussian[row_plus_one * width + column] - in_gaussian[row_minus_one * width + column];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width + col_minus_one];
                
                d_row1 = in_gaussian1[row_plus_one * width + column] - in_gaussian1[row_minus_one * width + column];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width + col_minus_one];

                d_row2 = in_gaussian2[row_plus_one * width + column] - in_gaussian2[row_minus_one * width + column];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width + col_minus_one];
                inc_read(3*2*2, float);
                    
                inc_adds(6); // 2 Subtractions
                    
                out_grads[row * width + column] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width + column] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width + column] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width + column] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width + column] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width + column] = fast_atan2_f(d_row2, d_column2);
                inc_write(3*2, float);
            }
        }


        
        // DO THE THE BORDER OF THE IMAGE AS WE HAVE DONE BEFORE.
        int row_plus_one1 = 1;
        int row1 = 0;
        
        int row2 = height-1;
        int row_minus_one2 = height-2;
        for(int column = 0; column < width; ++column){
            // UPPER ROW BORDER
            int col_plus_one = internal_min(internal_max(column + 1, 0), width - 1);
            int col_minus_one = internal_min(internal_max(column - 1, 0), width - 1);

            d_row = in_gaussian[row_plus_one1 * width + column] - in_gaussian[row1 * width + column];    
            d_column = in_gaussian[row1 * width + col_plus_one] - in_gaussian[row1 * width + col_minus_one];
            
            d_row1 = in_gaussian1[row_plus_one1 * width + column] - in_gaussian1[row1 * width + column];    
            d_column1 = in_gaussian1[row1 * width + col_plus_one] - in_gaussian1[row1 * width + col_minus_one];

            d_row2 = in_gaussian2[row_plus_one1 * width + column] - in_gaussian2[row1 * width + column];    
            d_column2 = in_gaussian2[row1 * width + col_plus_one] - in_gaussian2[row1 * width + col_minus_one];
                
            inc_adds(6); // 2 Subtractions
            inc_read(3*2*2, float);
                
            out_grads[row1 * width + column] = sqrtf(d_row * d_row + d_column * d_column);
            out_rots[row1 * width + column] = fast_atan2_f(d_row, d_column); 
            
            out_grads1[row1 * width + column] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
            out_rots1[row1 * width + column] = fast_atan2_f(d_row1, d_column1); 
            
            out_grads2[row1 * width + column] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
            out_rots2[row1 * width + column] = fast_atan2_f(d_row2, d_column2); 
            inc_write(3*2, float);

            // LOWER ROW BORDER
            d_row = in_gaussian[row2 * width + column] - in_gaussian[row_minus_one2 * width + column];    
            d_column = in_gaussian[row2 * width + col_plus_one] - in_gaussian[row2 * width + col_minus_one];
            
            d_row1 = in_gaussian1[row2 * width + column] - in_gaussian1[row_minus_one2 * width + column];    
            d_column1 = in_gaussian1[row2 * width + col_plus_one] - in_gaussian1[row2 * width + col_minus_one];

            d_row2 = in_gaussian2[row2 * width + column] - in_gaussian2[row_minus_one2 * width + column];    
            d_column2 = in_gaussian2[row2 * width + col_plus_one] - in_gaussian2[row2 * width + col_minus_one];
                
            inc_adds(6); // 2 Subtractions
            inc_read(3*2*2, float);
                
            out_grads[row2 * width + column] = sqrtf(d_row * d_row + d_column * d_column);
            out_rots[row2 * width + column] = fast_atan2_f(d_row, d_column); 
            
            out_grads1[row2 * width + column] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
            out_rots1[row2 * width + column] = fast_atan2_f(d_row1, d_column1); 
            
            out_grads2[row2 * width + column] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
            out_rots2[row2 * width + column] = fast_atan2_f(d_row2, d_column2);
            inc_write(3*2, float);
        }
        //DO COLUMN BORDERS
        for(int row = 1; row < height-1; ++row){
                int row_plus_one = row+1;
                int row_minus_one = row-1;

                //LEFTHAND SIDE COLUMN BORDER
                int col_plus_one = 1;
                d_row = in_gaussian[row_plus_one * width] - in_gaussian[row_minus_one * width];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width];
                
                d_row1 = in_gaussian1[row_plus_one * width] - in_gaussian1[row_minus_one * width];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width];

                d_row2 = in_gaussian2[row_plus_one * width] - in_gaussian2[row_minus_one * width];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width];
                    
                inc_adds(6); // 2 Subtractions
                inc_read(3*2*2, float);
                    
                out_grads[row * width] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width] = fast_atan2_f(d_row2, d_column2);
                inc_write(3*2, float);

                
                //RIGHTHAND SIDE COLUMN BORDER
                col_plus_one = width-1;
                int col_minus_one = width-2;
                
                d_row = in_gaussian[row_plus_one * width + col_plus_one] - in_gaussian[row_minus_one * width + col_plus_one];    
                d_column = in_gaussian[row * width + col_plus_one] - in_gaussian[row * width + col_minus_one];
                
                d_row1 = in_gaussian1[row_plus_one * width + col_plus_one] - in_gaussian1[row_minus_one * width + col_plus_one];    
                d_column1 = in_gaussian1[row * width + col_plus_one] - in_gaussian1[row * width + col_minus_one];

                d_row2 = in_gaussian2[row_plus_one * width + col_plus_one] - in_gaussian2[row_minus_one * width + col_plus_one];    
                d_column2 = in_gaussian2[row * width + col_plus_one] - in_gaussian2[row * width + col_minus_one];
                    
                inc_adds(6); // 2 Subtractions
                inc_read(3*2*2, float);
                    
                out_grads[row * width + col_plus_one] = sqrtf(d_row * d_row + d_column * d_column);
                out_rots[row * width + col_plus_one] = fast_atan2_f(d_row, d_column); 
                
                out_grads1[row * width + col_plus_one] = sqrtf(d_row1 * d_row1 + d_column1 * d_column1);
                out_rots1[row * width + col_plus_one] = fast_atan2_f(d_row1, d_column1); 
                
                out_grads2[row * width + col_plus_one] = sqrtf(d_row2 * d_row2 + d_column2 * d_column2);
                out_rots2[row * width + col_plus_one] = fast_atan2_f(d_row2, d_column2);
                inc_write(3*2, float);
            
        }

    }

    return 1;
}

int ethsift_generate_gradient_pyramid_janleu(struct ethsift_image gaussians[], 
                                      uint32_t gaussian_count, 
                                      struct ethsift_image gradients[], 
                                      struct ethsift_image rotations[], 
                                      uint32_t layers,
                                      uint32_t octave_count){
    int width, height;
    int idx;

    float *in_gaussian;
    float *out_grads;
    float *out_rots;

    for(int i = 0; i < octave_count; i++){
        
        width = (int) gaussians[i * gaussian_count].width;
        height = (int) gaussians[i * gaussian_count].height;
        
        inc_read(2, int32_t);   
        
        __m256 d_cp1;
        __m256 d_cm1;
        __m256 d_rp1;
        __m256 d_rm1;
        
        __m256 d_row;
        __m256 d_col;
        
        __m256 d_input_sqrt;
        
        __m256 d_sqrt;
        __m256 d_atan;

        int w_lim = width - 8;
        for (int l = 1; l <= layers; ++l) {
            idx = i * gaussian_count + l;

            in_gaussian = gaussians[idx].pixels;
            out_grads = gradients[idx].pixels;
            out_rots = rotations[idx].pixels;
            
            int c;
            for (int r = 1; r < height - 1; ++r) {
                for (c = 1; c < w_lim; c += 8) {
                    int c_m1 = c - 1;
                    int c_p1 = c + 1;
                    int r_m1 = r - 1;
                    int r_p1 = r + 1; 

                    int pos = r * width + c;

                    d_cp1 = _mm256_loadu_ps(in_gaussian + r * width + c_p1);
                    d_cm1 = _mm256_loadu_ps(in_gaussian + r * width + c_m1);
                    d_rm1 = _mm256_loadu_ps(in_gaussian + r_m1 * width + c);
                    d_rp1 = _mm256_loadu_ps(in_gaussian + r_p1 * width + c);
                    inc_read(4*8, float);

                    d_row = _mm256_sub_ps(d_rp1, d_rm1);
                    d_col = _mm256_sub_ps(d_cp1, d_cm1);
                    
                    d_input_sqrt = _mm256_mul_ps(d_col, d_col);
                    d_input_sqrt = _mm256_fmadd_ps(d_row, d_row, d_input_sqrt);
                    d_sqrt = _mm256_sqrt_ps(d_input_sqrt);

                    eth_mm256_atan2_ps(&d_row, &d_col, &d_atan);

                    _mm256_storeu_ps(out_grads + pos, d_sqrt);
                    _mm256_storeu_ps(out_rots + pos, d_atan);
                    inc_write(2*8, float);
                }

                for (; c < width; ++c) {
                    int r_p1 = r + 1;
                    int r_m1 = r - 1;
                    int c_m1 = internal_min(internal_max(c - 1, 0), width - 1);
                    int c_p1 = internal_min(internal_max(c + 1, 0), width - 1);

                    float row = in_gaussian[r_p1 * width + c] - in_gaussian[r_m1 * width + c];    
                    float col = in_gaussian[r * width + c_p1] - in_gaussian[r * width + c_m1];
                    inc_read(2*2, float);

                    out_grads[r * width + c] = sqrtf(row * row + col * col);
                    out_rots[r * width + c] = fast_atan2_f(row, col);
                    inc_write(2, float);
                }
            }


            for (int i = 0; i < width; ++ i) {
                int c_p1 = internal_min(internal_max(i + 1, 0), width - 1);
                int c_m1 = internal_min(internal_max(i - 1, 0), width - 1);
                
                float row1 = in_gaussian[width + i] - in_gaussian[i];
                float col1 = in_gaussian[c_p1] - in_gaussian[c_m1];
                inc_read(2*2, float);

                float row2 = in_gaussian[(height - 1) * width + i] - in_gaussian[(height - 2) * width + i];
                float col2 = in_gaussian[(height - 1) * width + c_p1] - in_gaussian[(height - 1) * width + c_m1];
                inc_read(2*2, float);

                out_grads[i] = sqrtf(row1 * row1 + col1 * col1);
                out_rots[i] = fast_atan2_f(row1, col1); 

                out_grads[(height - 1) * width + i] = sqrtf(row2 * row2 + col2 * col2);
                out_rots[(height - 1) * width + i] = fast_atan2_f(row2, col2); 
                inc_write(2, float);
            }

            for (int i = 0; i < height; ++i) {
                int c_m1 = 0;
                int c_p1 = 1;
                
                int r_p1 = internal_min(internal_max(i + 1, 0), height - 1);
                int r_m1 = internal_min(internal_max(i - 1, 0), height - 1);
                
                float row1 = in_gaussian[r_p1 * width] - in_gaussian[r_m1 * width];
                float col1 = in_gaussian[i * width + c_p1] - in_gaussian[i * width + c_m1];
                inc_read(2*2, float);

                out_grads[i * width] = sqrtf(row1 * row1 + col1 * col1);
                out_rots[i * width] = fast_atan2_f(row1, col1); 
                inc_write(2, float);
            }
        }      
    }
    return 1;
}

