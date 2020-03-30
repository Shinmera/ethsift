#include "internal.h"

// Refine the location of the keypoints to be sub-pixel accurate.
int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octaves, uint32_t layers, struct ethsift_keypoint *keypoint){
  
  // Settings of EzSift:
  int SIFT_MAX_INTERP_STEPS = 5;
  int SIFT_KEYPOINT_SUBPiXEL_THR = 0.6f;

  // Fields:
  int w = 0;
  int h = 0;
  int layer_ind = 0;
  int nDoGLayers = layers - 1;

  int octave = (*keypoint).octave;
  int layer = (*keypoint).layer;
  int r = (*keypoint).layer_pos.x;
  int c = (*keypoint).layer_pos.y;
  
  int xs_i = 0, xr_i = 0, xc_i = 0;
  float tmp_r = 0.0f, tmp_c = 0.0f, tmp_layer = 0.0f;
  float xr = 0.0f, xc = 0.0f, xs = 0.0f;
  float x_hat[3] = {xc, xr, xs};
  float dx = 0.0f, dy = 0.0f, ds = 0.0f;
  float dxx = 0.0f, dyy = 0.0f, dss = 0.0f, dxs = 0.0f, dys = 0.0f,
        dxy = 0.0f;

  tmp_r = (float)r;
  tmp_c = (float)c;
  tmp_layer = (float)layer;

  // Current, low and high index in DoG pyramid
  int curData = 0;
  int lowData = 0;
  int highData = 0;

  // Interpolation (x,y,sigma) 3D space to find sub-pixel accurate
  // location of keypoints.
  for (int i = 0; i < SIFT_MAX_INTERP_STEPS; ++i) {
    c += xc_i;
    r += xr_i;

    layer_ind = octave * nDoGLayers + layer;
    w = differences[layer_ind].width;
    h = differences[layer_ind].height;

    curData = layer_ind;
    lowData = layer_ind - 1;
    highData = layer_ind + 1;

    dx = 0.5f * (differences[curData].pixels[r * w + c + 1] - differences[curData].pixels[r * w + c - 1]); 
    dy = 0.5f * (differences[curData].pixels[(r + 1) * w + c] - differences[curData].pixels[(r - 1) * w + c]); 
    ds = 0.5f * (differences[highData].pixels[r * w + c] - differences[lowData].pixels[r * w + c]);

    float dD[3] = {-dx, -dy, -ds}; 

    float v2 = 2.0f * differences[curData].pixels[r * w + c];

    dxx = differences[curData].pixels[r * w + c + 1] + differences[curData].pixels[r * w + c - 1] - v2;
    dyy = differences[curData].pixels[(r + 1) * w + c] + differences[curData].pixels[(r - 1) * w + c] - v2;
    dss = differences[highData].pixels[r * w + c] + differences[lowData].pixels[r * w + c] - v2;

    dxy = 0.25f * (differences[curData].pixels[(r + 1) * w + c + 1] 
      - differences[curData].pixels[(r + 1) * w + c - 1]
      - differences[curData].pixels[(r - 1) * w + c + 1]
      + differences[curData].pixels[(r - 1) * w + c - 1]);
    dxs = 0.25f * (differences[highData].pixels[(r) * w + c + 1] 
      - differences[highData].pixels[(r) * w + c - 1]
      - differences[lowData].pixels[(r) * w + c + 1]
      + differences[lowData].pixels[(r) * w + c - 1]);
    dys = 0.25f * (differences[highData].pixels[(r + 1) * w + c] 
      - differences[highData].pixels[(r - 1) * w + c]
      - differences[lowData].pixels[(r + 1) * w + c]
      + differences[lowData].pixels[(r - 1) * w + c]);

    // The scale in two sides of the equation should cancel each other.
    float H[3][3] = {{dxx, dxy, dxs}, {dxy, dyy, dys}, {dxs, dys, dss}};
    
    float det;
    det =  H[0][0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1]);
    det -= H[0][1] * (H[1][0] * H[2][2] - H[1][2] * H[2][0]); 
    det += H[0][2] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]); 

    float Hinvert[3][3];
    // TODO:
    //    - invert H (Scale adjoint 3x3)
    //    - Update tmp data for keypoint update
    
  }

  // TODO Update keypoint fields and return if viable keypoint
  
  return 0;
}