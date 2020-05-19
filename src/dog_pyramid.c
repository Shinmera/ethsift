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
    __m256 gaussian_vec0_0, gaussian_vec0_1;
    __m256 gaussian_vec1_0, gaussian_vec1_1;
    __m256 gaussian_vec2_0, gaussian_vec2_1;
    __m256 gaussian_vec3_0, gaussian_vec3_1;
    __m256 gaussian_vec4_0, gaussian_vec4_1;
    __m256 gaussian_vec5_0, gaussian_vec5_1;

    __m256 gaussian_vec0_2, gaussian_vec0_3;
    __m256 gaussian_vec1_2, gaussian_vec1_3;
    __m256 gaussian_vec2_2, gaussian_vec2_3;
    __m256 gaussian_vec3_2, gaussian_vec3_3;
    __m256 gaussian_vec4_2, gaussian_vec4_3;
    __m256 gaussian_vec5_2, gaussian_vec5_3;

    __m256 dif_vec0_0;
    __m256 dif_vec1_0;
    __m256 dif_vec2_0;
    __m256 dif_vec3_0;
    __m256 dif_vec4_0;
    
    __m256 dif_vec0_1;
    __m256 dif_vec1_1;
    __m256 dif_vec2_1;
    __m256 dif_vec3_1;
    __m256 dif_vec4_1;
    
    __m256 dif_vec0_2;
    __m256 dif_vec1_2;
    __m256 dif_vec2_2;
    __m256 dif_vec3_2;
    __m256 dif_vec4_2;
    
    __m256 dif_vec0_3;
    __m256 dif_vec1_3;
    __m256 dif_vec2_3;
    __m256 dif_vec3_3;
    __m256 dif_vec4_3;

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
        
        for(int idx = 0; idx < (width * height); idx+= 32){
            int idx2 = idx + 8;
            int idx3 = idx + 16;
            int idx4 = idx + 24;

            gaussian_vec0_0 =  _mm256_loadu_ps(gaussian0 + idx);
            gaussian_vec1_0 =  _mm256_loadu_ps(gaussian1 + idx);
            gaussian_vec2_0 =  _mm256_loadu_ps(gaussian2 + idx);
            gaussian_vec3_0 =  _mm256_loadu_ps(gaussian3 + idx);
            gaussian_vec4_0 =  _mm256_loadu_ps(gaussian4 + idx);
            gaussian_vec5_0 =  _mm256_loadu_ps(gaussian5 + idx);
            
            gaussian_vec0_1 =  _mm256_loadu_ps(gaussian0 + idx2);
            gaussian_vec1_1 =  _mm256_loadu_ps(gaussian1 + idx2);
            gaussian_vec2_1 =  _mm256_loadu_ps(gaussian2 + idx2);
            gaussian_vec3_1 =  _mm256_loadu_ps(gaussian3 + idx2);
            gaussian_vec4_1 =  _mm256_loadu_ps(gaussian4 + idx2);
            gaussian_vec5_1 =  _mm256_loadu_ps(gaussian5 + idx2);
            
            gaussian_vec0_2 =  _mm256_loadu_ps(gaussian0 + idx3);
            gaussian_vec1_2 =  _mm256_loadu_ps(gaussian1 + idx3);
            gaussian_vec2_2 =  _mm256_loadu_ps(gaussian2 + idx3);
            gaussian_vec3_2 =  _mm256_loadu_ps(gaussian3 + idx3);
            gaussian_vec4_2 =  _mm256_loadu_ps(gaussian4 + idx3);
            gaussian_vec5_2 =  _mm256_loadu_ps(gaussian5 + idx3);
            
            gaussian_vec0_3 =  _mm256_loadu_ps(gaussian0 + idx4);
            gaussian_vec1_3 =  _mm256_loadu_ps(gaussian1 + idx4);
            gaussian_vec2_3 =  _mm256_loadu_ps(gaussian2 + idx4);
            gaussian_vec3_3 =  _mm256_loadu_ps(gaussian3 + idx4);
            gaussian_vec4_3 =  _mm256_loadu_ps(gaussian4 + idx4);
            gaussian_vec5_3 =  _mm256_loadu_ps(gaussian5 + idx4);

            dif_vec0_0 = _mm256_sub_ps(gaussian_vec1_0,gaussian_vec0_0);
            dif_vec1_0 = _mm256_sub_ps(gaussian_vec2_0,gaussian_vec1_0);
            dif_vec2_0 = _mm256_sub_ps(gaussian_vec3_0,gaussian_vec2_0);
            dif_vec3_0 = _mm256_sub_ps(gaussian_vec4_0,gaussian_vec3_0);
            dif_vec4_0 = _mm256_sub_ps(gaussian_vec5_0,gaussian_vec4_0);
            
            dif_vec0_1 = _mm256_sub_ps(gaussian_vec1_1,gaussian_vec0_1);
            dif_vec1_1 = _mm256_sub_ps(gaussian_vec2_1,gaussian_vec1_1);
            dif_vec2_1 = _mm256_sub_ps(gaussian_vec3_1,gaussian_vec2_1);
            dif_vec3_1 = _mm256_sub_ps(gaussian_vec4_1,gaussian_vec3_1);
            dif_vec4_1 = _mm256_sub_ps(gaussian_vec5_1,gaussian_vec4_1);
            
            dif_vec0_2 = _mm256_sub_ps(gaussian_vec1_2,gaussian_vec0_2);
            dif_vec1_2 = _mm256_sub_ps(gaussian_vec2_2,gaussian_vec1_2);
            dif_vec2_2 = _mm256_sub_ps(gaussian_vec3_2,gaussian_vec2_2);
            dif_vec3_2 = _mm256_sub_ps(gaussian_vec4_2,gaussian_vec3_2);
            dif_vec4_2 = _mm256_sub_ps(gaussian_vec5_2,gaussian_vec4_2);
            
            dif_vec0_3 = _mm256_sub_ps(gaussian_vec1_3,gaussian_vec0_3);
            dif_vec1_3 = _mm256_sub_ps(gaussian_vec2_3,gaussian_vec1_3);
            dif_vec2_3 = _mm256_sub_ps(gaussian_vec3_3,gaussian_vec2_3);
            dif_vec3_3 = _mm256_sub_ps(gaussian_vec4_3,gaussian_vec3_3);
            dif_vec4_3 = _mm256_sub_ps(gaussian_vec5_3,gaussian_vec4_3);


            _mm256_storeu_ps(dif_layer0 + idx, dif_vec0_0);
            _mm256_storeu_ps(dif_layer1 + idx, dif_vec1_0);
            _mm256_storeu_ps(dif_layer2 + idx, dif_vec2_0);
            _mm256_storeu_ps(dif_layer3 + idx, dif_vec3_0);
            _mm256_storeu_ps(dif_layer4 + idx, dif_vec4_0);

            _mm256_storeu_ps(dif_layer0 + idx2, dif_vec0_1);
            _mm256_storeu_ps(dif_layer1 + idx2, dif_vec1_1);
            _mm256_storeu_ps(dif_layer2 + idx2, dif_vec2_1);
            _mm256_storeu_ps(dif_layer3 + idx2, dif_vec3_1);
            _mm256_storeu_ps(dif_layer4 + idx2, dif_vec4_1);
            
            _mm256_storeu_ps(dif_layer0 + idx3, dif_vec0_2);
            _mm256_storeu_ps(dif_layer1 + idx3, dif_vec1_2);
            _mm256_storeu_ps(dif_layer2 + idx3, dif_vec2_2);
            _mm256_storeu_ps(dif_layer3 + idx3, dif_vec3_2);
            _mm256_storeu_ps(dif_layer4 + idx3, dif_vec4_2);

            _mm256_storeu_ps(dif_layer0 + idx4, dif_vec0_3);
            _mm256_storeu_ps(dif_layer1 + idx4, dif_vec1_3);
            _mm256_storeu_ps(dif_layer2 + idx4, dif_vec2_3);
            _mm256_storeu_ps(dif_layer3 + idx4, dif_vec3_3);
            _mm256_storeu_ps(dif_layer4 + idx4, dif_vec4_3);

            inc_adds(20);
            inc_mem(44);
        }

    }

    return 1;

}
