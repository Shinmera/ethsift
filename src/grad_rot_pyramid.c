#include "internal.h"
#include "float.h"
#include "math.h"

#define EPSILON_F 1.19209290E-07F
#define PI_4 0.785398163397448f
#define PI_3_4 2.356194490192345f

float pixel_w_zeropadding(struct ethsift_image gaussian, int row, int column)
{
    int width =  gaussian.width;
    int height =  gaussian.height; 
    float val;
    if (column >= 0 && column < width && row >= 0 && row < height) {
        val = gaussian.pixels[row * width + column];
    }
    else if (column < 0) {
        val = gaussian.pixels[row * width];
    }
    else if (column >= width) {
        val = gaussian.pixels[row * width + width - 1];
    }
    else if (row < 0) {
        val = gaussian.pixels[column];
    }
    else if (row >= height) {
        val = gaussian.pixels[(height - 1) * width + column];
    }
    else {
        val = 0.0f;
    }

    return val;
} 

float arctan_2f(float y, float x)
{
    float angle, r;
    float const c3 = 0.1821F;
    float const c1 = 0.9675F;
    float abs_y = fabsf(y) + EPSILON_F;

    if (x >= 0) {
        r = (x - abs_y) / (x + abs_y);
        angle = PI_4;
    }
    else {
        r = (x + abs_y) / (abs_y - x);
        angle = PI_3_4;
    }
    angle += (c3 * r * r - c1) * r;

    return (y < 0) ? M_TWOPI - angle : angle;
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

            idx = i * octave_count + j;         

            for(int row = 0; row < height; row++){
                for(int column = 0; column < width; column++){

                    d_row = pixel_w_zeropadding(gaussians[idx], row + 1, column) - 
                            pixel_w_zeropadding(gaussians[idx], row - 1, column);
                    
                    d_column = pixel_w_zeropadding(gaussians[idx], row, column + 1) - 
                               pixel_w_zeropadding(gaussians[idx], row, column - 1);
                    
                    gradients[idx].pixels[row * width + column] = sqrtf(pow(d_row, 2) + pow(d_column, 2));
                    rotations[idx].pixels[row * width + column] = arctan_2f(d_row, d_column);
                    
                }
            }

        }
        
    }

    return 1;
}
