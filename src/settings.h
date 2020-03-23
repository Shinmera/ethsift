/*
    This file serves as general settings file for setting the intial values in our project.
*/
// default sigma for initial gaussian smoothing
static float ETHSIFT_SIGMA = 1.6f;

// assumed gaussian blur for input image
static float ETHSIFT_INIT_SIGMA = 0.5f;

// the radius of Gaussian filter kernel;
// Gaussian filter mask will be (2*radius+1)x(2*radius+1).
// People use 2 or 3 most.
static float ETHSIFT_GAUSSIAN_FILTER_RADIUS = 3.0f;


// /****************************************
//  * Constant parameters
//  ***************************************/

// // default number of sampled intervals per octave
// static int SIFT_INTVLS = 3;

// // default sigma for initial gaussian smoothing
// static float SIFT_SIGMA = 1.6f;

// // the radius of Gaussian filter kernel;
// // Gaussian filter mask will be (2*radius+1)x(2*radius+1).
// // People use 2 or 3 most.
// static float SIFT_GAUSSIAN_FILTER_RADIUS = 3.0f;

// // default threshold on keypoint contrast |D(x)|
// static float SIFT_CONTR_THR = 8.0f; // 8.0f;

// // default threshold on keypoint ratio of principle curvatures
// static float SIFT_CURV_THR = 10.0f;

// // The keypoint refinement smaller than this threshold will be discarded.
// static float SIFT_KEYPOINT_SUBPiXEL_THR = 0.6f;

// // double image size before pyramid construction?
// static bool SIFT_IMG_DBL = false; // true;

// // assumed gaussian blur for input image
// static float SIFT_INIT_SIGMA = 0.5f;

// // width of border in which to ignore keypoints
// static int SIFT_IMG_BORDER = 5;

// // maximum steps of keypoint interpolation before failure
// static int SIFT_MAX_INTERP_STEPS = 5;

// // default number of bins in histogram for orientation assignment
// static int SIFT_ORI_HIST_BINS = 36;

// // determines gaussian sigma for orientation assignment
// static float SIFT_ORI_SIG_FCTR =
//     1.5f; // Can affect the orientation computation.

// // determines the radius of the region used in orientation assignment
// static float SIFT_ORI_RADIUS =
//     3 * SIFT_ORI_SIG_FCTR; // Can affect the orientation computation.

// // orientation magnitude relative to max that results in new feature
// static float SIFT_ORI_PEAK_RATIO = 0.8f;

// // maximum number of orientations for each keypoint location
// // static const float SIFT_ORI_MAX_ORI = 4;

// // determines the size of a single descriptor orientation histogram
// static float SIFT_DESCR_SCL_FCTR = 3.f;

// // threshold on magnitude of elements of descriptor vector
// static float SIFT_DESCR_MAG_THR = 0.2f;

// // factor used to convert floating-point descriptor to unsigned char
// static float SIFT_INT_DESCR_FCTR = 512.f;

// // default width of descriptor histogram array
// static int SIFT_DESCR_WIDTH = 4;

// // default number of bins per histogram in descriptor array
// static int SIFT_DESCR_HIST_BINS = 8;

// // default value of the nearest-neighbour distance ratio threshold
// // |DR_nearest|/|DR_2nd_nearest|<SIFT_MATCH_NNDR_THR is considered as a match.
// static float SIFT_MATCH_NNDR_THR = 0.65f;