#include "internal.h"


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
  float response = ETHSIFT_RESPONSE;
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
  
  int xr_i = 0, xc_i = 0;
  float dx = 0.0f, dy = 0.0f, ds = 0.0f;
  float dxx = 0.0f, dyy = 0.0f, dss = 0.0f, dxs = 0.0f, dys = 0.0f,
        dxy = 0.0f;

  // Current, low and high index in DoG pyramid  
  float *curData = 0;
  float *lowData = 0;
  float *highData = 0;

  // Interpolation (x,y,sigma) 3D space to find sub-pixel accurate
  // location of keypoints.
  // 5 * 91 = 455 FLOPs

  int i = 0;

  layer_ind = octave * nDoGLayers + layer;
  w = differences[layer_ind].width;
  h = differences[layer_ind].height;

  float temp[4] = {c, r, layer, 0.0};
  float xD[4] = {0.0, 0.0, 0.0, 0.0};

  float Hinvert[12];
  Hinvert[3] = 1.0;
  Hinvert[7] = 1.0;
  Hinvert[11] = 1.0;
  inc_mem(3);

  for (; i < max_interp_steps; i++) {
    
    c += xc_i;
    r += xr_i;

    int c_right =   internal_min(internal_max(c + 1, 0), w - 1);
    int c_center =  internal_min(internal_max(c, 0), w - 1);
    int c_left =    internal_min(internal_max(c - 1, 0), w - 1);

    int r_top =     internal_min(internal_max(r + 1, 0), h - 1);
    int r_center =  internal_min(internal_max(r, 0), h - 1);
    int r_bottom =  internal_min(internal_max(r - 1, 0), h - 1);


    curData  = differences[layer_ind].pixels;
    float cur_rb_cl = curData[r_bottom * w + c_left];       // [r - 1, c - 1]
    float cur_rb_cc = curData[r_bottom * w + c_center];     // [r - 1, c]
    float cur_rb_cr = curData[r_bottom * w + c_right];      // [r - 1, c + 1]

    float cur_rc_cl = curData[r_center * w + c_left];       // [r, c - 1]
    float cur_rc_cc = curData[r_center * w + c_center];     // [r, c]
    float cur_rc_cr = curData[r_center * w + c_right];      // [r, c + 1]
    
    float cur_rt_cl = curData[r_top * w + c_left];          // [r + 1, c - 1]
    float cur_rt_cc = curData[r_top * w + c_center];        // [r + 1, c]
    float cur_rt_cr = curData[r_top * w + c_right];         // [r + 1, c + 1]

    inc_mem(10);

    float v2 = 2.0f * cur_rc_cc; //1 MUL

    inc_mults(1);

    dx = 0.5f * (cur_rc_cr - cur_rc_cl); //1 MUL + 1 SUB
    dxx = cur_rc_cr + cur_rc_cl - v2; //1 ADD + 1 SUB

    inc_mults(1);
    inc_adds(3);

    dy = 0.5f * (cur_rt_cc - cur_rb_cc); //1 MUL + 1 SUB
    dyy = cur_rt_cc + cur_rb_cc - v2; //1 ADD + 1 SUB

    inc_mults(1);
    inc_adds(3);

    dxy = 0.25f * (cur_rt_cr -
    cur_rt_cl -
    cur_rb_cr +
    cur_rb_cl); // 1MUL + 2SUBs + 1ADD 17

    inc_mults(1);
    inc_adds(3);
    
    highData = differences[layer_ind + 1].pixels;

    float high_rc_cl = highData[r_center * w + c_left];     // [r, c - 1]
    float high_rc_cc = highData[r_center * w + c_center];   // [r, c]
    float high_rc_cr = highData[r_center * w + c_right];    // [r, c + 1]

    lowData  = differences[layer_ind - 1].pixels;

    float low_rc_cl = lowData[r_center * w + c_left];       // [r, c - 1]
    float low_rc_cc = lowData[r_center * w + c_center];     // [r, c]
    float low_rc_cr = lowData[r_center * w + c_right];      // [r, c + 1]

    float high_rt_cc = highData[r_top * w + c_center];      // [r + 1, c]
    float high_rb_cc = highData[r_bottom * w + c_center];   // [r - 1, c]

    float low_rt_cc = lowData[r_top * w + c_center];        // [r + 1, c]
    float low_rb_cc = lowData[r_bottom * w + c_center];     // [r - 1, c]

    inc_mem(12)
    
    ds = 0.5f * (high_rc_cc - low_rc_cc); //1 MUL + 1 SUB

    inc_mults(1);
    inc_adds(1);

    dss = high_rc_cc + low_rc_cc - v2; //1 ADD + 1 SUB

    inc_adds(2);

    dxs = 0.25f * (high_rc_cr -
      high_rc_cl -
      low_rc_cr +
      low_rc_cl); // 1MUL + 2SUBs + 1ADD
    
    inc_mults(1);
    inc_adds(3);
    
    dys = 0.25f * (high_rt_cc -
      high_rb_cc -
      low_rt_cc +
      low_rb_cc); // 1MUL + 2SUBs + 1ADD 
    
    inc_mults(1);
    inc_adds(3);

    // The scale in two sides of the equation should cancel each other.
    float H[9] = {dxx, dxy, dxs, dxy, dyy, dys, dxs, dys, dss};
    
    float det;
    // SARRUS

    Hinvert[0] = (H[4] * H[8] - H[5] * H[7]);
    Hinvert[1] = (H[5] * H[6] - H[3] * H[8]);
    Hinvert[2] = (H[3] * H[7] - H[4] * H[6]);

    inc_mults(6);
    inc_adds(3);
    inc_mem(15);

    det =  H[0] * Hinvert[0]; // 3 MUL + 1 SUB
    det += H[1] * Hinvert[1]; // 3 MUL + 2 SUB
    det += H[2] * Hinvert[2]; // 3 MUL + 1 SUB + 1 ADD
    
    inc_adds(2);
    inc_mults(3);
    inc_mem(6);
    
    if (fabsf(det) < FLT_MIN)
      break;

    Hinvert[4] = (H[2] * H[7] - H[1] * H[8]);
    Hinvert[5] = (H[0] * H[8] - H[2] * H[6]);
    Hinvert[6] = (H[1] * H[6] - H[0] * H[7]);
                                                                        
    Hinvert[8] = (H[1] * H[5] - H[2] * H[4]);
    Hinvert[9] = (H[2] * H[3] - H[0] * H[5]);
    Hinvert[10] = (H[0] * H[4] - H[1] * H[3]);

    inc_adds(8);
    inc_mults(12);
    inc_mem(27);

    float s = -1.0f / det; // 1 DIV
    inc_div(1);
    
    float t1 = dx * s;
    float t2 = dy * s;
    float t3 = ds * s;

    inc_mults(3);

    // MAT_DOT_VEC_3X3 
    __m128 xc_xr_xs, col1, col2, vec_t1, vec_t2, vec_t3, temp_add;

    vec_t1 = _mm_set1_ps(t1);
    vec_t2 = _mm_set1_ps(t2);
    vec_t3 = _mm_set1_ps(t3);

    xc_xr_xs = _mm_load_ps(Hinvert);
    col1 = _mm_load_ps(Hinvert+4);
    col2 = _mm_load_ps(Hinvert+8);

    inc_mem(3);

    xc_xr_xs = _mm_mul_ps(xc_xr_xs, vec_t1);
    xc_xr_xs = _mm_fmadd_ps(col1, vec_t2, xc_xr_xs);
    xc_xr_xs = _mm_fmadd_ps(col2, vec_t3, xc_xr_xs);

    inc_adds(8);
    inc_mults(12);

    _mm_store_ps(xD, xc_xr_xs);

    inc_mem(1);

    temp_add = _mm_set_ps(0.0, layer, r, c);

    temp_add = _mm_add_ps(xc_xr_xs, temp_add);

    inc_adds(4);

    _mm_store_ps(temp, temp_add);

    inc_mem(1);

    // Make sure there is room to move for next iteration.
    xc_i = ((xD[0] >= kpt_subpixel_thr && c < w - 2) ? 1 : 0) +
           ((xD[0] <= -kpt_subpixel_thr && c > 1) ? -1 : 0);
    
    inc_mem(2);

    xr_i = ((xD[1] >= kpt_subpixel_thr && r < h - 2) ? 1 : 0) +
           ((xD[1] <= -kpt_subpixel_thr && r > 1) ? -1 : 0);
    
    inc_mem(2);

    if (xc_i == 0 && xr_i == 0) // xs_i is never modifed, so I remove it from the checks
      break;
    
  }

  // We MIGHT be able to remove the following two checking conditions.
  // Condition 1
  if (i >= max_interp_steps) return 0;
  // Condition 2.
  if (fabsf(xD[0]) >= 1.5 || fabsf(xD[1]) >= 1.5 || fabsf(xD[2]) >= 1.5) return 0; 

  inc_mem(3);

  // If (r, c, layer) is out of range, return false.
  if (temp[2] < 0 || temp[2] > (((int) gaussian_count) - 1) || temp[1] < 0 ||
      temp[1] > h - 1 || temp[0] < 0 || temp[0] > w - 1)
    return 0;
  
  inc_mem(6);

  int c_center =  internal_min(internal_max(c, 0), w - 1); 
  int r_center =  internal_min(internal_max(r, 0), h - 1);
  float cur_rc_cc = curData[r_center * w + c_center];     // [r, c]

  inc_mem(1);

  float value = cur_rc_cc + 0.5f * (dx * xD[0] + dy * xD[1] + ds * xD[2]); // 4MUL + 3 ADD 
  
  inc_adds(3);
  inc_mults(4);
  inc_mem(3);

  if (fabsf(value) < contr_thr) // 1 MASK 
    return 0;

  float trH = dxx + dyy; // 1 ADD
  float detH = dxx * dyy - dxy * dxy; // 2 MUL
   // 2 ADDs + 1 MUL + 1 DIV

  inc_adds(3);
  inc_mults(3);
  inc_div(1);

  if (detH > 0) {
    inc_div(1);
    inc_mults(1);
  }
  
  if (detH <= 0 || (trH * trH / detH) >= response) // 1 MUL + 1 DIV
    return 0;
  
  keypoint->layer_pos.y = temp[1];
  keypoint->layer_pos.x = temp[0];
  keypoint->layer_pos.scale = sigma * powf(2.0f, temp[2] * inverse_intvls); // 1 ADD + 1 DIV + 1 POW

  inc_adds(1);
  inc_div(1);

  inc_mem(3);

  float norm = (float)(1 << octave); // 1 POW

  // Coordinates in the normalized format (compared to the original image).
  keypoint->global_pos.y = temp[1] * norm; // 1 MUL
  keypoint->global_pos.x = temp[0] * norm; // 1 MUL
  keypoint->global_pos.scale = keypoint->layer_pos.scale * norm; // 1 MUL

  inc_mults(3);
  inc_mem(2);

  //22 FLOPS + 2 POWs
  return 1;
}