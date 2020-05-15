import numpy as np
import math

from flops_util2 import flops_util2, rw_util
from flops_util import flops_util
from architecture_config import config as arch_conf

from os import listdir
from os.path import isfile, join, dirname, realpath, basename

# Settings for reading the logs
logs_folder = "../baseline_logs/"
flops_logs = "../flops_logs/"

start_line = 1
end_line = 21
scriptdir = dirname(realpath(__file__))

## Horrible hack to make the other horrible hacks work
resolution_map = {}

def read_logs(logs_folder, mode, flops_util_version=2, version=''):
    match = 'chrono'
    if(mode == 'rdtsc'):
        match = 'rdtsc'
    
    onlyfiles = [join(logs_folder,f) for f in listdir(logs_folder)
                 if isfile(join(logs_folder, f))
                 and match in f
                 and version in f]
    onlyfiles = np.sort(onlyfiles)
    print(onlyfiles)

    resolution_map = {}
    for f in onlyfiles:
        if not resolution_label(f) in resolution_map:
            resolution_map[resolution_label(f)] = pgm_resolution(image_file(f))
            print("Created keyword " + resolution_label(f))
    
    
    init_flops_util2()

    if mode == 'runtime':
        return get_runtime_measurements(onlyfiles)
    elif mode == 'stacked_runtime':
        return get_runtime_bars(onlyfiles)
    else:
        return get_performance_measurements(onlyfiles, mode, flops_util_version)

def resolution_label(f):
    return basename(f).split('_')[2].split('-')[1]

def image_name(f):
    return basename(f).split('_')[2]

def image_file(f):
    return join(scriptdir, '../data/', image_name(f)+'.pgm')

def pgm_resolution(f):
    with open(f,"r",encoding="latin-1") as stream:
        line = stream.readline()
        if line != 'P5\n':
            raise Exception(f, 'is not a PGM file!')
        line = stream.readline()
        while line.startswith('#'):
            line = stream.readline()
        size = [ int(f) for f in line.rstrip('\n').split(' ') ]
        return {'width': size[0], 'height': size[1], 'tot_pixels': size[0]*size[1]}

def get_performance_measurements(log_files, mode, flops_util_version):
    # modes are rdtsc, chrono and runtime
    measurements = dict()

    for f in log_files:                
        stream = open(f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = resolution_label(f)
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
                flops = flops_util2[func_name][resolution]


            measurements[func_name][lib]['performance'].append(flops / cycles)
            measurements[func_name][lib]['std'].append(flops / (std_dev+cycles) - flops / cycles )
            measurements[func_name][lib]['resolutions'].append(resolution_map[resolution]['tot_pixels'])

    return measurements, dict()

def get_runtime_measurements(log_files):
    # modes are rdtsc, chrono and runtime
    measurements = dict()

    for f in log_files:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = resolution_label(f)
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
        stream = open(f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = resolution_label(f)
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
        resolution = resolution_label(f)
        print("Creation  " + resolution)
        for l in lines:
            vals = l.split(',')
            if vals[0] not in flops_util2:
                flops_util2[vals[0]] = dict()
            if vals[0] not in rw_util:
                rw_util[vals[0]] = dict()

            flops_util2[vals[0]][resolution] = int(vals[1])
            rw_util[vals[0]][resolution] = int(vals[2])
