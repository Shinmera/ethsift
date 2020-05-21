#!/usr/bin/python3
from read_logs import read_logs
from performance_plots import PerformancePlot
from runtime_plots import RuntimePlot
from stacked_bars_plot import StackedPlot
from roofline_plots import RooflinePlot
from matplotlib import cm
import os
import numpy as np
import math

lib_markers = dict()
lib_markers['ethSIFT avx full flags'] = '*'
lib_markers['ethSIFT avx full flags No Memory Allocation'] = '+'
lib_markers['ethSIFT std-c full flags'] = 'x'
lib_markers['ethSIFT baseline full flags'] = '^'
lib_markers['ezSIFT O3'] = 'v'
scriptdir = os.path.dirname(os.path.realpath(__file__))

def main():
    print("Start Plotting Script")
    

    # Instead of opening plot window, auto-save images to perf_plot folder 
    save_plots = True 
    
    # Save files to following image format
    img_format = 'png'

    # Choosing the flops util version for measuring flops-count. Options:
    # - 1: Taking lambda functions from flops_util.py
    # - 2: Taking results from code of Jan which explicitly counted the flops.
    using_flops_util_version = 2
    
    version = os.getenv('VERSION','')
    directory = os.getenv('LOGS', os.path.join(scriptdir,'../logs/'))

    # Measurement methods are
    #   - rdtsc (requires measurements in cycles)
    #   - chrono (requires measurements to be in microseconds)
    if os.getenv('MEAS_METHOD') == None:
        meas_method = 'rdtsc'
    else:
        meas_method = os.getenv('MEAS_METHOD')

    # modes are:
    #   - performance
    #   - runtime
    #   - stacked_runtime
    if os.getenv('PLOT_MODE') == None:
        for mode in ['performance', 'runtime', 'stacked_runtime', 'roofline']:
            make_plots_for(directory, mode, meas_method, version, 
                           save_plots=save_plots, 
                           img_format=img_format, 
                           flops_util_version=using_flops_util_version)
    else:
        make_plots_for(directory, os.getenv('PLOT_MODE'), meas_method, version, 
                       save_plots=save_plots, 
                       img_format=img_format, 
                       flops_util_version=using_flops_util_version)


def make_plots_for(logs_folder, plot_mode, meas_method, version="", save_plots=True, img_format='png', flops_util_version=2):
    measurements, tot_runtimes = read_logs(logs_folder, 
                                        measurement_method=meas_method, 
                                        mode=plot_mode, 
                                        version=version, 
                                        flops_util_version=flops_util_version)

    if plot_mode == 'runtime':
        print("\nCreate Runtime Plot\n")
        make_runtime_plot(measurements=measurements, 
                          cycle_measurement_method=meas_method, 
                          autosave=save_plots, 
                          img_format=img_format)
    elif plot_mode == 'stacked_runtime':
        print("\nCreate StackedBar Plot\n")
        make_stackedruntime_plot(measurements=measurements, 
                                 tot_runtimes=tot_runtimes, 
                                 autosave=save_plots, 
                                 img_format=img_format)
    elif plot_mode == 'roofline':
        print("\nCreate Roofline Plot\n")
        make_roofline_plot(measurements=measurements, 
                                 tot_runtimes=tot_runtimes, 
                                 autosave=save_plots, 
                                 img_format=img_format)
    else:
        print("\nCreate Performance Plots\n")
        make_performance_plot(measurements=measurements, 
                              cycle_measurement_method=meas_method, 
                              autosave=save_plots, 
                              img_format=img_format)
                              
#===PLOTTING OF DIFFERENT MODES===#
def make_performance_plot(measurements, cycle_measurement_method, autosave=True, img_format='svg', debug=False):
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
        p.plot_pi(linewidth=2)
        peak_perf = 0
        
        nr_libs = len(measurements[function])
        col_map = cm.get_cmap('Dark2', nr_libs)
        colors = col_map(np.linspace(0, 1, nr_libs))
        it = 0
        for lib in measurements[function]:
            p.plot_points(x=np.array(measurements[function][lib]['resolutions']),
                          y=np.array(measurements[function][lib]['performance']),
                          linewidth=1.5,
                          marker=lib_markers[lib],
                          point_label=lib,
                          color=colors[it],
                          markersize=8,
                          error=np.array(measurements[function][lib]['std'])
                          )            
            if 'eth' in lib:
                temp = np.amax(measurements[function][lib]['performance'])
                peak_perf = max(temp, peak_perf)
                p.set_peak_performance(peak_perf)
            it += 1
        
        p.plot_graph(function, autosave=autosave, img_format=img_format)

def make_runtime_plot(measurements, cycle_measurement_method, show_plot=True, autosave=True, img_format='svg', debug=False):
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

    p = RuntimePlot(y_max=longest_runtime, meas_method=cycle_measurement_method)
    col_map =cm.get_cmap('tab20b', nr_lines)
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
                        error=np.array(measurements[function][lib]['std'])
                        )
            it += 1
    p.plot_graph("All Functions", autosave=autosave, img_format=img_format)

def make_stackedruntime_plot(measurements, tot_runtimes, autosave=True, img_format='svg', debug=False):
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
    
    prev = dict()

    for function in measurements:
        for lib in measurements[function]:
            if lib not in plots:
                nr_resolutions = len([tot_runtimes[lib][i] for i in tot_runtimes[lib] if tot_runtimes[lib][i]>0])
                plots[lib] = StackedPlot(nr_resolutions)   
            
            if lib not in prev:
                prev[lib] = None
            
            y_mod = np.array(measurements[function][lib]['runtime']) / np_runtimes[lib]
            
            error=np.array(measurements[function][lib]['std']) / np_runtimes[lib] 
            if prev[lib] is None:
                plots[lib].plot_points(y=y_mod, func_name=function)
                prev[lib] = y_mod
            else:
                plots[lib].plot_points(y=y_mod, func_name=function, bottom=prev[lib])
                prev[lib] = np.add(y_mod, prev[lib])


    for lib in plots:
        plots[lib].plot_graph("Stacked Proportional Runtime " + lib, autosave=autosave, img_format=img_format)


def make_roofline_plot(measurements, tot_runtimes, autosave=True, img_format='svg', debug=False):
    # Only test code because I cannot run the read log files
    plt = RooflinePlot(64)

    plt.plot_bounds()

    plt.plot_graph("Banana")
    

        
if __name__ == '__main__':
    main()
