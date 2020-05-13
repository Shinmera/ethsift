#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>
#include "settings.h"
#include "ethsift.h"
#include "flop_counters.h"

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
    float val;
    if (c >= 0 && c < w && r >= 0 && r < h) {
        val = imageData[r * w + c];
    }
    else if (c < 0) {
        val = imageData[r * w];
    }
    else if (c >= w) {
        val = imageData[r * w + w - 1];
    }
    else if (r < 0) {
        val = imageData[c];
    }
    else if (r >= h) {
        val = imageData[(h - 1) * w + c];
    }
    else {
        val = 0.0f;
    }
    return val;
}

// MATH
#define M_TWOPI 6.283185307179586f
#define M_PI_FRAC4 0.785398163397448f
#define M_THREEPI_FRAC4 2.356194490192345f
//#define M_SQRT2 1.414213562373095f
#define EPSILON_F 1.19209290E-07F

/// <summary> 
/// Refine the location of the keypoints to be sub-pixel accurate.
/// </summary>
/// <param name="imageData"> IN: Set of pixels in image. </param>
/// <param name="w"> IN: Total width of image. </param> 
/// <param name="h"> IN: Total height of image. </param> 
/// <param name="r"> IN: Row position to extract pixel from. </param> 
/// <param name="c"> IN: Column position to extract pixel from. </param> 
/// <returns> The pixel value at position (r,c). </returns>
static inline float fast_atan2_f(float y, float x)
{
    float angle, r;
    float const c3 = 0.1821F;
    float const c1 = 0.9675F;
    float abs_y = fabsf(y) + EPSILON_F;

    if (x >= 0) {
        r = (x - abs_y) / (x + abs_y);
        angle = M_PI_FRAC4;
    }
    else {
        r = (x + abs_y) / (abs_y - x);
        angle = M_THREEPI_FRAC4;
    }
    angle += (c3 * r * r - c1) * r;

    return (y < 0) ? M_TWOPI - angle : angle;
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
