#include "tester.h"

struct test{
  const char *title;
  const char *reason;
  int has_measurement_comp;
  bool enabled = false;
  int (*func)();
};

#if USE_RDTSC
  myInt64 start;
#else
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
#endif

std::vector<size_t> durations;
std::vector<LogTuple> test_logs;
bool measurement_pending = false;
int test_count = 0;
struct test tests[1024] = {0};
std::string* g_testImgName;
int NR_RUNS = 30;

int register_test(const char *title, int has_measurement_comp, int (*func)()){
  tests[test_count].title = title;
  tests[test_count].reason = 0;
  tests[test_count].has_measurement_comp = has_measurement_comp;
  tests[test_count].enabled = true;
  tests[test_count].func = func;
  return test_count++;
}

static int _debugger_present = -1;
static void _sigtrap_handler(int signum){
  _debugger_present = 0;
  signal(SIGTRAP, SIG_DFL);
}

void debug_break(void){
  if (-1 == _debugger_present) {
    _debugger_present = 1;
    signal(SIGTRAP, _sigtrap_handler);
    raise(SIGTRAP);
  }
}

int register_failure(int test, const char *reason){
  if(test_count <= test) return 0;
  tests[test].reason = reason;
  debug_break();
  return 1;
}

void abort(const char *message){
  fprintf(stderr, "\033[1;31m[ERROR]\033[0;0m %s\n", message);
  exit(1);
}

template<typename T, typename Func>
T reduce(std::vector<T> &arg, Func func){
  switch(arg.size()){
  case 0: return 0.0;
  case 1: return arg[0];
  default: {
    T result = arg[0];
    for(size_t i=1; i<arg.size(); ++i)
      result = func(result, arg[1]);
    return result;
  }
  }
}

template<typename U, typename V, typename Func>
void map(std::vector<U> &in, std::vector<V> &out, Func func){
  for(size_t i=0; i<in.size() && i<out.size(); ++i)
    out[i] = func(in[i]);
}

void write_logfile(char *filename){
    std::ofstream myfile;
    myfile.open(filename);
    myfile << "MethodName, Median, Std" << std::endl;
    for (auto t : test_logs) {
        myfile << std::get<0>(t) << ", " << std::get<1>(t)
            << ", " << std::get<2>(t) << std::endl;
    }
    myfile.close();
}

//// Empty test to measure base noise. Seems we incur 14-20 (median 15)
//// cycles per measurement no matter what.
// define_test(foo, 0, {with_repeating()})

int run_test(struct test test){
  fprintf(stderr, "Running %-36s \033[0;90m...\033[0;0m ", test.title);
  durations.clear();

  // Start measurement without the pending check to allow user override
  #if USE_RDTSC
    start = start_tsc();
  #else
    start = std::chrono::high_resolution_clock::now();
  #endif
  
  // Run the check.
  int ret = test.func();

  // If we had no other measurement so far, force our start by setting the pending now.
  if(durations.size() == 0) measurement_pending = true;
  // End any possible measurement that might still be going on now.
  // We have to do this here in order to allow correct measurement even in the face
  // of early returns from within measurement blocks. We do not have an unwind-protect
  // operator in C after all.
  end_measurement();

  // Compute statistics for ethsift
  size_t cumulative = reduce(durations, [](auto a,auto b){return a+b;});
  std::sort(durations.begin(), durations.end());
  size_t median = durations[durations.size()/2];
  std::vector<size_t> deviations(durations.size());
  map(durations, deviations, [&](auto d){return std::abs((int64_t)d - (int64_t)median);});
  std::sort(deviations.begin(), deviations.end());
  double mad = K_FACTOR * deviations[deviations.size()/2];

  if (test.has_measurement_comp) {
    LogTuple t = { test.title, median, mad };
    test_logs.push_back(t);
  }
  
  // Show
  #if USE_RDTSC
    fprintf(stderr, " %20li cycles ±%10.2f", median, mad);
  #else
    fprintf(stderr, " %10liµs ±%10.2f", median, mad);
  #endif

  fprintf(stderr, (ret==0)?" \033[1;31m[FAIL]":" \033[0;32m[OK  ]");
  fprintf(stderr, "\033[0;0m\n");
  return ret;
}

int run_tests(struct test *tests, uint32_t count){
  int failed[count];
  int failures = 0;
  int passes = 0;

  fprintf(stderr, "\033[1;33m --> \033[0;0mRunning %i tests\n", count);
  for(uint32_t i=0; i<count; ++i){
    if(!tests[i].enabled) continue;
    if(run_test(tests[i])){
        passes++;
      }else{
        failed[failures] = i;
      failures++;
    }
  }

  fprintf(stderr, "\nPassed: %3i", passes);
  fprintf(stderr, "\nFailed: %3i\n", failures);
  if(failures){
    fprintf(stderr, "\033[1;33m --> \033[0;0mThe following tests failed:\n");
    for(int i=0; i<failures; ++i){
      struct test test = tests[failed[i]];
      fprintf(stderr, "%s: %s\n", test.title, test.reason);
    }
  }
  return (failures == 0);
}

int main(int argc, char *argv[]){
  // Parse arguments
  g_testImgName = new std::string((1 < argc)? argv[1]
                                  :getenv("IMAGE")? getenv("IMAGE")
                                  :"lena.pgm");
  NR_RUNS = getenv("RUNS")? atoi(getenv("RUNS"))
    : 30;
  if(2 < argc){
    // Disable all tests
    for(int i=0; i<test_count; ++i)
      tests[i].enabled = false;
    // Only enable the ones we specify
    for(int i=2; i<argc; ++i){
      for(int j=0; j<test_count; ++j){
        if(strcmp(argv[i], tests[j].title) == 0)
          tests[j].enabled = true;
      }
    }
  }
  if(getenv("TESTS")){
    // Disable all tests
    for(int i=0; i<test_count; ++i)
      tests[i].enabled = false;
    char *name;
    name = strtok(getenv("TESTS"), " ");
    while(name){
      for(int j=0; j<test_count; ++j){
        if(strcmp(name, tests[j].title) == 0)
          tests[j].enabled = true;
      }
      name = strtok(0, " ");
    }
  }
  // Init
  if(!ethsift_init())
    abort("Failed to initialise ETHSIFT");
  srand(time(NULL));
  // Run the tests
  fprintf(stderr, "This is ETHSIFT %s\n", ethsift_version());
  fprintf(stderr, "Will use %i runs on %s for measurement.\n", NR_RUNS, g_testImgName->c_str());
  int result = run_tests(tests, test_count);
  // Write logs
  char filename[200] = ETHSIFT_LOGS;
  strcat(filename, "/");
  strcat(filename, g_testImgName->substr(0, g_testImgName->size()-4).c_str());
  strcat(filename, " ");
  strcat(filename, ethsift_version());
  strcat(filename, ".csv");
  write_logfile(filename);
  // Return
  return !result;
}
