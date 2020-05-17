/*
    This file serves as general settings file for setting the intial values in our project.
*/


// default number of sampled intervals per octave
#define ETHSIFT_INTVLS 3

#define ETHSIFT_INVERSE_INTVLS 1.0f / ETHSIFT_INTVLS;

// default sigma for initial gaussian smoothing
#define ETHSIFT_SIGMA 1.6f

// assumed gaussian blur for input image
#define ETHSIFT_INIT_SIGMA 0.5f

// the radius of Gaussian filter kernel;
// Gaussian filter mask will be (2*radius+1)x(2*radius+1).
// People use 2 or 3 most.
#define ETHSIFT_GAUSSIAN_FILTER_RADIUS 3.0f

// default number of bins in histogram for orientation assignment
#define ETHSIFT_ORI_HIST_BINS 36

// determines gaussian sigma for orientation assignment
#define ETHSIFT_ORI_SIG_FCTR  1.5f // Can affect the orientation computation.

// determines the radius of the region used in orientation assignment
#define ETHSIFT_ORI_RADIUS (3.0f * ETHSIFT_ORI_SIG_FCTR) // Can affect the orientation computation.

// default width of descriptor histogram array
#define ETHSIFT_DESCR_WIDTH 4

#define ETHSIFT_DESCR_EXP_SCALE -2.0f / (ETHSIFT_DESCR_WIDTH * ETHSIFT_DESCR_WIDTH);

// default number of bins per histogram in descriptor array
#define ETHSIFT_DESCR_HIST_BINS 8

#define ETHSIFT_DESCR_HIST_BINS_DEGREE ((float) ETHSIFT_DESCR_HIST_BINS / M_TWOPI);

// determines the size of a single descriptor orientation histogram
#define ETHSIFT_DESCR_SCL_FCTR 3.f;

// threshold on magnitude of elements of descriptor vector
#define ETHSIFT_DESCR_MAG_THR 0.2f;

// factor used to convert floating-point descriptor to unsigned char
#define ETHSIFT_INT_DESCR_FCTR 512.f;

// Maximum amount of Keypoints we want to be able to track.
#define ETHSIFT_MAX_TRACKABLE_KEYPOINTS 1000

// default threshold on keypoint contrast |D(x)|
#define ETHSIFT_CONTR_THR 8.0f; // 8.0f;

// default threshold on keypoint ratio of principle curvatures
#define ETHSIFT_CURV_THR 10.0f;

// width of border in which to ignore keypoints
#define ETHSIFT_IMG_BORDER 5;

// orientation magnitude relative to max that results in new feature
#define ETHSIFT_ORI_PEAK_RATIO 0.8f;

// maximum steps of keypoint interpolation before failure
#define ETHSIFT_MAX_INTERP_STEPS 5;

// The keypoint refinement smaller than this threshold will be discarded.
#define ETHSIFT_KEYPOINT_SUBPiXEL_THR 0.6f;
