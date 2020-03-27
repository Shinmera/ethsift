#include "ethsift.h"
#include "ezsift.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>

struct test{
  const char *title;
  int (*func)();
};

extern struct test tests[1024];
extern int test_count;

static int register_test(const char *title, int (*func)()){
  tests[test_count].title = title;
  tests[test_count].func = func;
  return ++test_count;
}

// Macro to define new test cases.
// Note that the test title must be a valid C token, so it may only contain
// alphanumerics or underscores.
#define define_test(TITLE,BODY) static int __test_ ## TITLE = register_test(# TITLE, []()BODY);

/// Helper functions to be used in tests.
int convert_image(const ezsift::Image<unsigned char> &input, struct ethsift_image *output);
int load_image(const char *file, struct ethsift_image &image);
int compare_image(struct ethsift_image a, struct ethsift_image b);
