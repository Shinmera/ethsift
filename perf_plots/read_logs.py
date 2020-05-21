import numpy as np
import math

from flops_util import flops_util
from architecture_config import config as arch_conf

from os import listdir
from os.path import isfile, join, dirname, realpath, basename, isdir

# Settings for reading the logs
#logs_folder = "../baseline_logs/"
flops_logs = "../flops_logs/"

start_line = 1
end_line = 21
scriptdir = dirname(realpath(__file__))

## Horrible hack to make the other horrible hacks work
resolution_map = {}

lib_to_fc_rw_map = dict()

def read_logs(logs_folder, measurement_method, mode='performance', flops_util_version=2, version=''):
    # can be 'chrono' or 'rdtsc'
    match = measurement_method
    
    libs = [lib_name for lib_name in listdir(logs_folder) if isdir(join(logs_folder,lib_name))]
    libs = np.sort(libs)

    onlyfiles = []
    for lib in libs:
        folder = join(logs_folder,lib)
        onlyfiles += [join(folder,f) for f in listdir(folder)
                 if isfile(join(folder, f))
                 and match in f
                 and version in f
                 and ".csv" in f]
        fc_file = [f for f in listdir(folder)
                if isfile(join(folder, f)) and ".fc" in f]
        print(fc_file)
        lib_to_fc_rw_map[lib] = dict()
        lib_to_fc_rw_map[lib]['fc'], lib_to_fc_rw_map[lib]['rw'] = init_flops_util2(fc_file[0].split('.')[0])
        

    onlyfiles = np.sort(onlyfiles)

    for f in onlyfiles:
        if resolution_label(f) not in resolution_map:
            resolution_map[resolution_label(f)] = pgm_resolution(image_file(f))
    
    

    if mode == 'runtime':
        return get_runtime_measurements(onlyfiles, libs)
    elif mode == 'stacked_runtime':
        return get_runtime_bars(onlyfiles, libs)
    else:
        return get_performance_measurements(onlyfiles, libs, measurement_method, flops_util_version)

def resolution_label(f):
    return basename(f).split('_')[2].split('-')[1]

def image_name(f):
    return basename(f).split('_')[2]

def image_file(f):
    return join(scriptdir, '../data/', image_name(f)+'.pgm')

def get_lib_name(f, libs):
    name = 'unkown_lib'
    for lib in libs:
        if (lib+'/') in f:
            name = lib
    return name

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

def get_performance_measurements(log_files, libs, mode, flops_util_version):
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
            lib = get_lib_name(f, libs)
            func_name = method_name_split[1]
            if func_name == "MeasureFullNoAlloc":
                func_name = "MeasureFull"
                lib += " No Memory Allocation"
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
            #elif mode == 'chrono':
            else:
                cycles = get_cycles_from_time_measurement(median)
                std_dev = get_cycles_from_time_measurement(std_dev)

            if flops_util_version is 1:
                flops = flops_util[lib][func_name](resolution_map[resolution]['width'], resolution_map[resolution]['height'])
            elif func_name == "Downscale":
                flops = 0
            else:
                flops = lib_to_fc_rw_map[lib]['fc'][func_name][resolution]


            measurements[func_name][lib]['performance'].append(flops / cycles)
            measurements[func_name][lib]['std'].append(flops / (std_dev+cycles) - flops / cycles )
            measurements[func_name][lib]['resolutions'].append(resolution_map[resolution]['tot_pixels'])

    return measurements, dict()

def get_runtime_measurements(log_files, libs):
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
            lib = get_lib_name(f, libs)
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

def get_runtime_bars(log_files, libs):
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
            lib = get_lib_name(f, libs)
            func_name = method_name_split[1]
            median = int(vals[1])
            std_dev = float(vals[2])
                        
            if func_name in measurements:
                pass                        
            elif func_name == "MeasureFull" or func_name == "MeasureFullNoAlloc":
                continue
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


def init_flops_util2(folder_name):
    flops_util2 = dict()
    rw_util = dict()
    full_folder= join(flops_logs, folder_name)
    print(full_folder)
    onlyfiles = [f for f in listdir(full_folder) if isfile(join(full_folder, f))]    
    onlyfiles = np.sort(onlyfiles)
    
    for f in onlyfiles:
        stream = open(join(full_folder, f),"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = resolution_label(f)
        for l in lines:
            vals = l.split(',')
            if vals[0] not in flops_util2:
                flops_util2[vals[0]] = dict()
            if vals[0] not in rw_util:
                rw_util[vals[0]] = dict()

            flops_util2[vals[0]][resolution] = int(vals[1])
            rw_util[vals[0]][resolution] = int(vals[2])
    
    return flops_util2, rw_util
