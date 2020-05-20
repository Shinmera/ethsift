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

  layer_ind = octave * nDoGLayers + layer;
  w = differences[layer_ind].width;
  h = differences[layer_ind].height;
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

    float v2 = 2.0f * cur_rc_cc; //1 ADD

    dx = 0.5f * (cur_rc_cr - cur_rc_cl); //1 MUL + 1 SUB
    dxx = cur_rc_cr + cur_rc_cl - v2; //1 ADD + 1 SUB

    dy = 0.5f * (cur_rt_cc - cur_rb_cc); //1 MUL + 1 SUB
    dyy = cur_rt_cc + cur_rb_cc - v2; //1 ADD + 1 SUB

    dxy = 0.25f * (cur_rt_cr -
    cur_rt_cl -
    cur_rb_cr +
    cur_rb_cl); // 1MUL + 2SUBs + 1ADD 17
    
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

    inc_mem(19)
    
    ds = 0.5f * (high_rc_cc - low_rc_cc); //1 MUL + 1 SUB

    dss = high_rc_cc + low_rc_cc - v2; //1 ADD + 1 SUB

    dxs = 0.25f * (high_rc_cr -
      high_rc_cl -
      low_rc_cr +
      low_rc_cl); // 1MUL + 2SUBs + 1ADD
    
    dys = 0.25f * (high_rt_cc -
      high_rb_cc -
      low_rt_cc +
      low_rb_cc); // 1MUL + 2SUBs + 1ADD 25

    float dD[3] = {-dx, -dy, -ds}; 

    inc_adds(3); // TODO: update counters
    inc_mults(3);
  
    inc_adds(6);
    inc_mults(1); 
    
    inc_adds(3);
    inc_mults(1);
      
    inc_adds(3);
    inc_mults(1);
    
    inc_adds(3);
    inc_mults(1);

    // The scale in two sides of the equation should cancel each other.
    float H[9] = {dxx, dxy, dxs, dxy, dyy, dys, dxs, dys, dss};
    
    float det;
    // SARRUS
    det =  H[0] * (H[4] * H[8] - H[5] * H[7]); // 3 MUL + 1 SUB
    det += H[1] * (H[5] * H[6] - H[3] * H[8]); // 3 MUL + 2 SUB
    det += H[2] * (H[3] * H[7] - H[4] * H[6]); // 3 MUL + 1 SUB + 1 ADD
    
    inc_adds(6);
    inc_mults(9);
    inc_mem(15);

    if (fabsf(det) < FLT_MIN)
      break;

    float Hinvert[9];

    float s = 1.0f / det; // 1 DIV

    inc_div(1);

    // CROSS PRODUCT WITH SCALING ? (was named scale adjoint)
    Hinvert[0] = (H[4] * H[8] - H[5] * H[7]);
    Hinvert[3] = (H[5] * H[6] - H[3] * H[8]);
    Hinvert[6] = (H[3] * H[7] - H[4] * H[6]);
                                                                          
    Hinvert[1] = (H[2] * H[7] - H[1] * H[8]);
    Hinvert[4] = (H[0] * H[8] - H[2] * H[6]);
    Hinvert[7] = (H[1] * H[6] - H[0] * H[7]);
                                                                        
    Hinvert[2] = (H[1] * H[5] - H[2] * H[4]);
    Hinvert[5] = (H[2] * H[3] - H[0] * H[5]);
    Hinvert[8] = (H[0] * H[4] - H[1] * H[3]);

    inc_adds(9);
    inc_mults(27);
    inc_mem(45);

    float t1 = dD[0] * s;
    float t2 = dD[1] * s;
    float t3 = dD[2] * s;



    // MAT_DOT_VEC_3X3  
    //            MUL              <- FMA             <- FMA
    xc          = Hinvert[0] * t1 + Hinvert[1] * t2 + Hinvert[2] * t3;
    xr          = Hinvert[3] * t1 + Hinvert[4] * t2 + Hinvert[5] * t3;
    xs          = Hinvert[6] * t1 + Hinvert[7] * t2 + Hinvert[8] * t3;
    reduntdant  = 1 * t1          + 1 * t2          + 1 * t3;

    inc_adds(6);
    inc_mults(9);
    inc_mem(21); 

    
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

    if (xc_i == 0 && xr_i == 0) // xs_i is never modifed, so I remove it from the checks
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

  float norm = (float)(1 << octave); // 1 POW

  // Coordinates in the normalized format (compared to the original image).
  keypoint->global_pos.y = tmp_r * norm; // 1 MUL
  keypoint->global_pos.x = tmp_c * norm; // 1 MUL
  keypoint->global_pos.scale = keypoint->layer_pos.scale * norm; // 1 MUL

  inc_mults(3);

  //22 FLOPS + 2 POWs
  return 1;
}