from read_logs import read_logs
from performance_plots import PerformancePlot

import numpy as np
import math

lib_markers = dict()
lib_markers['eth'] = '*'
lib_markers['ez'] = '^'



def main():
    print("Start Plotting Script")
    measurements = read_logs(6)
    print(measurements)

    for method in measurements:
        p = PerformancePlot()
        p.set_method_used(method)
        p.plot_pi()
        for lib in measurements[method]:
            p.plot_points(x=np.array(measurements[method][lib]['resolutions']),
                        y=np.array(measurements[method][lib]['performance']),
                        marker=lib_markers[lib],
                        point_label=lib)
        p.plot_graph()

    

if __name__ == '__main__':
    main()