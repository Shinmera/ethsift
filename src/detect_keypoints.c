#include "internal.h"

static int is_local_max(float pixel, int pos, int w, float *curData, float *lowData, float *highData) {
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
  return val;
}

static int is_local_min(float pixel, int pos, int w, float *curData, float *lowData, float *highData) {
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
  return val;
}


/// <summary> 
/// Detect the keypoints in the image that SIFT finds interesting.
/// </summary>
/// <param name="differences"> IN: DOG pyramid. </param>
/// <param name="gradients"> IN: Gradients pyramid. </param>
/// <param name="rotations"> IN: Rotation pyramid.  </param>
/// <param name="octaves"> IN: Number of Octaves. </param> 
/// <param name="layers"> IN: Number of layers. </param> 
/// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
/// <param name="keypoint_count"> IN: How many keypoints we can store at most (allocated size of memory).
///                               OUT: Number of keypoints found. </param> 
/// <returns> 1 IF computation was successful, ELSE 0. </returns>
int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  
  // Settings as in EzSift
  // TODO Move to settings.h
  int SIFT_IMAGE_BORDER = 5;
  float SIFT_CONTR_THR = 8.0f;
  float SIFT_ORI_PEAK_RATIO = 0.8f;
  
  float threshold = 0.8f * SIFT_CONTR_THR;
  
  // Layers of DoG
  int layersDoG = layers - 1;

  // Requested number of keypoints and actual number of keypoints
  uint32_t keypoints_required = *keypoint_count;
  uint32_t keypoints_found = 0;
  uint32_t keypoints_current = 0;

  struct ethsift_keypoint temp;

  // Loop variables
  float *curData;
  float *lowData;
  float *highData;

  uint32_t w, h;
  int layer_ind, pos;
  float pixel;

  // Histogram
  int nBins = ETHSIFT_ORI_HIST_BINS;
  float hist[nBins];
  float max_mag;

  for (int i = 0; i < octaves; ++i) {
    w = differences[i * layersDoG].width;
    h = differences[i * layersDoG].height;

    for (int j = 1; j < layersDoG - 1; ++j) {
      layer_ind = i * layersDoG + j;

      highData = differences[layer_ind + 1].pixels; 
      curData = differences[layer_ind ].pixels; 
      lowData = differences[layer_ind - 1].pixels; 

      // Iterate over all pixels in image, ignore border values
      for (int r = SIFT_IMAGE_BORDER; r < h - SIFT_IMAGE_BORDER; ++r) {
        for (int c = SIFT_IMAGE_BORDER; c < w - SIFT_IMAGE_BORDER; ++c) {
          // Pixel position and value
          pos = r * w + c;
          pixel = curData[pos];

          // Test if pixel value is an extrema:
          int isExtrema =
            (pixel >= threshold  && is_local_max(pixel, pos, w, curData, lowData, highData)) ||
            (pixel <= -threshold && is_local_min(pixel, pos, w, curData, lowData, highData));

          if (isExtrema) {
            if (keypoints_found < keypoints_required) {
              keypoints[keypoints_current].layer = j;
              keypoints[keypoints_current].octave = i;

              keypoints[keypoints_current].layer_pos.x = (float) r;
              keypoints[keypoints_current].layer_pos.y = (float) c;
  
              // EzSift does the refinement here and decides at this moment if the keypoint is useable
              int isGoodKeypoint = ethsift_refine_local_extrema(differences, octaves, layers, &keypoints[keypoints_current]);
              
              if (!isGoodKeypoint) {
                continue;
              }

              ethsift_compute_orientation_histogram(
                gradients[i * layers + j], 
                rotations[i * layers + j], 
                &(keypoints[keypoints_current]), 
                hist, &max_mag);

              float hist_threshold = max_mag * SIFT_ORI_PEAK_RATIO;

              for (int ii = 0; ii < nBins; ++ii) {
                int left = ii > 0 ? ii - 1 : nBins - 1;
                int right = ii < (nBins - 1) ? ii + 1 : 0;
                float currHist = hist[ii];
                float lhist = hist[left];
                float rhist = hist[right];
                if (currHist > lhist && currHist > rhist &&
                  currHist > hist_threshold) {
                  // Refer to here:
                  // http://stackoverflow.com/questions/717762/how-to-calculate-the-vertex-of-a-parabola-given-three-points
                  float accu_ii =
                    ii + 0.5f * (lhist - rhist) /
                    (lhist - 2.0f * currHist + rhist);

                  // Since bin index means the starting point of a
                  // bin, so the real orientation should be bin
                  // index plus 0.5. for example, angles in bin 0
                  // should have a mean value of 5 instead of 0;
                  accu_ii += 0.5f;
                  accu_ii = accu_ii < 0 ? (accu_ii + nBins)
                                        : accu_ii >= nBins
                                          ? (accu_ii - nBins)
                                          : accu_ii;
                  // The magnitude should also calculate the max
                  // number based on fitting But since we didn't
                  // actually use it in image matching, we just
                  // lazily use the histogram value.
                  keypoints[keypoints_current].magnitude = currHist;
                  keypoints[keypoints_current].orientation = accu_ii * M_TWOPI / nBins;
                }
              }       

              // Update keypoint counts
              ++keypoints_current;
              ++keypoints_found;

            } else {
              // Still test keypoint if usable
              temp.layer = j;
              temp.octave = i;

              temp.layer_pos.x = (float) r;
              temp.layer_pos.y = (float) c;

              // EzSift does the refinement here and decides at this moment if the keypoint is useable
              int isGoodKeypoint = ethsift_refine_local_extrema(differences, octaves, layers, &temp);
              
              if (!isGoodKeypoint) {
                continue;
              }

              // Update keypoints found 
              ++keypoints_found;
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