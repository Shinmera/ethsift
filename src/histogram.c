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

    float *gradient_pixels = gradient.pixels;
    float *rotation_pixels = rotation.pixels;
    int w = gradient.width;
    int h = gradient.height;

    int r, c;
    float magnitude, angle, weight;
    int bin;
    float fbin; // float point bin

    float tmpHist[bin_count];
    memset(tmpHist, 0, bin_count * sizeof(float));
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

            fbin = angle * bin_count / M_TWOPI; // 1 MUL and + 1 DIV
            weight = expf(
                ((i - d_kptr) * (i - d_kptr) + (j - d_kptc) * (j - d_kptc)) *
                exp_factor); // 4 SUBS + 3 MULs + 1 ADD + 1 EXP

            bin = (int)(fbin - 0.5f); // 1 SUB
            float d_fbin = fbin - 0.5f - bin; // 2 SUBs

            float mw = weight * magnitude; // 1 MUL
            float dmw = d_fbin * mw;// 1 MUL
            tmpHist[(bin + bin_count) % bin_count] += mw - dmw; // 1ADD + 1 SUB
            tmpHist[(bin + 1) % bin_count] += dmw; // 1ADD
        }
    }

   
    // bin_count * 10 FLOPs
    // Smooth the histogram. Algorithm comes from OpenCV.
    histogram[0] = (tmpHist[0] + tmpHist[2]) * 1.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
              (tmpHist[0] + tmpHist[1]) * 4.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
              tmpHist[0] * 6.0f / 16.0f;  //  1 MUL + 1 DIV
    histogram[1] = (tmpHist[0] + tmpHist[3]) * 1.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
              (tmpHist[0] + tmpHist[2]) * 4.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
              tmpHist[1] * 6.0f / 16.0f; //  1 MUL + 1 DIV
    histogram[bin_count - 2] = (tmpHist[bin_count - 4] + tmpHist[bin_count - 1]) * 1.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                      (tmpHist[bin_count - 3] + tmpHist[bin_count - 1]) * 4.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                      tmpHist[bin_count - 2] * 6.0f / 16.0f; //  1 MUL + 1 DIV
    histogram[bin_count - 1] = (tmpHist[bin_count - 3] + tmpHist[bin_count - 1]) * 1.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                      (tmpHist[bin_count - 2] + tmpHist[bin_count - 1]) * 4.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                      tmpHist[bin_count - 1] * 6.0f / 16.0f; //  1 MUL + 1 DIV

    for (int i = 2; i < bin_count - 2; i++) {
        histogram[i] = (tmpHist[i - 2] + tmpHist[i + 2]) * 1.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                  (tmpHist[i - 1] + tmpHist[i + 1]) * 4.0f / 16.0f + // 2 ADD + 1 MUL + 1 DIV
                  tmpHist[i] * 6.0f / 16.0f; //  1 MUL + 1 DIV
    }

    // Find the maximum item of the histogram
    float temp = histogram[0];
    int max_i = 0;
    for (int i = 0; i < bin_count; i++) {
        if (temp < histogram[i]) {
            temp = histogram[i];
            max_i = i;
        }
    }
    *max_histval = temp;

    keypoint->orientation = max_i * M_TWOPI / bin_count; // 1 MUL + 1 DIV

    //free(tmpHist);
    //tmpHist = nullptr;
    return 1;
}