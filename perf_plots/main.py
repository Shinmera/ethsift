from read_logs import read_logs
from performance_plots import PerformancePlot
from runtime_plots import RuntimePlot

import numpy as np
import math

lib_markers = dict()
lib_markers['eth'] = '*'
lib_markers['ez'] = '^'
lib_cols = dict()
lib_cols['eth'] = '#207020'
lib_cols['ez'] = '#e0e070'



def main():
    print("Start Plotting Script")
    # modes are rdtsc, chrono and runtime
    reading_mode = 'rdtsc' 
    nr_of_different_resoltuions = 6
    measurements = read_logs(nr_of_different_resoltuions, reading_mode)

    if reading_mode == 'runtime':
        make_runtime_plot(measurements=measurements)
    else:
        make_performance_plot(measurements=measurements, cycle_measurement_method=reading_mode)


def make_performance_plot(measurements, cycle_measurement_method, debug=False):
    if debug:
        for key1 in measurements:
            print(key1)
            print("======================================================\n")
            for key2 in measurements[key1]:
                print(key2)
                for key3 in measurements[key1][key2]:
                    print(key3 + ":")
                    print(measurements[key1][key2][key3])
                print("\n")
            print("\n")
    for function in measurements:
        p = PerformancePlot()
        p.set_method_used(cycle_measurement_method)
        p.plot_pi(linewidth=3)
        peak_perf = 0
        for lib in measurements[function]:
            p.plot_points(x=np.array(measurements[function][lib]['resolutions']),
                        y=np.array(measurements[function][lib]['performance']),
                        marker=lib_markers[lib],
                        point_label=lib,
                        color=lib_cols[lib],
                        markersize=12
                        )
            if lib == 'eth':
                temp = np.amax(measurements[function][lib]['performance'])
                peak_perf = max(temp, peak_perf)
                p.set_peak_performance(peak_perf)
        p.plot_graph(function)

def make_runtime_plot(measurements):
     if debug:
        for key1 in measurements:
            print(key1)
            print("======================================================\n")
            for key2 in measurements[key1]:
                print(key2)
                for key3 in measurements[key1][key2]:
                    print(key3 + ":")
                    print(measurements[key1][key2][key3])
                print("\n")
            print("\n")

    for function in measurements:
        for lib in measurements[function]:
            longest_runtime = 0
            for lib in measurements[function]:
                temp_max = np.amax(measurements[function][lib]['runtime'])
                longest_runtime = max(temp, peak_perf)

    p = RuntimePlot(y_max=longest_runtime)
    p.plot_pi(linewidth=3)

    for function in measurements:
        for lib in measurements[function]:
            p.plot_points(x=np.array(measurements[function][lib]['resolutions']),
                        y=np.array(measurements[function][lib]['runtime']),
                        marker=lib_markers[lib],
                        point_label=lib + " " + function,
                        color=lib_cols[lib],
                        markersize=12
                        )
        p.plot_graph("All Functions")
if __name__ == '__main__':
    main()