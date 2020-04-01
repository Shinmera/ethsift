#include "internal.h"

int mat_dot_vec_3x3(float p[], float (*m)[3], float v[]) {
  p[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
  p[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2];
  p[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2]; 

  return 1;
}

int scale_adjoint_3x3(float (*a)[3], float (*m)[3], float s) {
  a[0][0] = (s) * (m[1][1] * m[2][2] - m[1][2] * m[2][1]);
  a[1][0] = (s) * (m[1][2] * m[2][0] - m[1][0] * m[2][2]);
  a[2][0] = (s) * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
                                                                         
  a[0][1] = (s) * (m[0][2] * m[2][1] - m[0][1] * m[2][2]);
  a[1][1] = (s) * (m[0][0] * m[2][2] - m[0][2] * m[2][0]);
  a[2][1] = (s) * (m[0][1] * m[2][0] - m[0][0] * m[2][1]);
                                                                        
  a[0][2] = (s) * (m[0][1] * m[1][2] - m[0][2] * m[1][1]);
  a[1][2] = (s) * (m[0][2] * m[1][0] - m[0][0] * m[1][2]);
  a[2][2] = (s) * (m[0][0] * m[1][1] - m[0][1] * m[1][0]);

  return 1;
}


/// <summary> 
/// Refine the location of the keypoints to be sub-pixel accurate.
/// </summary>
/// <param name="differences"> IN: DOG pyramid. </param>
/// <param name="octave_count"> IN: Number of Octaves. </param> 
/// <param name="gaussian_count"> IN: Number of layers. </param> 
/// <param name="keypoints"> OUT: Array of detected keypoints. </param> 
/// <returns> 1 IF computation was successful, ELSE 0. </returns>
int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint *keypoint){
  
  // Settings
  int intvls = ETHSIFT_INTVLS;
  int max_interp_steps = ETHSIFT_MAX_INTERP_STEPS;
  float kpt_subpixel_thr = ETHSIFT_KEYPOINT_SUBPiXEL_THR;
  float contr_thr = ETHSIFT_CONTR_THR;
  float curv_thr = ETHSIFT_CURV_THR;
  float sigma = ETHSIFT_SIGMA;

  // Fields:
  int w = 0;
  int h = 0;
  int layer_ind = 0;
  int nDoGLayers = ((int) gaussian_count) - 1;

  int octave = (int) keypoint->octave;
  int layer = (int) keypoint->layer;
  int r = keypoint->layer_pos.y;
  int c = keypoint->layer_pos.x;
  
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
  float *curData = 0;
  float *lowData = 0;
  float *highData = 0;

  // Interpolation (x,y,sigma) 3D space to find sub-pixel accurate
  // location of keypoints.
  int i = 0;
  for (; i < max_interp_steps; ++i) {
    c += xc_i;
    r += xr_i;

    layer_ind = octave * nDoGLayers + layer;
    w = differences[layer_ind].width;
    h = differences[layer_ind].height;
    
    curData  = differences[layer_ind].pixels;
    lowData  = differences[layer_ind - 1].pixels;
    highData = differences[layer_ind + 1].pixels;

    dx = 0.5f * (get_pixel_f(curData, w, h, r, c + 1) - get_pixel_f(curData, w, h, r, c - 1)); 
    dy = 0.5f * (get_pixel_f(curData, w, h, r + 1, c) - get_pixel_f(curData, w, h, r - 1, c)); 
    ds = 0.5f * (get_pixel_f(highData, w, h, r, c) - get_pixel_f(lowData, w, h, r, c));

    float dD[3] = {-dx, -dy, -ds}; 

    float v2 = 2.0f * get_pixel_f(curData, w, h, r, c);

    dxx = get_pixel_f(curData, w, h, r, c + 1) + get_pixel_f(curData, w, h, r, c - 1) - v2;
    dyy = get_pixel_f(curData, w, h, r + 1, c) + get_pixel_f(curData, w, h, r - 1, c) - v2;
    dss = get_pixel_f(highData, w, h, r, c) + get_pixel_f(lowData, w, h, r, c) - v2;
      
    dxy = 0.25f * (get_pixel_f(curData, w, h, r + 1, c + 1) -
      get_pixel_f(curData, w, h, r + 1, c - 1) -
      get_pixel_f(curData, w, h, r - 1, c + 1) +
      get_pixel_f(curData, w, h, r - 1, c - 1));
    dxs = 0.25f * (get_pixel_f(highData, w, h, r, c + 1) -
      get_pixel_f(highData, w, h, r, c - 1) -
      get_pixel_f(lowData, w, h, r, c + 1) +
      get_pixel_f(lowData, w, h, r, c - 1));
    dys = 0.25f * (get_pixel_f(highData, w, h, r + 1, c) -
      get_pixel_f(highData, w, h, r - 1, c) -
      get_pixel_f(lowData, w, h, r + 1, c) +
      get_pixel_f(lowData, w, h, r - 1, c));

    // The scale in two sides of the equation should cancel each other.
    float H[3][3] = {{dxx, dxy, dxs}, {dxy, dyy, dys}, {dxs, dys, dss}};
    
    float det;
    det =  H[0][0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1]);
    det -= H[0][1] * (H[1][0] * H[2][2] - H[1][2] * H[2][0]); 
    det += H[0][2] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]); 

    if (fabsf(det) < FLT_MIN)
      break;

    float Hinvert[3][3];

    float s = 1.0f / det;
    // SCALE_ADJOINT_3X3
    scale_adjoint_3x3(Hinvert, H, s);

    // MAT_DOT_VEC_3X3
    mat_dot_vec_3x3(x_hat, Hinvert, dD);

    xs = x_hat[2];
    xr = x_hat[1];
    xc = x_hat[0];
    
    // Update tmp data for keypoint update.
    tmp_r = r + xr;
    tmp_c = c + xc;
    tmp_layer = layer + xs;

    // Make sure there is room to move for next iteration.
    xc_i = ((xc >= kpt_subpixel_thr && c < w - 2) ? 1 : 0) +
           ((xc <= -kpt_subpixel_thr && c > 1) ? -1 : 0);

    xr_i = ((xr >= kpt_subpixel_thr && r < h - 2) ? 1 : 0) +
           ((xr <= -kpt_subpixel_thr && r > 1) ? -1 : 0);

    if (xc_i == 0 && xr_i == 0 && xs_i == 0)
      break;
  }

  // We MIGHT be able to remove the following two checking conditions.
  // Condition 1
  if (i >= max_interp_steps) return 0;
  // Condition 2.
  if (fabsf(xc) >= 1.5 || fabsf(xr) >= 1.5 || fabsf(xs) >= 1.5) return 0;

  // If (r, c, layer) is out of range, return false.
  if (tmp_layer < 0 || tmp_layer > (((int) gaussian_count) - 1) || tmp_r < 0 ||
      tmp_r > h - 1 || tmp_c < 0 || tmp_c > w - 1)
    return 0;

  
  float value = get_pixel_f(curData, w, h, r, c) + 0.5f * (dx * xc + dy * xr + ds * xs);
  if (fabsf(value) < contr_thr)
    return 0;

  float trH = dxx + dyy;
  float detH = dxx * dyy - dxy * dxy;
  float response = (curv_thr + 1) * (curv_thr + 1) / (curv_thr);

  if (detH <= 0 || (trH * trH / detH) >= response)
    return 0;
  
  keypoint->layer_pos.y = tmp_r;
  keypoint->layer_pos.x = tmp_c;
  keypoint->layer_pos.scale = sigma * powf(2.0f, tmp_layer / intvls);

  float norm = powf(2.0f, (float)(octave));

  // Coordinates in the normalized format (compared to the original image).
  keypoint->global_pos.y = tmp_r * norm;
  keypoint->global_pos.x = tmp_c * norm;
  keypoint->global_pos.scale = keypoint->layer_pos.scale * norm;

  return 1;
}
