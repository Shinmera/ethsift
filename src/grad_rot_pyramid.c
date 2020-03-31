#include "internal.h"
#include "float.h"
#include "math.h"

float get_pixel(struct ethsift_image gaussian, int row, int column)
{
    int width = (int) gaussian.width;
    int height = (int) gaussian.height; 
    if (column >= 0 && column < width && row >= 0 && row < height) {
        return gaussian.pixels[row * width + column];
    }
    else if (column < 0) {
        return gaussian.pixels[row * width];
    }
    else if (column >= width) {
        return gaussian.pixels[row * width + width - 1];
    }
    else if (row < 0) {
        return gaussian.pixels[column];
    }
    else if (row >= height) {
        return gaussian.pixels[(height - 1) * width + column];
    }
    else {
        return 0.0f;
    }
}

int ethsift_generate_gradient_pyramid(struct ethsift_image gaussians[], 
                                      uint32_t gaussian_count, 
                                      struct ethsift_image gradients[], 
                                      struct ethsift_image rotations[], 
                                      uint32_t layers,
                                      uint32_t octave_count)
{
    uint32_t width, height;
    int idx;
    float d_row, d_column, angle;

    for(int i = 0; i < octave_count; i++){
        
        width = gaussians[i * gaussian_count].width;
        height = gaussians[i * gaussian_count].height;

        for(int j = 1; j <= layers; j++){

            idx = i * gaussian_count + j;

            for(int row = 0; row < height; row++){
                for(int column = 0; column < width; column++){
                    
                    d_row = get_pixel(gaussians[idx], row+1, column) - 
                            get_pixel(gaussians[idx], row-1, column);
                    
                    d_column = get_pixel(gaussians[idx], row, column+1) - 
                               get_pixel(gaussians[idx], row, column-1);
                    
                    gradients[idx].pixels[row * width + column] = sqrtf(d_row * d_row + d_column * d_column);
                    
                    angle = atan2f(d_row, d_column + FLT_MIN);
                    if(angle < 0){
                        angle += (2 * M_PI);
                    }
                    //rotations[idx].pixels[row * width + column] = angle;
                    
                }
            }

        }
        
    }

    return 1;
}
