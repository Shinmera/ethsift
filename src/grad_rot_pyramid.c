#include "internal.h"

int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], 
                                      uint32_t gaussian_count, 
                                      struct ethsift_image gradients[], 
                                      struct ethsift_image rotations[], 
                                      uint32_t layers,
                                      uint32_t octave_count)
{
    int width, height;
    int idx;
    float d_row, d_column, angle;


    for(int i = 0; i < octave_count; i++){
        
        width = (int) gaussians[i * gaussian_count].width;
        height = (int) gaussians[i * gaussian_count].height;

        for(int j = 1; j <= layers; j++){

            idx = i * octave_count + j;         

            for(int row = 0; row < height; row++){
                for(int column = 0; column < width; column++){

                    d_row = get_pixel_f(gaussians[idx].pixels, width, height, row+1, column) - 
                            get_pixel_f(gaussians[idx].pixels, width, height, row-1, column);
                    
                    d_column = get_pixel_f(gaussians[idx].pixels, width, height, row, column+1) - 
                               get_pixel_f(gaussians[idx].pixels, width, height, row, column-1);
                    
                    gradients[idx].pixels[row * width + column] = sqrtf(d_row * d_row + d_column * d_column);      
                    
                    rotations[idx].pixels[row * width + column] = fast_atan2_f(d_row, d_column);   
                }
            }

        }
        
    }

    return 1;
}
