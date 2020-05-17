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


    for(int i = 0; i < octave_count; i++){
        
        width = (int) gaussians[i * gaussian_count].width;
        height = (int) gaussians[i * gaussian_count].height;
        inc_mem(2);


        idx = i * gaussian_count + 1;         

        for(int row = 0; row < height; row++){
            for(int column = 0; column < width; column++){

                d_row = get_pixel_f(gaussians[idx].pixels, width, height, row+1, column) - 
                        get_pixel_f(gaussians[idx].pixels, width, height, row-1, column);
                    
                d_column = get_pixel_f(gaussians[idx].pixels, width, height, row, column+1) - 
                            get_pixel_f(gaussians[idx].pixels, width, height, row, column-1);
                    
                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?
                    
                gradients[idx].pixels[row * width + column] = fast_sqrt_f(d_row * d_row + d_column * d_column);
                rotations[idx].pixels[row * width + column] = fast_atan2_f(d_row, d_column); 
                inc_mem(2); // At least two writes
            }
        }

        idx = i * gaussian_count + 2;

        for (int row = 0; row < height; row++) {
            for (int column = 0; column < width; column++) {

                d_row = get_pixel_f(gaussians[idx].pixels, width, height, row + 1, column) -
                    get_pixel_f(gaussians[idx].pixels, width, height, row - 1, column);

                d_column = get_pixel_f(gaussians[idx].pixels, width, height, row, column + 1) -
                    get_pixel_f(gaussians[idx].pixels, width, height, row, column - 1);

                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?

                gradients[idx].pixels[row * width + column] = fast_sqrt_f(d_row * d_row + d_column * d_column);
                rotations[idx].pixels[row * width + column] = fast_atan2_f(d_row, d_column);
                inc_mem(2); // At least two writes
            }
        }

        idx = i * gaussian_count + 3;

        for (int row = 0; row < height; row++) {
            for (int column = 0; column < width; column++) {

                d_row = get_pixel_f(gaussians[idx].pixels, width, height, row + 1, column) -
                    get_pixel_f(gaussians[idx].pixels, width, height, row - 1, column);

                d_column = get_pixel_f(gaussians[idx].pixels, width, height, row, column + 1) -
                    get_pixel_f(gaussians[idx].pixels, width, height, row, column - 1);

                inc_adds(2); // 2 Subtractions
                inc_mem(4); // Maybe?

                gradients[idx].pixels[row * width + column] = fast_sqrt_f(d_row * d_row + d_column * d_column);
                rotations[idx].pixels[row * width + column] = fast_atan2_f(d_row, d_column);
                inc_mem(2); // At least two writes
            }
        }
     
    }

    return 1;
}
