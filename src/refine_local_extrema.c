#include "internal.h"

// 3 * (2 ADDs + 3 MULs) = 15 FLOPs
int mat_dot_vec_3x3(float p[], float (*m)[3], float v[]) {
  p[0] = m[0][0] * v[0] + m[0][1] * v[1] + m[0][2] * v[2];
  p[1] = m[1][0] * v[0] + m[1][1] * v[1] + m[1][2] * v[2];
  p[2] = m[2][0] * v[0] + m[2][1] * v[1] + m[2][2] * v[2];

  inc_adds(6);
  inc_mults(9);
  inc_mem(21); 

  return 1;
}
// 9 * (3MULs + 1 SUB) = 36 FLOPs
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

  inc_adds(9);
  inc_mults(27);
  inc_mem(45);

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
/// <remarks> 477 + 2POWs FLOPs </remarks>
int ethsift_refine_local_extrema(struct ethsift_image differences[], uint32_t octave_count, uint32_t gaussian_count, struct ethsift_keypoint *keypoint){
  
  // Settings
  int intvls = ETHSIFT_INTVLS;
  float inverse_intvls = ETHSIFT_INVERSE_INTVLS;
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
  // 5 * 91 = 455 FLOPs
  int i = 0;
  for (; i < max_interp_steps; i+=2) {
    c += xc_i;
    r += xr_i;

    layer_ind = octave * nDoGLayers + layer;
    w = differences[layer_ind].width;
    h = differences[layer_ind].height;

    int c_right =   internal_min(internal_max(c + 1, 0), w - 1);
    int c_left =    internal_min(internal_max(c - 1, 0), w - 1);
    int c_center =  internal_min(internal_max(c, 0), w - 1);

    int r_top =     internal_min(internal_max(r + 1, 0), h - 1);
    int r_bottom =  internal_min(internal_max(r - 1, 0), h - 1);
    int r_center =  internal_min(internal_max(r, 0), h - 1);
    
    curData  = differences[layer_ind].pixels;
    lowData  = differences[layer_ind - 1].pixels;
    highData = differences[layer_ind + 1].pixels;

    inc_mem(3);

    float cur_rc_cc = curData[r_center * w + c_center];     // [r, c]
    float cur_rc_cr = curData[r_center * w + c_right];      // [r, c + 1]
    float cur_rc_cl = curData[r_center * w + c_left];       // [r, c - 1]
    float cur_rt_cc = curData[r_top * w + c_center];        // [r + 1, c]
    float cur_rb_cc = curData[r_bottom * w + c_center];     // [r - 1, c]
    float cur_rt_cr = curData[r_top * w + c_right];         // [r + 1, c + 1]
    float cur_rb_cl = curData[r_bottom * w + c_left];       // [r - 1, c - 1]
    float cur_rt_cl = curData[r_top * w + c_left];          // [r + 1, c - 1]
    float cur_rb_cr = curData[r_bottom * w + c_right];      // [r - 1, c + 1]
    
    float high_rc_cc = highData[r_center * w + c_center];   // [r, c]
    float high_rc_cr = highData[r_center * w + c_right];    // [r, c + 1]
    float high_rc_cl = highData[r_center * w + c_left];     // [r, c - 1]
    float high_rt_cc = highData[r_top * w + c_center];      // [r + 1, c]
    float high_rb_cc = highData[r_bottom * w + c_center];   // [r - 1, c]

    float low_rc_cc = lowData[r_center * w + c_center];     // [r, c]
    float low_rc_cr = lowData[r_center * w + c_right];      // [r, c + 1]
    float low_rc_cl = lowData[r_center * w + c_left];       // [r, c - 1]
    float low_rt_cc = lowData[r_top * w + c_center];        // [r + 1, c]
    float low_rb_cc = lowData[r_bottom * w + c_center];     // [r - 1, c]

    inc_mem(19)

    dx = 0.5f * (cur_rc_cr - cur_rc_cl); //1 MUL + 1 SUB
    dy = 0.5f * (cur_rt_cc - cur_rb_cc); //1 MUL + 1 SUB
    ds = 0.5f * (high_rc_cc - low_rc_cc); //1 MUL + 1 SUB
    
    inc_adds(3);
    inc_mults(3);

    float dD[3] = {-dx, -dy, -ds}; 

    float v2 = 2.0f * cur_rc_cc; //1 ADD

    dxx = cur_rc_cr + cur_rc_cl - v2; //1 ADD + 1 SUB
    dyy = cur_rt_cc + cur_rb_cc - v2; //1 ADD + 1 SUB
    dss = high_rc_cc + low_rc_cc - v2; //1 ADD + 1 SUB

    inc_adds(6);
    inc_mults(1);

    dxy = 0.25f * (cur_rt_cr -
      cur_rt_cl -
      cur_rb_cr +
      cur_rb_cl); // 1MUL + 2SUBs + 1ADD 17
    
    inc_adds(3);
    inc_mults(1);

    dxs = 0.25f * (high_rc_cr -
      high_rc_cl -
      low_rc_cr +
      low_rc_cl); // 1MUL + 2SUBs + 1ADD 
      
    inc_adds(3);
    inc_mults(1);

    dys = 0.25f * (high_rt_cc -
      high_rb_cc -
      low_rt_cc +
      low_rb_cc); // 1MUL + 2SUBs + 1ADD 25
    
    inc_adds(3);
    inc_mults(1);

    // The scale in two sides of the equation should cancel each other.
    float H[3][3] = {{dxx, dxy, dxs}, {dxy, dyy, dys}, {dxs, dys, dss}};
    
    float det;
    det =  H[0][0] * (H[1][1] * H[2][2] - H[1][2] * H[2][1]); // 3 MUL + 1 SUB
    det -= H[0][1] * (H[1][0] * H[2][2] - H[1][2] * H[2][0]); // 3 MUL + 2 SUB
    det += H[0][2] * (H[1][0] * H[2][1] - H[1][1] * H[2][0]); // 3 MUL + 1 SUB + 1 ADD
    
    inc_adds(6);
    inc_mults(9);
    inc_mem(15);

    if (fabsf(det) < FLT_MIN)
      break;

    float Hinvert[3][3];

    float s = 1.0f / det; // 1 DIV

    inc_div(1);

    // SCALE_ADJOINT_3X3
    scale_adjoint_3x3(Hinvert, H, s); // 36 FLOPs

    // MAT_DOT_VEC_3X3
    mat_dot_vec_3x3(x_hat, Hinvert, dD); // 15 FLOPs

    xs = x_hat[2];
    xr = x_hat[1];
    xc = x_hat[0];

    inc_mem(3);
    
    // Update tmp data for keypoint update.
    tmp_r = r + xr;
    tmp_c = c + xc;
    tmp_layer = layer + xs;

    inc_adds(3);

    // Make sure there is room to move for next iteration.
    xc_i = ((xc >= kpt_subpixel_thr && c < w - 2) ? 1 : 0) +
           ((xc <= -kpt_subpixel_thr && c > 1) ? -1 : 0);

    xr_i = ((xr >= kpt_subpixel_thr && r < h - 2) ? 1 : 0) +
           ((xr <= -kpt_subpixel_thr && r > 1) ? -1 : 0);

    if (xc_i == 0 && xr_i == 0)
      break;
    
    c += xc_i;
    r += xr_i;

    layer_ind = octave * nDoGLayers + layer;
    w = differences[layer_ind].width;
    h = differences[layer_ind].height;

    c_right =   internal_min(internal_max(c + 1, 0), w - 1);
    c_left =    internal_min(internal_max(c - 1, 0), w - 1);
    c_center =  internal_min(internal_max(c, 0), w - 1);

    r_top =     internal_min(internal_max(r + 1, 0), h - 1);
    r_bottom =  internal_min(internal_max(r - 1, 0), h - 1);
    r_center =  internal_min(internal_max(r, 0), h - 1);
    
    curData  = differences[layer_ind].pixels;
    lowData  = differences[layer_ind - 1].pixels;
    highData = differences[layer_ind + 1].pixels;

    inc_mem(3);

    cur_rc_cc = curData[r_center * w + c_center];     // [r, c]
    cur_rc_cr = curData[r_center * w + c_right];      // [r, c + 1]
    cur_rc_cl = curData[r_center * w + c_left];       // [r, c - 1]
    cur_rt_cc = curData[r_top * w + c_center];        // [r + 1, c]
    cur_rb_cc = curData[r_bottom * w + c_center];     // [r - 1, c]
    cur_rt_cr = curData[r_top * w + c_right];         // [r + 1, c + 1]
    cur_rb_cl = curData[r_bottom * w + c_left];       // [r - 1, c - 1]
    cur_rt_cl = curData[r_top * w + c_left];          // [r + 1, c - 1]
    cur_rb_cr = curData[r_bottom * w + c_right];      // [r - 1, c + 1]
    
    high_rc_cc = highData[r_center * w + c_center];   // [r, c]
    high_rc_cr = highData[r_center * w + c_right];    // [r, c + 1]
    high_rc_cl = highData[r_center * w + c_left];     // [r, c - 1]
    high_rt_cc = highData[r_top * w + c_center];      // [r + 1, c]
    high_rb_cc = highData[r_bottom * w + c_center];   // [r - 1, c]

    low_rc_cc = lowData[r_center * w + c_center];     // [r, c]
    low_rc_cr = lowData[r_center * w + c_right];      // [r, c + 1]
    low_rc_cl = lowData[r_center * w + c_left];       // [r, c - 1]
    low_rt_cc = lowData[r_top * w + c_center];        // [r + 1, c]
    low_rb_cc = lowData[r_bottom * w + c_center];     // [r - 1, c]

    inc_mem(19);

    dx = 0.5f * (cur_rc_cr - cur_rc_cl); //1 MUL + 1 SUB
    dy = 0.5f * (cur_rt_cc - cur_rb_cc); //1 MUL + 1 SUB
    ds = 0.5f * (high_rc_cc - low_rc_cc); //1 MUL + 1 SUB
    
    inc_adds(3);
    inc_mults(3);

    float temp[3] = {-dx, -dy, -ds}; 

    v2 = 2.0f * cur_rc_cc; //1 ADD

    dxx = cur_rc_cr + cur_rc_cl - v2; //1 ADD + 1 SUB
    dyy = cur_rt_cc + cur_rb_cc - v2; //1 ADD + 1 SUB
    dss = high_rc_cc + low_rc_cc - v2; //1 ADD + 1 SUB

    inc_adds(6);
    inc_mults(1);

    dxy = 0.25f * (cur_rt_cr -
      cur_rt_cl -
      cur_rb_cr +
      cur_rb_cl); // 1MUL + 2SUBs + 1ADD 17
    
    inc_adds(3);
    inc_mults(1);

    dxs = 0.25f * (high_rc_cr -
      high_rc_cl -
      low_rc_cr +
      low_rc_cl); // 1MUL + 2SUBs + 1ADD 
      
    inc_adds(3);
    inc_mults(1);

    dys = 0.25f * (high_rt_cc -
      high_rb_cc -
      low_rt_cc +
      low_rb_cc); // 1MUL + 2SUBs + 1ADD 25
    
    inc_adds(3);
    inc_mults(1);

    // The scale in two sides of the equation should cancel each other.
    float tempH[3][3] = {{dxx, dxy, dxs}, {dxy, dyy, dys}, {dxs, dys, dss}};
    
    det;
    det =  tempH[0][0] * (tempH[1][1] * tempH[2][2] - tempH[1][2] * tempH[2][1]); // 3 MUL + 1 SUB
    det -= tempH[0][1] * (tempH[1][0] * tempH[2][2] - tempH[1][2] * tempH[2][0]); // 3 MUL + 2 SUB
    det += tempH[0][2] * (tempH[1][0] * tempH[2][1] - tempH[1][1] * tempH[2][0]); // 3 MUL + 1 SUB + 1 ADD
    
    inc_adds(6);
    inc_mults(9);
    inc_mem(15);

    if (fabsf(det) < FLT_MIN)
      break;

    Hinvert[3][3];

    s = 1.0f / det; // 1 DIV

    inc_div(1);

    // SCALE_ADJOINT_3X3
    scale_adjoint_3x3(Hinvert, tempH, s); // 36 FLOPs

    // MAT_DOT_VEC_3X3
    mat_dot_vec_3x3(x_hat, Hinvert, temp); // 15 FLOPs

    xs = x_hat[2];
    xr = x_hat[1];
    xc = x_hat[0];

    inc_mem(3);
    
    // Update tmp data for keypoint update.
    tmp_r = r + xr;
    tmp_c = c + xc;
    tmp_layer = layer + xs;

    inc_adds(3);

    // Make sure there is room to move for next iteration.
    xc_i = ((xc >= kpt_subpixel_thr && c < w - 2) ? 1 : 0) +
           ((xc <= -kpt_subpixel_thr && c > 1) ? -1 : 0);

    xr_i = ((xr >= kpt_subpixel_thr && r < h - 2) ? 1 : 0) +
           ((xr <= -kpt_subpixel_thr && r > 1) ? -1 : 0);

    if (xc_i == 0 && xr_i == 0)
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

  int c_center =  internal_min(internal_max(c, 0), w - 1); 
  int r_center =  internal_min(internal_max(r, 0), h - 1);
  float cur_rc_cc = curData[r_center * w + c_center];     // [r, c]

  inc_mem(1);

  float value = cur_rc_cc + 0.5f * (dx * xc + dy * xr + ds * xs); // 4MUL + 3 ADD 
  
  inc_adds(3);
  inc_mults(4);

  if (fabsf(value) < contr_thr) // 1 MASK 
    return 0;

  float trH = dxx + dyy; // 1 ADD
  float detH = dxx * dyy - dxy * dxy; // 2 MUL
  float response = (curv_thr + 1) * (curv_thr + 1) / (curv_thr); // 2 ADDs + 1 MUL + 1 DIV

  inc_adds(3);
  inc_mults(3);
  inc_div(1);

  if (detH > 0) {
    inc_div(1);
    inc_mults(1);
  }
  
  if (detH <= 0 || (trH * trH / detH) >= response) // 1 MUL + 1 DIV
    return 0;
  
  keypoint->layer_pos.y = tmp_r;
  keypoint->layer_pos.x = tmp_c;
  keypoint->layer_pos.scale = sigma * powf(2.0f, tmp_layer * inverse_intvls); // 1 ADD + 1 DIV + 1 POW

  inc_adds(1);
  inc_div(1);

  float norm = powf(2.0f, (float)(octave)); // 1 POW

  // Coordinates in the normalized format (compared to the original image).
  keypoint->global_pos.y = tmp_r * norm; // 1 MUL
  keypoint->global_pos.x = tmp_c * norm; // 1 MUL
  keypoint->global_pos.scale = keypoint->layer_pos.scale * norm; // 1 MUL

  inc_mults(3);

  //22 FLOPS + 2 POWs
  return 1;
}