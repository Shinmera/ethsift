import numpy as np
import math

from flops_util2 import flops_util2, rw_util
from flops_util import flops_util
from architecture_config import config as arch_conf

from os import listdir
from os.path import isfile, join

# Settings for reading the logs
logs_folder = "../firstmeetinglogs/"
flops_logs = "../flops_logs/"

start_line = 1
end_line = 21

resolution_map = dict()
resolution_map['240p']= dict()
resolution_map['240p']['width'] = 427
resolution_map['240p']['height'] = 240
resolution_map['240p']['tot_pixels'] = resolution_map['240p']['width']*resolution_map['240p']['height'] 

resolution_map['360p']= dict()
resolution_map['360p']['width'] = 640
resolution_map['360p']['height'] = 360
resolution_map['360p']['tot_pixels'] = resolution_map['360p']['width']*resolution_map['360p']['height'] 

resolution_map['480p']= dict()
resolution_map['480p']['width'] = 853 
resolution_map['480p']['height'] = 480
resolution_map['480p']['tot_pixels'] = resolution_map['480p']['width']*resolution_map['480p']['height'] 

resolution_map['720p']= dict()
resolution_map['720p']['width'] = 1280
resolution_map['720p']['height'] = 720
resolution_map['720p']['tot_pixels'] = resolution_map['720p']['width']*resolution_map['720p']['height'] 

resolution_map['1080p']= dict()
resolution_map['1080p']['width'] = 1920
resolution_map['1080p']['height'] = 1080
resolution_map['1080p']['tot_pixels'] = resolution_map['1080p']['width']*resolution_map['1080p']['height'] 

resolution_map['2160p']= dict()
resolution_map['2160p']['width'] = 3840
resolution_map['2160p']['height'] = 2160
resolution_map['2160p']['tot_pixels'] = resolution_map['2160p']['width']*resolution_map['2160p']['height'] 

resolution_map['4320p']= dict()
resolution_map['4320p']['width'] = 7680
resolution_map['4320p']['height'] = 4320
resolution_map['4320p']['tot_pixels'] = resolution_map['4320p']['width']*resolution_map['4320p']['height'] 

def read_logs(mode='rdtsc', flops_util_version=2):    
    # modes are rdtsc, chrono and runtime    
    onlyfiles = [f for f in listdir(logs_folder) if isfile(join(logs_folder, f))]    
    onlyfiles = np.sort(onlyfiles)
    print(onlyfiles)
    if flops_util_version:
        init_flops_util2()
    if mode is 'runtime':
        return get_runtime_measurements(onlyfiles, flops_util_version= flops_util_version)
    elif mode is 'stacked_runtime':
        return get_runtime_bars(onlyfiles)
    else:
        return get_performance_measurements(onlyfiles, mode)

def get_performance_measurements(log_files, mode, flops_util_version=2):
    # modes are rdtsc, chrono and runtime
    measurements = dict()

    for f in log_files:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = f.split('-')[1].split('_')[0]
        for l in lines:
            vals = l.split(',')
            method_name_split = vals[0].split('_')
            lib = method_name_split[0]
            func_name = method_name_split[1]
            median = int(vals[1])
            std_dev = float(vals[2])
                        
            if func_name in measurements:
                pass
            elif func_name == "Octaves" or func_name == "GaussianKernelGeneration":
                continue
            else:                
                measurements[func_name] = dict()

            if lib in measurements[func_name]:
                pass
            else:
                measurements[func_name][lib] = dict()
                measurements[func_name][lib]['performance'] = []
                measurements[func_name][lib]['resolutions'] = []
                measurements[func_name][lib]['std'] = []

            if mode == 'rdtsc':
                #if we already measured the cycles of the method, simply calculate cycles/flops
                cycles = median
            elif mode == 'chrono':
                cycles = get_cycles_from_time_measurement(median)
                std_dev = get_cycles_from_time_measurement(std_dev)

            if flops_util_version is 1:
                flops = flops_util[lib][func_name](resolution_map[resolution]['width'], resolution_map[resolution]['height'])
            else:
                print("Access  " + resolution + "  " + func_name + "  " + lib)
                flops = flops_util2[lib][func_name][resolution]


            measurements[func_name][lib]['performance'].append(flops / cycles)
            measurements[func_name][lib]['std'].append(flops / std_dev)
            measurements[func_name][lib]['resolutions'].append(resolution_map[resolution]['tot_pixels'])

    return measurements, dict()

def get_runtime_measurements(log_files):
    # modes are rdtsc, chrono and runtime
    measurements = dict()

    for f in log_files:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = f.split('-')[1].split('_')[0]
        for l in lines:
            vals = l.split(',')
            method_name_split = vals[0].split('_')
            lib = method_name_split[0]
            func_name = method_name_split[1]
            median = int(vals[1])
            std_dev = float(vals[2])
                        
            if func_name in measurements:
                pass
            else:                
                measurements[func_name] = dict()

            if lib in measurements[func_name]:
                pass
            else:
                measurements[func_name][lib] = dict()
                measurements[func_name][lib]['runtime'] = []
                measurements[func_name][lib]['resolutions'] = []
                measurements[func_name][lib]['std'] = []

            measurements[func_name][lib]['runtime'].append(median)
            measurements[func_name][lib]['std'].append(std_dev)
            measurements[func_name][lib]['resolutions'].append(resolution_map[resolution]['tot_pixels'])

    return measurements, dict()

def get_runtime_bars(log_files):
    # modes are rdtsc, chrono and runtime
    measurements = dict()

    for f in log_files:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = f.split('-')[1].split('_')[0]
        for l in lines:
            vals = l.split(',')
            method_name_split = vals[0].split('_')
            lib = method_name_split[0]
            func_name = method_name_split[1]
            median = int(vals[1])
            std_dev = float(vals[2])
                        
            if func_name in measurements:
                pass
            else:                
                measurements[func_name] = dict()

            if lib in measurements[func_name]:
                pass
            else:
                measurements[func_name][lib] = dict()
                measurements[func_name][lib]['runtime'] = []
                measurements[func_name][lib]['resolutions'] = []
                measurements[func_name][lib]['std'] = []

            measurements[func_name][lib]['runtime'].append(median)
            measurements[func_name][lib]['std'].append(std_dev)
            measurements[func_name][lib]['resolutions'].append(resolution_map[resolution]['tot_pixels'])

    tot_runtimes = dict()
    
    for func_name in measurements:
        for lib in measurements[func_name]:
            if lib in tot_runtimes:
                pass
            else:
                tot_runtimes[lib] = dict()
                for res_key in resolution_map:
                    tot_runtimes[lib][res_key] = 0
            it =0
            for res_key in resolution_map:
                if it < len(measurements[func_name][lib]['runtime']):
                    tot_runtimes[lib][res_key] += measurements[func_name][lib]['runtime'][it]
                    it += 1
            
    return measurements, tot_runtimes



def get_cycles_from_time_measurement(median):
    #calculate the approximate number of cycles the algorithm had according to the time measured
    return median * arch_conf['frequency']

def get_resolutions_in_pixels():
    all_res = []
    for key in resolution_map:
        all_res.append(resolution_map[key]['tot_pixels'])
    return np.array(all_res)

def get_resolutions_in_labels():
    all_res = []
    for key in resolution_map:
        all_res.append(key)
    return np.array(all_res)

def get_resolution_in_indices():    
    all_res = []
    it = 1
    for key in resolution_map:
        all_res.append(it)
        it +=1
    return np.array(all_res)


def init_flops_util2():
    onlyfiles = [f for f in listdir(flops_logs) if isfile(join(flops_logs, f))]    
    onlyfiles = np.sort(onlyfiles)

    
    for f in onlyfiles:
        stream = open(flops_logs + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = f.split('_')[2].split('-')[1].split('.')[0]
        print("Creation  " + resolution)
        for l in lines:
            vals = l.split(',')
            flops_util2['eth'][vals[0]][resolution] = int(vals[1])
            rw_util['eth'][vals[0]][resolution] = int(vals[2])

            flops_util2['ez'][vals[0]][resolution] = int(vals[1])
            rw_util['ez'][vals[0]][resolution] = int(vals[2])