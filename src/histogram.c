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
                                          float *max_histval){
  const int bin_count = ETHSIFT_ORI_HIST_BINS;
  const float kptr = keypoint->layer_pos.y;
  const float kptc = keypoint->layer_pos.x;
  const float kpt_scale = keypoint->layer_pos.scale;
  const int kptr_i = (int)(kptr + 0.5f); // 1 ADD
  const int kptc_i = (int)(kptc + 0.5f); // 1 ADD
  const float d_kptr = kptr - kptr_i; // 1SUB
  const float d_kptc = kptc - kptc_i; // 1SUB
  const float sigma = ETHSIFT_ORI_SIG_FCTR * kpt_scale; // 1MUL
  const int win_radius = (int)(ETHSIFT_ORI_RADIUS * kpt_scale); // 1MUL
  const float exp_factor = -1.0f / (2.0f * sigma * sigma); // 2MUL + 1 DIV

  inc_adds(4);
  inc_mults(4);
  inc_div(1);

  const float *gradient_pixels = gradient.pixels;
  const float *rotation_pixels = rotation.pixels;
  const int w = gradient.width;
  const int h = gradient.height;

  float tmpHist[ETHSIFT_ORI_HIST_BINS] = {0};
  const int is = int_max(1, kptr_i-win_radius)-kptr_i;
  const int ie = int_min(h-2, kptr_i+win_radius)-kptr_i;
  const int js = int_max(1, kptc_i-win_radius)-kptc_i;
  const int je = int_min(w-2, kptc_i+win_radius)-kptc_i;
  // (2*win_radius+1)^2 * (18 + EXP)
  for (int i = is; i <= ie; i++){
    const int r = kptr_i + i;
    for (int j = js; j <= je; j++){
      const int c = kptc_i + j;
      const float magnitude = gradient_pixels[r * w + c];
      const float angle = rotation_pixels[r * w + c];
      inc_read(2, float);

      const float fbin = angle * bin_count * M_1_2PI;
      const float w1 = i-d_kptr;
      const float w2 = j-d_kptc;
      const float weight = expf((w1*w1 + w2*w2) * exp_factor);

      inc_mults(5);
      inc_adds(3);
      // 1 EXP

      const int bin = (int)(fbin - 0.5f); // 1 SUB
      const float d_fbin = fbin - 0.5f - bin; // 2 SUBs
      inc_adds(3);

      const float mw = weight * magnitude; // 1 MUL
      const float dmw = d_fbin * mw;// 1 MUL
      tmpHist[(bin + bin_count) % bin_count] += mw - dmw; // 1ADD + 1 SUB
      tmpHist[(bin + 1) % bin_count] += dmw; // 1ADD
      inc_write(2, float);

      inc_mults(2);
      inc_adds(3);
    }
  }

  // bin_count * 10 FLOPs
  // Smooth the histogram. Algorithm comes from OpenCV.
  const float div1 = 1.0f/16.0f;
  const float div2 = 4.0f/16.0f;
  const float div3 = 6.0f/16.0f;

  const float tmpHist0 = tmpHist[0];
  histogram[0] = (tmpHist0 + tmpHist[2]) * div1 +
    (tmpHist0 + tmpHist[1]) * div2 +
    tmpHist0 * div3;
  inc_mults(3);
  inc_adds(4);
  inc_read(4, float);
  inc_write(1, float);

  histogram[1] = (tmpHist0 + tmpHist[3]) * div1 +
    (tmpHist0 + tmpHist[2]) * div2 +
    tmpHist[1] * div3;
  inc_mults(3);
  inc_adds(4);
  inc_read(3, float);
  inc_write(1, float);

  const float tmpHistb1 = tmpHist[bin_count - 1];
  histogram[bin_count - 2] = (tmpHist[bin_count - 4] + tmpHistb1) * div1 +
    (tmpHist[bin_count - 3] + tmpHistb1) * div2 +
    tmpHist[bin_count - 2] * div3;
  inc_mults(3);
  inc_adds(4);
  inc_read(3, float);
  inc_write(1, float);
    
  histogram[bin_count - 1] = (tmpHist[bin_count - 3] + tmpHistb1) * div1 +
    (tmpHist[bin_count - 2] + tmpHistb1) * div2 +
    tmpHistb1 * div3;
  inc_mults(3);
  inc_adds(4);
  inc_read(2, float);
  inc_write(1, float);

  for (int i = 2; i < bin_count - 2; i++) {
    histogram[i] = (tmpHist[i - 2] + tmpHist[i + 2]) * div1 +
      (tmpHist[i - 1] + tmpHist[i + 1]) * div2 +
      tmpHist[i] * div3;

    inc_mults(3);
    inc_adds(4);
    inc_read(5, float);
    inc_write(1, float);
  }

  // Find the maximum item of the histogram
  float temp = histogram[0];
  inc_read(1, float);

  int max_i = 0;
  for (int i = 1; i < bin_count; i++) {
    if (temp < histogram[i]) {
      temp = histogram[i];
      max_i = i;
    }
    inc_read(1, float);
  }
  *max_histval = temp;
  inc_write(1, float);

  keypoint->orientation = max_i * M_TWOPI / bin_count; // 1 MUL + 1 DIV
  inc_write(1, float);
  inc_mults(1);
  inc_div(1);
  return 1;
}
