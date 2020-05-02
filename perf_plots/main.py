from read_logs import read_logs
from performance_plots import PerformancePlot

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
    measurement_method = 'rdtsc'
    measurements = read_logs(6, measurement_method)

    if False:
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
        p.set_method_used(measurement_method)
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

    

if __name__ == '__main__':
    main()