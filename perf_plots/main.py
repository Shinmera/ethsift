from read_logs import read_logs
from performance_plots import PerformancePlot
from runtime_plots import RuntimePlot
from stacked_bars_plot import StackedPlot
from matplotlib import cm

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
    # modes are rdtsc, chrono and runtime, stacked_runtime
    reading_mode = 'stacked_runtime' 
    show_plots = True
    save_plots = True
    measurements, tot_runtimes = read_logs(reading_mode)

    if reading_mode is 'runtime':
        make_runtime_plot(measurements=measurements, show_plot=show_plots, autosave=save_plots)
    elif reading_mode is 'stacked_runtime': 
        make_stackedruntime_plot(measurements=measurements, tot_runtimes=tot_runtimes, show_plot=show_plots, autosave=save_plots)
    else:
        make_performance_plot(measurements=measurements, cycle_measurement_method=reading_mode, show_plot=show_plots, autosave=save_plots)


def make_performance_plot(measurements, cycle_measurement_method, show_plot=True, autosave=True, debug=False):
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
                          markersize=12,
                          error=np.array(measurements[function][lib]['std']))
            if lib == 'eth':
                temp = np.amax(measurements[function][lib]['performance'])
                peak_perf = max(temp, peak_perf)
                p.set_peak_performance(peak_perf)
        p.plot_graph(function)

def make_runtime_plot(measurements, show_plot=True, autosave=True, debug=False):
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

    longest_runtime = 0
    nr_lines = 0
    for function in measurements:
        for lib in measurements[function]:
            nr_lines += 1
            temp_max = np.amax(measurements[function][lib]['runtime'])
            longest_runtime = max(temp_max, longest_runtime)

    p = RuntimePlot(y_max=longest_runtime)
    col_map =cm.get_cmap('jet', nr_lines)
    colors = col_map(np.linspace(0, 1, nr_lines))
    it = 0
    for function in measurements:
        for lib in measurements[function]:            
            p.plot_points(x=np.array(measurements[function][lib]['resolutions']),
                        y=np.array(measurements[function][lib]['runtime']),
                        marker=lib_markers[lib],
                        point_label=lib + " " + function,
                        color=colors[it],
                        markersize=12,
                        error=np.array(measurements[function][lib]['std']))
            it += 1
    p.plot_graph("All Functions", show=show_plot, autosave=autosave)

def make_stackedruntime_plot(measurements, tot_runtimes, show_plot=True, autosave=True, debug=True):
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

    longest_runtime = 0
    nr_lines = 0
    for function in measurements:
        for lib in measurements[function]:
            nr_lines += 1
            temp_max = np.amax(measurements[function][lib]['runtime'])
            longest_runtime = max(temp_max, longest_runtime)

    np_runtimes = dict()
    for lib in tot_runtimes:
        nr_resolutions = len([tot_runtimes[lib][i] for i in tot_runtimes[lib] if tot_runtimes[lib][i]>0])
        np_runtimes[lib]= np.zeros(nr_resolutions)
        it = 0
        for res_key in tot_runtimes[lib]:
            if it < nr_resolutions:
                np_runtimes[lib][it] = tot_runtimes[lib][res_key]
                it += 1

    plots = dict()
    
    col_map =cm.get_cmap('jet', nr_lines)
    colors = col_map(np.linspace(0, 1, nr_lines))

    for function in measurements:
        for lib in measurements[function]:
            if lib not in plots:
                nr_resolutions = len([tot_runtimes[lib][i] for i in tot_runtimes[lib] if tot_runtimes[lib][i]>0])
                plots[lib] = StackedPlot(nr_resolutions)   
            
            y_mod = np.array(measurements[function][lib]['runtime']) / np_runtimes[lib]
            
            error=np.array(measurements[function][lib]['std']) / np_runtimes[lib] 
            plots[lib].plot_points(y=y_mod, std=error, func_name=function)
    for lib in plots:
        plots[lib].plot_graph("All Functions " + lib, show=show_plot, autosave=autosave)

        
if __name__ == '__main__':
    main()
