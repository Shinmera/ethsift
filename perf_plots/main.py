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
    measurements = read_logs(6)

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

    for method in measurements:
        p = PerformancePlot()
        p.set_method_used(method)
        p.plot_pi()
        for lib in measurements[method]:
            p.plot_points(x=np.array(measurements[method][lib]['resolutions']),
                        y=np.array(measurements[method][lib]['performance']),
                        marker=lib_markers[lib],
                        point_label=lib,
                        color=lib_cols[lib],
                        linewidth=5,
                        linestyle='solid',
                        markersize=12
                        )
            if lib == 'eth':
                p.set_peak_performance(np.amax(measurements[method][lib]['performance']))
        p.plot_graph()

    

if __name__ == '__main__':
    main()