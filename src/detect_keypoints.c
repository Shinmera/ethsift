#include "internal.h"

// Detect the keypoints in the image that SIFT finds interesting.
// keypoint_count
//   [in] how many keypoints we can store at most
//   [in] layers - number of layers of Gaussian Pyramid
//   [out] how many keypoints we actually found
int ethsift_detect_keypoints(struct ethsift_image differences[], struct ethsift_image gradients[], struct ethsift_image rotations[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint keypoints[], uint32_t *keypoint_count){
  
  // Settings as in EzSift
  int SIFT_IMAGE_BORDER = 5;
  float SIFT_CONTR_THR = 8.0f;
  
  float threshold = 0.8f * SIFT_CONTR_THR;
  
  // Layers of DoG
  int layersDoG = layers - 1;

  // Requested number of keypoints and actual number of keypoints
  uint32_t keypoints_required = *keypoint_count;
  uint32_t keypoints_found = 0;
  uint32_t keypoints_current = 0;

  // Loop variables
  float *curData;
  float *lowData;
  float *highData;

  uint32_t w, h;
  int layer_ind, pos;
  float pixel;

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
            (pixel >= threshold && pixel > highData[pos - w - 1] &&
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
              pixel > lowData[pos + w + 1]) || // Local min
            (pixel <= -threshold && pixel < highData[pos - w - 1] &&
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
              pixel < lowData[pos + w + 1]);

          if (isExtrema) {
            if (keypoints_found < keypoints_required) {
              keypoints[keypoints_current].layer = j;
              keypoints[keypoints_current].octave = i;

              keypoints[keypoints_current].layer_pos.x = (float) r;
              keypoints[keypoints_current].layer_pos.y = (float) c;
              keypoints[keypoints_current].layer_pos.scale = 1.0f;
  
              // EzSift does the refinement here and decides at this moment if the keypoint is useable
              int isGoodKeypoint = ethsift_refine_local_extrema(differences, octaves, layers, &keypoints[keypoints_current]);
              
              if (!isGoodKeypoint) {
                continue;
              }

              ++keypoints_current;
            }

            // Count keypoints found 
            ++keypoints_found;

            
            // And also computes the histograms here... TODO Move Histogram calculations to compute_keypoints

          }
        }
      }
    }
  }

  // Update count with actual number of keypoints found
  keypoint_count = &keypoints_found;
  
  return 0;
}