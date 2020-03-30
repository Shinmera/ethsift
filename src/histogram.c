#include "internal.h"

/// <summary> 
/// Compute the histogram for the given keypoint in the image
/// </summary>
/// <param name="gradient"> IN: DOG pyramid. </param>
/// <param name="rotation"> IN: Number of octaves. </param>
/// <param name="keypoint"> IN: Struct of gaussians to compute.
/// <param name="histogram"> OUT: Number of gaussian blurred images per layer. </param> 
/// <returns> max value in the histogram IF computation was successful, ELSE 0. </returns>
int ethsift_compute_orientation_histogram(struct ethsift_image gradient, struct ethsift_image rotation, struct ethsift_keypoint *keypoint, float *histogram, float *max_histval){
    int bin_count = ETHSIFT_ORI_HIST_BINS;

    float kptr = keypoint->global_pos.y;
    float kptc = keypoint->global_pos.x;
    float kpt_scale = keypoint->global_pos.scale;

    int kptr_i = (int)(kptr + 0.5f);
    int kptc_i = (int)(kptc + 0.5f);
    float d_kptr = kptr - kptr_i;
    float d_kptc = kptc - kptc_i;

    float sigma = ETHSIFT_ORI_SIG_FCTR * kpt_scale;
    int win_radius = (int)(ETHSIFT_ORI_RADIUS * kpt_scale);
    float exp_factor = -1.0f / (2.0f * sigma * sigma);

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

            fbin = angle * bin_count / M_TWOPI;
            weight = expf(
                ((i - d_kptr) * (i - d_kptr) + (j - d_kptc) * (j - d_kptc)) *
                exp_factor);

            bin = (int)(fbin - 0.5f);
            float d_fbin = fbin - 0.5f - bin;

            float mw = weight * magnitude;
            float dmw = d_fbin * mw;
            tmpHist[(bin + bin_count) % bin_count] += mw - dmw;
            tmpHist[(bin + 1) % bin_count] += dmw;
        }
    }

    // Smooth the histogram. Algorithm comes from OpenCV.
    histogram[0] = (tmpHist[0] + tmpHist[2]) * 1.0f / 16.0f +
              (tmpHist[0] + tmpHist[1]) * 4.0f / 16.0f +
              tmpHist[0] * 6.0f / 16.0f;
    histogram[1] = (tmpHist[0] + tmpHist[3]) * 1.0f / 16.0f +
              (tmpHist[0] + tmpHist[2]) * 4.0f / 16.0f +
              tmpHist[1] * 6.0f / 16.0f;
    histogram[bin_count - 2] = (tmpHist[bin_count - 4] + tmpHist[bin_count - 1]) * 1.0f / 16.0f +
                      (tmpHist[bin_count - 3] + tmpHist[bin_count - 1]) * 4.0f / 16.0f +
                      tmpHist[bin_count - 2] * 6.0f / 16.0f;
    histogram[bin_count - 1] = (tmpHist[bin_count - 3] + tmpHist[bin_count - 1]) * 1.0f / 16.0f +
                      (tmpHist[bin_count - 2] + tmpHist[bin_count - 1]) * 4.0f / 16.0f +
                      tmpHist[bin_count - 1] * 6.0f / 16.0f;

    for (int i = 2; i < bin_count - 2; i++) {
        histogram[i] = (tmpHist[i - 2] + tmpHist[i + 2]) * 1.0f / 16.0f +
                  (tmpHist[i - 1] + tmpHist[i + 1]) * 4.0f / 16.0f +
                  tmpHist[i] * 6.0f / 16.0f;
    }


    // Find the maximum item of the histogram
    *max_histval = histogram[0];
    int max_i = 0;
    for (int i = 0; i < bin_count; i++) {
        if (*max_histval < histogram[i]) {
            *max_histval = histogram[i];
            max_i = i;
        }
    }

    keypoint->orientation = max_i * M_TWOPI / bin_count;

    //free(tmpHist);
    //tmpHist = nullptr;
    return 1;
}