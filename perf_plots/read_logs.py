import numpy as np
import math

from flops_util import flops_util

from os import listdir
from os.path import isfile, join

# Settings for reading the logs
logs_folder = "../firstmeetinglogs/"

start_line = 1
end_line = 21

resolution_map = dict()
resolution_map['240p']= dict()
resolution_map['240p']['width'] = 240
resolution_map['240p']['height'] = 427
resolution_map['240p']['tot_pixels'] = resolution_map['240p']['width']*resolution_map['240p']['height'] 

resolution_map['360p']= dict()
resolution_map['360p']['width'] = 360
resolution_map['360p']['height'] = 640
resolution_map['360p']['tot_pixels'] = resolution_map['360p']['width']*resolution_map['360p']['height'] 

resolution_map['480p']= dict()
resolution_map['480p']['width'] = 480
resolution_map['480p']['height'] = 853
resolution_map['480p']['tot_pixels'] = resolution_map['480p']['width']*resolution_map['480p']['height'] 

resolution_map['720p']= dict()
resolution_map['720p']['width'] = 720
resolution_map['720p']['height'] = 1280
resolution_map['720p']['tot_pixels'] = resolution_map['720p']['width']*resolution_map['720p']['height'] 

resolution_map['1080p']= dict()
resolution_map['1080p']['width'] = 1080
resolution_map['1080p']['height'] = 1920
resolution_map['1080p']['tot_pixels'] = resolution_map['1080p']['width']*resolution_map['1080p']['height'] 

resolution_map['2160p']= dict()
resolution_map['2160p']['width'] = 2160
resolution_map['2160p']['height'] = 3840
resolution_map['2160p']['tot_pixels'] = resolution_map['2160p']['width']*resolution_map['2160p']['height'] 

resolution_map['4320p']= dict()
resolution_map['4320p']['width'] = 4320
resolution_map['4320p']['height'] = 7680
resolution_map['4320p']['tot_pixels'] = resolution_map['4320p']['width']*resolution_map['4320p']['height'] 

def read_logs(nr_resoltuions, mode='rdtsc'):
    #modes are rdtsc and chrono
    measurements = dict()
    resolutions = np.zeros(nr_resoltuions)
    
    onlyfiles = [f for f in listdir(logs_folder) if isfile(join(logs_folder, f))]    
    onlyfiles = np.sort(onlyfiles)

    index = 0
    for f in onlyfiles:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        lines.pop(0)
        resolution = f.split('-')[1].split('_')[0]
        resolutions[index] = resolution_map[resolution]
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
                measurements[func_name][lib]['performance'] = []
                measurements[func_name][lib]['resolutions'] = []
                measurements[func_name][lib]['std'] = []

            if mode == 'rdtsc':
                #if we already measured the cycles of the method, simply calculate cycles/flops
                cycles = median
            elif mode == 'chrono':
                cycles = get_cycles_from_time_measurement(median)
                std_dev = get_cycles_from_time_measurement(std_dev)
            
            measurements[func_name][lib]['performance'].append(cycles / flops_util[lib][func_name](resolutions[index]))
            measurements[func_name][lib]['std'].append(std_dev / flops_util[lib][func_name](resolutions[index]))
            measurements[func_name][lib]['resolutions'].append(resolutions[index])

        index += 1

    return measurements, resolutions

def get_cycles_from_time_measurement(median):
    #calculate the approximate number of cycles the algorithm had according to the time measured
    return median