#ifndef PERF_COUNTERS_H
#define PERF_COUNTERS_H

#ifndef ETHSIFT_DATA
#error "ETHSIFT_DATA must be defined."
#endif

#ifndef ETHSIFT_LOGS
#error "ETHSIFT_LOGS must be defined."
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include "internal.h"

#define OCTAVE_COUNT 6
#define GAUSSIAN_COUNT 6
#define DOG_COUNT 5
#define GRAD_ROT_LAYERS 3
#define LENA_KEYPOINTS 136

// Test struct
typedef struct {
  const char *title;
  int enabled;
  int (*func)();
} test;

// Arrays used for logs
extern size_t add_counts[];
extern size_t mult_counts[];
extern size_t mem_counts[];

// Array containing all tests that get executed
extern int test_count;
test *tests;

int init_tests();

int run_test(int id, test test) {
  #ifdef IS_COUNTING
    fprintf(stderr, "Running %-36s \033[0;90m...\033[0;0m ", test.title);
    // Reset counters to zero
    reset_counters();

    int ret = test.func();

    fprintf(stderr, " %8ld adds/subs, %8ld mults, %8ld read/writes\n", add_count, mult_count, mem_count);
    add_counts[id] = add_count;
    mult_counts[id] = mult_count;
    mem_counts[id] = mem_count;
    return ret;
  #else
    fprintf(stderr, "IS_COUNTING is not defined");
    return 0;
  #endif
}

int write_log(int n_tests) {
  // TODO write log to csv file
  return 0;
}

// Return an absolute path to a file within the project root's data/ directory.
static char* data_file(char* file) {
  const char* data = ETHSIFT_DATA;
  char* path = (char*)calloc(sizeof(char), strlen(data) + strlen(file) + 1);
  path = strcat(path, data);
  path = strcat(path, "/");
  path = strcat(path, file);
  return path;
}

// Simple method to read pgm image and return an ethsift_image
int read_pgm(const char *filename, struct ethsift_image *output)
{
  unsigned char *_data;
  FILE *in_file;
  char ch, type;
  int i;
  int w, h;

  in_file = fopen(filename, "rb");
  if (!in_file) {
      fprintf(stderr, "ERROR(0): Fail to open file %s\n", filename);
      return -1;
  }
  // Determine pgm image type (only type three can be used)
  ch = getc(in_file);
  if (ch != 'P') {
      printf("ERROR(1): Not valid pgm/ppm file type\n");
      return -1;
  }
  ch = getc(in_file);
  // Convert the one digit integer currently represented as a character to
  // an integer(48 == '0')
  type = ch - 48;
  if (type != 5) {
      printf("ERROR(2): this file type (P%d) is not supported!\n", type);
      return -1;
  }
  while (getc(in_file) != '\n')
      ;                          // Skip to end of line
  while (getc(in_file) == '#') { // Skip comment lines
      while (getc(in_file) != '\n')
          ;
  }
  fseek(in_file, -1, SEEK_CUR); // Backup one character

  fscanf(in_file, "%d", &w);
  fscanf(in_file, "%d", &h);
  fscanf(in_file, "%d", &i);
  while (getc(in_file) != '\n')
      ;
  _data = (unsigned char *)malloc((w) * (h) * sizeof(unsigned char));
  fread(_data, sizeof(unsigned char), (w) * (h), in_file);

  // Save data in ethsift_image struct
  output->width = w;
  output->height = h;
  output->pixels = (float *)calloc(sizeof(float), w * h);
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      size_t idx = i * w + j;
      output->pixels[idx] = (float) _data[idx];
    }
  }
  return 1;
}

#endif // PERF_COUNTERS_H
