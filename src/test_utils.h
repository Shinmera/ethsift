// Allocate the pixel array in the given output image according to its width and height.
struct ethsift_image allocate_image(uint32_t width, uint32_t height);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<unsigned char> &input, struct ethsift_image *output);

// Convert an ezsift image to an ethsift image. The pixels array will be replaced!
int convert_image(const ezsift::Image<float> &input, struct ethsift_image *output);

// Convert ezSift Keypoint to eth_sift keypoint
struct ethsift_keypoint convert_keypoint(ezsift::SiftKeypoint *k);

// Directly load an ethsift image
int load_image(const char *file, struct ethsift_image &image, ezsift::Image<unsigned char> *ezimage = 0);

// Compare two images for pixel precise equality
int compare_image(const ezsift::Image<unsigned char> &ez_img, struct ethsift_image &eth_img);

// Compare two images for pixel precise equality. Returns 1 if they match, 0 otherwise.
int compare_image(struct ethsift_image a, struct ethsift_image b);

// Compare two images for approximate equality. Returns 1 if they match, 0 otherwise.
// Pixels are considered to be equal if their difference is smaller than eps.
int compare_image_approx(const ezsift::Image<unsigned char> &ez_img, struct ethsift_image &eth_img);
int compare_image_approx(const ezsift::Image<float> &ez_img, struct ethsift_image &eth_img);

// Compare two images for approximate equality. Returns 1 if they match, 0 otherwise.
// Pixels are considered to be equal if their difference is smaller than eps.
int compare_image_approx(struct ethsift_image a, struct ethsift_image b, float eps);

// Compare an ezsift kernel with an ethsift kernel for correctness
int compare_kernel(std::vector<float> ez_kernel, float* eth_kernel, int eth_kernel_size);

// Compare an ezsift descriptor with an ethsift descriptor for correctness
int compare_descriptor(float* ez_descriptors, float* eth_descriptors);

// Write an eth_sift image to pgm format
int write_image(struct ethsift_image image, const char* filename);

// Create the pyramids in ezsift. A common test operation.
int build_ezsift_pyramids(ezsift::Image<unsigned char> ez_img, std::vector<ezsift::Image<float>> &ez_differences, std::vector<ezsift::Image<float>> &ez_gradients, std::vector<ezsift::Image<float>> &ez_rotations);
