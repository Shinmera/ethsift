#include "internal.h"

static inline int is_local_max(float pixel, int pos, int w, float *curData, float *lowData, float *highData) {
  int val;
  val = pixel > highData[pos - w - 1] &&
        pixel > highData[pos - w] &&
        pixel > highData[pos - w + 1] &&
        pixel > highData[pos - 1] && pixel > highData[pos] &&
        pixel > highData[pos + 1] &&
        pixel > highData[pos + w - 1] &&
        pixel > highData[pos + w] &&
        pixel > highData[pos + w + 1] &&
        pixel > curData[pos - w - 1] &&
        pixel > curData[pos - w] &&
        pixel > curData[pos - w + 1] &&
        pixel > curData[pos - 1] &&
        pixel > curData[pos + 1] &&
        pixel > curData[pos + w - 1] &&
        pixel > curData[pos + w] &&
        pixel > curData[pos + w + 1] &&
        pixel > lowData[pos - w - 1] &&
        pixel > lowData[pos - w] &&
        pixel > lowData[pos - w + 1] &&
        pixel > lowData[pos - 1] && pixel > lowData[pos] &&
        pixel > lowData[pos + 1] &&
        pixel > lowData[pos + w - 1] &&
        pixel > lowData[pos + w] &&
        pixel > lowData[pos + w + 1];

  inc_mem(26); // worst case
  return val;
}

static inline int is_local_min(float pixel, int pos, int w, float *curData, float *lowData, float *highData) {
  int val;
  val = pixel < highData[pos - w - 1] &&
        pixel < highData[pos - w] &&
        pixel < highData[pos - w + 1] &&
        pixel < highData[pos - 1] && pixel < highData[pos] &&
        pixel < highData[pos + 1] &&
        pixel < highData[pos + w - 1] &&
        pixel < highData[pos + w] &&
        pixel < highData[pos + w + 1] &&
        pixel < curData[pos - w - 1] &&
        pixel < curData[pos - w] &&
        pixel < curData[pos - w + 1] &&
        pixel < curData[pos - 1] &&
        pixel < curData[pos + 1] &&
        pixel < curData[pos + w - 1] &&
        pixel < curData[pos + w] &&
        pixel < curData[pos + w + 1] &&
        pixel < lowData[pos - w - 1] &&
        pixel < lowData[pos - w] &&
        pixel < lowData[pos - w + 1] &&
        pixel < lowData[pos - 1] && pixel < lowData[pos] &&
        pixel < lowData[pos + 1] &&
        pixel < lowData[pos + w - 1] &&
        pixel < lowData[pos + w] &&
        pixel < lowData[pos + w + 1];
  
  inc_mem(26); // worst case
  return val;
}


/// <summary> 
/// Detect the keypoints in the image that SIFT finds interesting.
/// </summary>
/// <param name="differences"> IN: DOG pyramid. </param>
/// <param name="gradients"> IN: Gradients pyramid. </param>
/// <param name="rotations"> IN: Rotation pyramid.  </param>
/// <param name="octave_count"> IN: Number of octaves. </param> 
/// <param name="gaussian_count"> IN: Number of layers. </param> 
/// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
/// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
///                               OUT: Number of keypoints found. </param> 
/// <returns> 1 IF computation was successful, ELSE 0. </returns>
int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  
  // Settings
  int image_border = ETHSIFT_IMG_BORDER;
  float contr_thr = ETHSIFT_CONTR_THR;
  float orientation_peak_ratio = ETHSIFT_ORI_PEAK_RATIO;
  
  float threshold = 0.8f * contr_thr;
  
  inc_mults(1);
  
  // Layers of DoG
  int layersDoG = gaussian_count - 1;

  // Requested number of keypoints and actual number of keypoints
  int keypoints_required = *keypoint_count;
  int keypoints_found = 0;
  int keypoints_current = 0;

  struct ethsift_keypoint temp;
  
  // Loop variables
  float *curData;
  float *lowData;
  float *highData;

  int w, h;
  int layer_ind, pos;
  float pixel;

  // Histogram
  int nBins = ETHSIFT_ORI_HIST_BINS;
  float hist[nBins];
  float max_mag;

  for (int i = 0; i < octave_count; ++i) {
    w = (int) differences[i * layersDoG].width;
    h = (int) differences[i * layersDoG].height;

    inc_mem(2);

    // (h-10)(w-10)(11 + rle + coh)*layersDoG*
    for (int j = 1; j < layersDoG - 1; ++j) {
      layer_ind = i * layersDoG + j;

      highData = differences[layer_ind + 1].pixels; 
      curData  = differences[layer_ind ].pixels; 
      lowData  = differences[layer_ind - 1].pixels; 

      inc_mem(3);

      // (h-10)(w-10)(11 + rle + coh)
      // Iterate over all pixels in image, ignore border values
      for (int r = image_border; r < h - image_border; ++r) {
        for (int c = image_border; c < w - image_border; ++c) {
          // Pixel position and value
          pos = r * w + c;
          pixel = curData[pos];

          inc_mem(1);

          // Test if pixel value is an extrema:
          int isExtrema =
            (pixel >= threshold  && is_local_max(pixel, pos, w, curData, lowData, highData)) ||
            (pixel <= -threshold && is_local_min(pixel, pos, w, curData, lowData, highData));

          // 11 + rle + coh
          if (isExtrema) {
            if (keypoints_found < keypoints_required) {
              keypoints[keypoints_current].layer = j;
              keypoints[keypoints_current].octave = i;

              keypoints[keypoints_current].layer_pos.y = (float) r;
              keypoints[keypoints_current].layer_pos.x = (float) c;
  
              // EzSift does the refinement here and decides at this moment if the keypoint is useable
              int isGoodKeypoint = ethsift_refine_local_extrema(differences, octave_count, gaussian_count, &keypoints[keypoints_current]);
              
              if (!isGoodKeypoint) {
                continue;
              }
              
              ethsift_compute_orientation_histogram(
                gradients[i * ((int) gaussian_count) + j], 
                rotations[i * ((int) gaussian_count) + j], 
                &(keypoints[keypoints_current]), 
                hist, &max_mag);

              float hist_threshold = max_mag * orientation_peak_ratio; // 1 MUL

              for (int ii = 0; ii < nBins; ++ii) {
                int left = ii > 0 ? ii - 1 : nBins - 1;
                int right = ii < (nBins - 1) ? ii + 1 : 0;
                float currHist = hist[ii];
                float lhist = hist[left];
                float rhist = hist[right];

                inc_mem(3);

                if (currHist > lhist && currHist > rhist &&
                  currHist > hist_threshold) {
                  // Refer to here:
                  // http://stackoverflow.com/questions/717762/how-to-calculate-the-vertex-of-a-parabola-given-three-points
                  float accu_ii =
                    ii + 0.5f * (lhist - rhist) /
                    (lhist - 2.0f * currHist + rhist);  // 2 ADD + 2 SUBs + 2 MULs

                  inc_adds(4);
                  inc_mults(2);

                  // Since bin index means the starting point of a
                  // bin, so the real orientation should be bin
                  // index plus 0.5. for example, angles in bin 0
                  // should have a mean value of 5 instead of 0;
                  accu_ii += 0.5f; // 1 ADD
                  accu_ii = accu_ii < 0 ? (accu_ii + nBins) // 1 ADD
                                        : accu_ii >= nBins
                                          ? (accu_ii - nBins) // 1 SUB
                                          : accu_ii;

                  #ifdef IS_COUNTING
                  if (accu_ii < 0) {
                    inc_adds(1);
                  } else if (accu_ii >= nBins) {
                    inc_adds(1);
                  }
                  #endif
                  // The magnitude should also calculate the max
                  // number based on fitting But since we didn't
                  // actually use it in image matching, we just
                  // lazily use the histogram value.
                  keypoints[keypoints_current].magnitude = currHist;
                  keypoints[keypoints_current].orientation = accu_ii * M_TWOPI / nBins; // 1 MUL + 1 DIV

                  inc_mults(1);
                  inc_div(1);

                  // Update keypoint counters
                  ++keypoints_current;
                  ++keypoints_found;
                  
                  if (keypoints_current >= keypoints_required) {
                    break;
                  }

                  // Copy values of previous keypoint to possible new keypoint
                  keypoints[keypoints_current].layer = keypoints[keypoints_current - 1].layer;
                  keypoints[keypoints_current].octave = keypoints[keypoints_current - 1].octave;

                  keypoints[keypoints_current].layer_pos = keypoints[keypoints_current - 1].layer_pos;
                  keypoints[keypoints_current].global_pos =  keypoints[keypoints_current - 1].global_pos;

                }
              }    
            } else {
              // Still test keypoint if usable, to find the actual number of keypoints
              temp.layer = j;
              temp.octave = i;

              temp.layer_pos.y = (float) r;
              temp.layer_pos.x = (float) c;

              // EzSift does the refinement here and decides at this moment if the keypoint is useable
              int isGoodKeypoint = ethsift_refine_local_extrema(differences, octave_count, gaussian_count, &temp);
              
              if (!isGoodKeypoint) {
                continue;
              }

              ethsift_compute_orientation_histogram(
                gradients[i * ((int) gaussian_count) + j], 
                rotations[i * ((int) gaussian_count) + j], 
                &temp, 
                hist, &max_mag);

              inc_mem(2); //?

              float hist_threshold = max_mag * orientation_peak_ratio; // 1 MUL

              inc_mults(1);

              for (int ii = 0; ii < nBins; ++ii) {
                int left = ii > 0 ? ii - 1 : nBins - 1;
                int right = ii < (nBins - 1) ? ii + 1 : 0;
                float currHist = hist[ii];
                float lhist = hist[left];
                float rhist = hist[right];

                if (currHist > lhist && currHist > rhist &&
                  currHist > hist_threshold) {

                  // Update keypoint counter
                  ++keypoints_found;      
                }
              } 
            }     
          }
        }
      }
    }
  }

  // Update count with actual number of keypoints found
  *keypoint_count = keypoints_found;
  
  return 1;
}