#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "settings.h"
#include "ethsift.h"
#include "flop_counters.h"
#include <immintrin.h>

extern float** g_kernel_ptrs;
extern int* g_kernel_rads;
extern int* g_kernel_sizes;
extern float *row_buf;
extern float *img_buf;


#define internal_max(a,b) (((a) > (b)) ? (a) : (b))
#define internal_min(a,b) (((a) < (b)) ? (a) : (b))

// Wrap image pixel access. Note this does not handle border conditions!
static inline float pixel(struct ethsift_image image, uint32_t x, uint32_t y){
  return image.pixels[image.width*y+x];
};

/// <summary> 
/// Refine the location of the keypoints to be sub-pixel accurate.
/// </summary>
/// <param name="imageData"> IN: Set of pixels in image. </param>
/// <param name="w"> IN: Total width of image. </param> 
/// <param name="h"> IN: Total height of image. </param> 
/// <param name="r"> IN: Row position to extract pixel from. </param> 
/// <param name="c"> IN: Column position to extract pixel from. </param> 
/// <returns> The pixel value at position (r,c). </returns>
static inline float get_pixel_f(float *imageData, int w, int h, int r, int c)
{
    int c_mod = internal_min(c, w - 1);
    int r_mod = internal_min(r, h - 1);
    c_mod = internal_max(0, c_mod);
    r_mod = internal_max(0, r_mod);
    return imageData[r_mod * w + c_mod];
}

// MATH
#define M_TWOPI 6.283185307179586f
#define M_1_2PI 0.15915494309189535f
#define M_PI_FRAC4 0.785398163397448f
#define M_THREEPI_FRAC4 2.356194490192345f
//#define M_SQRT2 1.414213562373095f
#define EPSILON_F 1.19209290E-07F

/// <summary> 
/// Calculates atan2 "in a fast" manner
/// </summary>
/// <param name="y"> IN: first input val. </param>
/// <param name="x"> IN: second input val. </param> 
/// <returns> atan2 value of y and x. </returns>
static inline float fast_atan2_f(float y, float x)
{
    //PT1
    float angle, r;
    float const c3 = 0.1821F;
    float const c1 = 0.9675F;
    float abs_y = fabsf(y) + EPSILON_F;
    inc_adds(1);
    //PT2
    if (x >= 0) {
        r = (x - abs_y) / (x + abs_y);
        angle = M_PI_FRAC4;
        inc_adds(2);
        inc_div(1);
    }
    else {
        r = (x + abs_y) / (abs_y - x);
        angle = M_THREEPI_FRAC4;
        inc_adds(2);
        inc_div(1);
    }
    angle += (c3 * r * r - c1) * r;
    inc_adds(2);
    inc_mults(3);
    //PT3
    if (y < 0) {
        inc_adds(1);
        return M_TWOPI - angle;
    } else {
        return angle;
    }
    // return (y < 0) ? M_TWOPI - angle : angle;
}


/// <summary> 
/// Calculates atan2 with intel intrinsics
/// </summary>
/// <param name="y"> IN: first input vector. </param>
/// <param name="x"> IN: second input vector. </param> 
/// <returns> __mm256 float vector which is the atan2 value of y and x. </returns>
static inline void eth_mm256_atan2_ps(__m256* y, __m256* x, __m256* dst)
{
    //PT1
    __m256 angle, r;
    __m256 c3 = _mm256_set1_ps(0.1821f);
    __m256 c1 = _mm256_set1_ps(-0.9675f);
    __m256 zeros = _mm256_set1_ps(-0.000001);
    __m256 ones = _mm256_set1_ps(1.0);
    __m256 twos = _mm256_set1_ps(2.0);
    __m256 threes = _mm256_set1_ps(3.0);
    __m256 minus_ones = _mm256_set1_ps(-1.0);
    __m256 pi_frac4 = _mm256_set1_ps(M_PI_FRAC4);
    __m256 twopi = _mm256_set1_ps(M_TWOPI);

    // vars leading up to abs_mask
    __m256 mask_geq_y = _mm256_cmp_ps(*y, zeros, _CMP_GE_OQ);    
    __m256 mask_lt_y = _mm256_cmp_ps(*y, zeros, _CMP_LE_OQ);
    __m256 zero_or_one = _mm256_and_ps(mask_geq_y, ones);
    __m256 zero_or_minusone = _mm256_and_ps(mask_lt_y, minus_ones);


    __m256 abs_mask_y = _mm256_add_ps(zero_or_one, zero_or_minusone);
    __m256 epsilon = _mm256_set1_ps(EPSILON_F);
    __m256 abs_y = _mm256_fmadd_ps(abs_mask_y, *y, epsilon);
    inc_adds(8);
        
    //PT2
    __m256 mask_geq = _mm256_cmp_ps(*x, zeros, _CMP_GE_OQ);    
    __m256 mask_lt = _mm256_cmp_ps(*x, zeros, _CMP_LE_OQ);
    zero_or_one = _mm256_and_ps(mask_lt, ones);
    zero_or_minusone = _mm256_and_ps(mask_geq, minus_ones);

    __m256 dividend_mask = _mm256_add_ps(zero_or_one, zero_or_minusone);
    __m256 divisor_mask = _mm256_mul_ps(dividend_mask, minus_ones);

    __m256 angle_zero_or_one = _mm256_and_ps(mask_geq, ones);
    __m256 angle_zero_or_three = _mm256_and_ps(mask_lt, threes);
    __m256 angle_mask = _mm256_add_ps(angle_zero_or_one, angle_zero_or_three);

    __m256 x_lt_zero = _mm256_add_ps(zero_or_one, zero_or_minusone); //TODO: write -1.f where x >= 0, 1.f where x < 0

    __m256 dividend = _mm256_fmadd_ps(dividend_mask, abs_y, *x);
    __m256 divisor = _mm256_fmadd_ps(divisor_mask, *x, abs_y);
    r = _mm256_div_ps(dividend, divisor);
    angle = _mm256_mul_ps(angle_mask, pi_frac4);
    
    inc_adds(16);
    inc_div(8);

    __m256 r_squared = _mm256_mul_ps(r, r);
    __m256 t_ang = _mm256_fmadd_ps(c3, r_squared, c1);
    angle = _mm256_fmadd_ps(t_ang, r, angle);
    
    inc_adds(16);
    inc_mults(24);
    
    //PT3
    __m256 twoangle = _mm256_mul_ps(twos, angle);
    __m256 return_mask_twopi = _mm256_and_ps(mask_lt_y, twopi);
    __m256 return_mask_twoangle = _mm256_and_ps(mask_geq_y, twoangle);
    __m256 return_mask = _mm256_add_ps(return_mask_twopi, return_mask_twoangle);
    inc_adds(8);
    *dst = _mm256_sub_ps(return_mask, angle);
}


/// <summary> 
/// Calculates the inverted square-root of x ( 1/sqrt(x) ).
/// </summary>
/// <param name="x"> IN: Value to get the inverted square-root of. </param>
/// <returns> The inverted square-root of x. </returns>
static inline float fast_resqrt_f(float x)
{
    // 32-bit version
    union {
        float x;
        int i;
    } u;

    float xhalf = (float)0.5 * x;

    // convert floating point value in RAW integer
    u.x = x;

    // gives initial guess y0
    u.i = 0x5f3759df - (u.i >> 1);

    // two Newton steps
    u.x = u.x * ((float)1.5 - xhalf * u.x * u.x);
    u.x = u.x * ((float)1.5 - xhalf * u.x * u.x);
    return u.x;
}

/// <summary> 
/// Calculate the squareroot of x.
/// </summary>
/// <param name="x"> IN: Value to get the square root of. </param>
/// <returns> The square-root of x. </returns>
static inline float fast_sqrt_f(float x)
{
    return (x < 1e-8) ? 0 : x * fast_resqrt_f(x);
}

static inline int int_max(int a, int b) {
  return a >= b ? a : b;
}

static inline int int_min(int a, int b) {
  return a <= b ? a : b;
}

static inline float float_min(float a, float b) {
  return a < b ? a : b;
}
