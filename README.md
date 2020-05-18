# ETH 2020 Systems Lab
Implementation of the SIFT algorithm for image feature matching

## Building
In order to build ethsift you need a recent version of GCC and Cmake.

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DMEASUREMENT_MODE=chrono -DOPT_FLAGS=full
make
```

When debugging, change `Release` to `Debug`. This will compile the static library version of ethsift and create a tester application, creatively called `tester`. To minimise noise in measurements, the tester can only measure either cycles or runtime. Which to measure must be configured with the above cmake flag `MEASUREMENT_MODE`, which can be either `chrono` (runtime) or `rdtsc` (cycles). Finally, when running in `Release`, you can choose the set of optimisation flags passed to the compiler with `OPT_FLAGS`, which may be one of `full`, `avx`, `fastmath`, `O3`, `O2`, `O1`, and `O0`.

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

## Counting
For counting floating point operations and memory accesses. After building the application, run the following application (a `/logs/` folder must exist!)

```
./count_flops "filename"
```

Where filename specifies, which image file from `/data/` is used to count flops and memory accesses of our implementations (default is lena.pgm). The tool generates CSV logs that are saved in `/logs/` folder.


## Plotting
For plotting there are two things to consider before running "perf_plots/main.py":
- folder structures and naming in "logs" folder
- lib_markers (dictionary of markers for each plot curve) in main.py
- Run `./count_flops` first, and copy the generated output logs to `/flops_logs/` folder

First create for each measurement run which belongs together an own folder (e.g. "ethSIFT baseline") and place the logfiles which belong to that measurement into that folder. perf_plots library will automatically read in the name of the folder and chose it as label for the plot curve.
Second make sure, that for each folder name you create a dictionary entry in lib_markers and you set a distinct marker for that very folder. Use the full folder name as dictionary key.

## Measuring Different Variants
In order to conveniently measure and compare different variants of compilation configurations, you can use the provided `variant.sh` script. Simply supply a variant name and the desired CMake arguments. It will automatically configure a build, compile it, run it for all standard image sizes, and put the resulting logs into a directory named after the variant.

Example:
```
./variant.sh clang-O3 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DMEASUREMENT_MODE=chrono -DOPT_FLAGS=03
```

This will create a build directory called `build-clang-O3` and put the resulting measurement logs into `logs/clang-O3/`.
