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