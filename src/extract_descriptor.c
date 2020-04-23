#include "internal.h"

/// <summary> 
/// Extract the keypoint descriptors.
/// </summary>
/// <param name="gradients"> IN: Gradients pyramid. </param>
/// <param name="rotations"> IN: Rotation pyramid.  </param>
/// <param name="octave_count"> IN: Number of Octaves. </param> 
/// <param name="gaussian_count"> IN: Number of gaussian layers. </param> 
/// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
/// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
///                               OUT: Number of keypoints found. </param> 
/// <returns> 1 IF computation was successful, ELSE 0. </returns>
int ethsift_extract_descriptor(struct ethsift_image gradients[], 
                              struct ethsift_image rotations[], 
                              uint32_t octave_count, 
                              uint32_t gaussian_count, 
                              struct ethsift_keypoint keypoints[], 
                              uint32_t keypoint_count)
{
    // Number of subregions, default 4x4 subregions.
    // The width of subregion is determined by the scale of the keypoint.
    // Or, in Lowe's SIFT paper[2004], width of subregion is 16x16.
    int nSubregion = ETHSIFT_DESCR_WIDTH;
    int nHalfSubregion = nSubregion >> 1;

    // Number of histogram bins for each descriptor subregion.
    int nBinsPerSubregion = ETHSIFT_DESCR_HIST_BINS;
    float nBinsPerSubregionPerDegree = (float)nBinsPerSubregion / M_TWOPI;

    // 3-D structure for histogram bins (rbin, cbin, obin);
    // (rbin, cbin, obin) means (row of hist bin, column of hist bin,
    // orientation bin) In Lowe's paper, 4x4 histogram, each has 8 bins. that
    // means for each (rbin, cbin), there are 8 bins in the histogram.

    // In this implementation, histBin is a circular buffer.
    // we expand the cube by 1 for each direction.
    int nBins = nSubregion * nSubregion * nBinsPerSubregion;
    int nHistBins = (nSubregion + 2) * (nSubregion + 2) * (nBinsPerSubregion + 2);
    int nSliceStep = (nSubregion + 2) * (nBinsPerSubregion + 2);
    int nRowStep = (nBinsPerSubregion + 2);
    float histBin[nHistBins];

    float exp_scale = -2.0f / (nSubregion * nSubregion);

    struct ethsift_keypoint* kpt;
    for (int k = 0; k < keypoint_count; ++k) {
        kpt = &keypoints[k];
        // Keypoint information
        int octave = kpt->octave;
        int layer = kpt->layer;

        float kpt_ori = kpt->orientation;
        float kptr = kpt->layer_pos.y;
        float kptc = kpt->layer_pos.x;
        float kpt_scale = kpt->layer_pos.scale;

        // Nearest coordinate of keypoints
        int kptr_i = (int)(kptr + 0.5f);
        int kptc_i = (int)(kptc + 0.5f);
        float d_kptr = kptr_i - kptr;
        float d_kptc = kptc_i - kptc;

        int layer_index = octave * gaussian_count + layer;
        int w = gradients[layer_index].width;
        int h = gradients[layer_index].height;

        // Note for Gaussian weighting.
        // OpenCV and vl_feat uses non-fixed size of subregion.
        // But they all use (0.5 * 4) as the Gaussian weighting sigma.
        // In Lowe's paper, he uses 16x16 sample region,
        // partition 16x16 region into 16 4x4 subregion.
        float size_desc = ETHSIFT_DESCR_SCL_FCTR;
        float subregion_width = size_desc * kpt_scale;
        int win_size =
            (int)(M_SQRT2 * subregion_width * (nSubregion + 1) * 0.5f + 0.5f);

        // Normalized cos() and sin() value.
        float sin_t = sinf(kpt_ori) / (float)subregion_width;
        float cos_t = cosf(kpt_ori) / (float)subregion_width;

        // Re-init histBin
        memset(histBin, 0, nHistBins * sizeof(float));

        // Start to calculate the histogram in the sample region.
        float rr, cc;
        float mag, angle, gaussian_weight;

        // Used for tri-linear interpolation.
        // int rbin_i, cbin_i, obin_i;
        float rrotate, crotate;
        float rbin, cbin, obin;
        float d_rbin, d_cbin, d_obin;

        // Boundary of sample region.
        int r, c;
        int left = int_max(-win_size, 1 - kptc_i);
        int right = int_min(win_size, w - 2 - kptc_i);
        int top = int_max(-win_size, 1 - kptr_i);
        int bottom = int_min(win_size, h - 2 - kptr_i);

        for (int i = top; i <= bottom; i++) // rows
        {
            for (int j = left; j <= right; j++) // columns
            {
                // Accurate position relative to (kptr, kptc)
                rr = i + d_kptr;
                cc = j + d_kptc;

                // Rotate the coordinate of (i, j)
                rrotate = (cos_t * cc + sin_t * rr);
                crotate = (-sin_t * cc + cos_t * rr);

                // Since for a bin array with 4x4 bins, the center is actually
                // at (1.5, 1.5)
                rbin = rrotate + nHalfSubregion - 0.5f;
                cbin = crotate + nHalfSubregion - 0.5f;

                // rbin, cbin range is (-1, d); if outside this range, then the
                // pixel is counted.
                if (rbin <= -1 || rbin >= nSubregion || cbin <= -1 ||
                    cbin >= nSubregion)
                    continue;

                // All the data need for gradient computation are valid, no
                // border issues.
                r = kptr_i + i;
                c = kptc_i + j;
                mag = gradients[layer_index].pixels[r * w + c];
                angle = rotations[layer_index].pixels[r * w + c] - kpt_ori;
                float angle1 = (angle < 0) ? (M_TWOPI + angle) : angle; // Adjust angle to [0, 2PI)
                obin = angle1 * nBinsPerSubregionPerDegree;

                int x0, y0, z0;
                int x1, y1;
                y0 = (int)floor(rbin);
                x0 = (int)floor(cbin);
                z0 = (int)floor(obin);
                d_rbin = rbin - y0;
                d_cbin = cbin - x0;
                d_obin = obin - z0;
                x1 = x0 + 1;
                y1 = y0 + 1;


                // Gaussian weight relative to the center of sample region.
                gaussian_weight =
                    expf((rrotate * rrotate + crotate * crotate) * exp_scale);

                // Gaussian-weighted magnitude
                float gm = mag * gaussian_weight;
                // Tri-linear interpolation

                float vr1, vr0;
                float vrc11, vrc10, vrc01, vrc00;
                float vrco110, vrco111, vrco100, vrco101, vrco010, vrco011,
                    vrco000, vrco001;

                vr1 = gm * d_rbin;
                vr0 = gm - vr1;
                vrc11 = vr1 * d_cbin;
                vrc10 = vr1 - vrc11;
                vrc01 = vr0 * d_cbin;
                vrc00 = vr0 - vrc01;
                vrco111 = vrc11 * d_obin;
                vrco110 = vrc11 - vrco111;
                vrco101 = vrc10 * d_obin;
                vrco100 = vrc10 - vrco101;
                vrco011 = vrc01 * d_obin;
                vrco010 = vrc01 - vrco011;
                vrco001 = vrc00 * d_obin;
                vrco000 = vrc00 - vrco001;

                // int idx =  y0  * nSliceStep + x0  * nRowStep + z0;
                // All coords are offseted by 1. so x=[1, 4], y=[1, 4];
                // data for -1 coord is stored at position 0;
                // data for 8 coord is stored at position 9.
                // z doesn't need to move.
                int idx = y1 * nSliceStep + x1 * nRowStep + z0;
                histBin[idx] += vrco000;

                idx++;
                histBin[idx] += vrco001;

                idx += nRowStep - 1;
                histBin[idx] += vrco010;

                idx++;
                histBin[idx] += vrco011;

                idx += nSliceStep - nRowStep - 1;
                histBin[idx] += vrco100;

                idx++;
                histBin[idx] += vrco101;

                idx += nRowStep - 1;
                histBin[idx] += vrco110;

                idx++;
                histBin[idx] += vrco111;
            }
        }

        // Discard all the edges for row and column.
        // Only retrive edges for orientation bins.
        float dstBins[nBins];
        for (int i = 1; i <= nSubregion; i++) // slice
        {
            for (int j = 1; j <= nSubregion; j++) // row
            {
                int idx = i * nSliceStep + j * nRowStep;
                // comments: how this line works.
                // Suppose you want to write w=width, y=1, due to circular
                // buffer, we should write it to w=0, y=1; since we use a
                // circular buffer, it is written into w=width, y=1. Now, we
                // fectch the data back.
                histBin[idx] = histBin[idx + nBinsPerSubregion];

                // comments: how this line works.
                // Suppose you want to write x=-1 y=1, due to circular, it
                // should be at y=1, x=width-1; since we use circular buffer,
                // the value goes to y=0, x=width, now, we need to get it back.
                if (idx != 0)
                    histBin[idx + nBinsPerSubregion + 1] = histBin[idx - 1];

                int idx1 = ((i - 1) * nSubregion + j - 1) * nBinsPerSubregion;
                for (int k = 0; k < nBinsPerSubregion; k++) {
                    dstBins[idx1 + k] = histBin[idx + k];
                }
            }
        }

        // Normalize the histogram
        float sum_square = 0.0f;
        for (int i = 0; i < nBins; i++)
            sum_square += dstBins[i] * dstBins[i];

        float thr = fast_sqrt_f(sum_square) * ETHSIFT_DESCR_MAG_THR;

        float tmp = 0.0;
        sum_square = 0.0;
        // Cut off the numbers bigger than 0.2 after normalized.
        for (int i = 0; i < nBins; i++) {
            tmp = float_min(thr, dstBins[i]);
            dstBins[i] = tmp;
            sum_square += tmp * tmp;
        }

// Re-normalize
// The numbers are usually too small to store, so we use
// a constant factor to scale up the numbers.
        float conv_f_to_char = ETHSIFT_INT_DESCR_FCTR;
        float norm_factor = conv_f_to_char / fast_sqrt_f(sum_square);
        for (int i = 0; i < nBins; i++)
            dstBins[i] = dstBins[i] * norm_factor;

        memcpy(kpt->descriptors, dstBins, nBins * sizeof(float));
    }

  return 1;
}