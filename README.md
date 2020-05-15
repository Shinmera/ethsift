# ETH 2020 Systems Lab
Implementation of the SIFT algorithm for image feature matching

## Building
In order to build ethsift you need a recent version of GCC and Cmake.

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DMEASUREMENT_MODE=chrono
make
```

When debugging, change `Release` to `Debug`. This will compile the static library version of ethsift and create a tester application, creatively called `tester`. To minimise noise in measurements, the tester can only measure either cycles or runtime. Which to measure must be configured with the above cmake flag `MEASUREMENT_MODE`, which can be either `chrono` (runtime) or `rdtsc` (cycles).

The tester can be run as follows:

```
tester [image [test...]]

   image  --- The name of the image file to use for measurements.
              Must be a .pgm file in the data/ directory.
   test   --- The name of a test defined in our test files. If one or
              more tests are specified, only the specified tests are
              actually run, otherwise all tests are run.

Additionally, the following environment variables are considered:

   IMAGE  --- Same as the command line image option.
   RUNS   --- How many times to repeat a measurement. Defaults to 30.
   TESTS  --- A space delimited list of test names, fulfilling the same
              purpose as the command line arguments option.

If a test fails, the return value will be 1, otherwise 0.
```

## Profiling
A few helper targets for profiling are available as well. To generate a flame graph:

```
make flamegraph
```

This requires the `perf` utility and a `perl` installation. It will output to a `flamegraph.svg` file.

To generate a precise profiling record of the entire program execution:

```
make gperf
```

This requires the `gperf` utility, and requires the project to be built in `Debug` mode. It will output to a `gperf.txt` file.

Both utilities by default only consider the `eth_MeasureFull` test that should give an overview of a standard execution of the SIFT algorithm. If you would like to measure different tests, you can do so with the `TESTS` environment variable.

To generate performance plots:

```
make plots
```

This requires a python3 installation with matplotlib and numpy. The plotter can be configured to select log files from a different directory using the `LOGS` environment variable, and to limit the files in the directory to ones containing a specified string in their name using `VERSION`. By default it will generate all plots. You can set it to only generate a certain kind of plot by setting `PLOT_MODE`.
