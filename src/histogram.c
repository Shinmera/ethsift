#include "internal.h"

/// <summary> 
/// Compute the histogram for the given keypoints in the image.
/// </summary>
/// <param name="gradient"> IN: DOG pyramid. </param>
/// <param name="rotation"> IN: Rotation pyramid. </param>
/// <param name="keypoint"> IN: Detected Keypoints.
/// <param name="histogram"> OUT: Histogram of the detected keypoints. </param> 
/// <returns> max value in the histogram IF computation was successful, ELSE 0. </returns>
/// <remarks> 11 + (2*win_radius+1)^2 * (18 + EXP) + (bin_count * 10) FLOPs </remarks>
int ethsift_compute_orientation_histogram(struct ethsift_image gradient, 
                                          struct ethsift_image rotation, 
                                          struct ethsift_keypoint *keypoint, 
                                          float *histogram, 
                                          float *max_histval)
{
    int bin_count = ETHSIFT_ORI_HIST_BINS;

    float kptr = keypoint->layer_pos.y;
    float kptc = keypoint->layer_pos.x;
    float kpt_scale = keypoint->layer_pos.scale;

    int kptr_i = (int)(kptr + 0.5f); // 1 ADD
    int kptc_i = (int)(kptc + 0.5f); // 1 ADD
    float d_kptr = kptr - kptr_i; // 1SUB
    float d_kptc = kptc - kptc_i; // 1SUB
    
    inc_adds(4);

    float sigma = ETHSIFT_ORI_SIG_FCTR * kpt_scale; // 1MUL
    int win_radius = (int)(ETHSIFT_ORI_RADIUS * kpt_scale); // 1MUL
    float exp_factor = -1.0f / (2.0f * sigma * sigma); // 2MUL + 1 DIV

    inc_mults(4);
    inc_div(1);

    float *gradient_pixels = gradient.pixels;
    float *rotation_pixels = rotation.pixels;
    int w = gradient.width;
    int h = gradient.height;

    int r, c;
    float magnitude, angle, weight;
    int bin;
    float fbin; // float point bin

    float tmpHist[ETHSIFT_ORI_HIST_BINS] = {0};
    // (2*win_radius+1)^2 * (18 + EXP)
    for (int i = -win_radius; i <= win_radius; i++) // rows
    {
        r = kptr_i + i;
        if (r <= 0 || r >= h - 1) // Cannot calculate dy
            continue;
        for (int j = -win_radius; j <= win_radius; j++) // columns
        {
            c = kptc_i + j;
            if (c <= 0 || c >= w - 1)
                continue;

            magnitude = gradient_pixels[r * w + c];
            angle = rotation_pixels[r * w + c];
            
            inc_mem(2);

            fbin = angle * bin_count * M_1_2PI;
            float w1 = i-d_kptr;
            float w2 = j-d_kptc;
            weight = expf((w1*w1 + w2*w2) * exp_factor);

            inc_mults(5);
            inc_adds(3);
            // 1 EXP

            bin = (int)(fbin - 0.5f); // 1 SUB
            float d_fbin = fbin - 0.5f - bin; // 2 SUBs
            
            inc_adds(3);

            float mw = weight * magnitude; // 1 MUL
            float dmw = d_fbin * mw;// 1 MUL
            tmpHist[(bin + bin_count) % bin_count] += mw - dmw; // 1ADD + 1 SUB
            tmpHist[(bin + 1) % bin_count] += dmw; // 1ADD

            inc_mults(2);
            inc_adds(3);
            inc_mem(4); // 2 reads / 2writes
        }
    }

    // bin_count * 10 FLOPs
    // Smooth the histogram. Algorithm comes from OpenCV.
    float div1 = 1.0f/16.0f;
    float div2 = 4.0f/16.0f;
    float div3 = 6.0f/16.0f;

    float tmpHist0 = tmpHist[0];
    histogram[0] = (tmpHist0 + tmpHist[2]) * div1 +
      (tmpHist0 + tmpHist[1]) * div2 +
      tmpHist0 * div3;
    inc_mults(3);
    inc_adds(4);
    inc_mem(4);

    histogram[1] = (tmpHist0 + tmpHist[3]) * div1 +
      (tmpHist0 + tmpHist[2]) * div2 +
      tmpHist[1] * div3;
    inc_mults(3);
    inc_adds(4);
    inc_mem(4);

    float tmpHistb1 = tmpHist[bin_count - 1];
    histogram[bin_count - 2] = (tmpHist[bin_count - 4] + tmpHistb1) * div1 +
      (tmpHist[bin_count - 3] + tmpHistb1) * div2 +
      tmpHist[bin_count - 2] * div3;
    inc_mults(3);
    inc_adds(4);
    inc_mem(5);
    
    histogram[bin_count - 1] = (tmpHist[bin_count - 3] + tmpHistb1) * div1 +
      (tmpHist[bin_count - 2] + tmpHistb1) * div2 +
      tmpHistb1 * div3;
    inc_mults(3);
    inc_adds(4);
    inc_mem(3);

    for (int i = 2; i < bin_count - 2; i++) {
      histogram[i] = (tmpHist[i - 2] + tmpHist[i + 2]) * div1 +
        (tmpHist[i - 1] + tmpHist[i + 1]) * div2 +
        tmpHist[i] * div3;

      inc_mults(3);
      inc_adds(4);
      inc_mem(6);
    }

    // Find the maximum item of the histogram
    float temp = histogram[0];

    inc_mem(1);

    int max_i = 0;
    for (int i = 0; i < bin_count; i++) {
        if (temp < histogram[i]) {
            temp = histogram[i];
            max_i = i;
        }
        inc_mem(1); // 1 read
    }
    *max_histval = temp;

    keypoint->orientation = max_i * M_TWOPI / bin_count; // 1 MUL + 1 DIV

    inc_mults(1);
    inc_div(1);
    return 1;
}
