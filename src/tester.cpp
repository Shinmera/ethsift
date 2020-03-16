// File implementing a test harness of some kind.
#include "ethsift.h"

int main(){
  // Just making sure things are building right.
  ethsift_init();
  ezsift::Image<unsigned char> image1;
  image1.read_pgm("../ezsift/data/img1.pgm");
  return 0;
}
