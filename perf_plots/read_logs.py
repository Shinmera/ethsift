import numpy as np
import math

from os import listdir
from os.path import isfile, join

# Settings for reading the logs
logs_folder = "../logs"

start_line = 1
end_line = 21

resolution_map = dict()
resolution_map['240p'] = 240*427
resolution_map['360p'] = 360*640
resolution_map['480p'] = 480*853
resolution_map['720p'] = 720*1280
resolution_map['1080p'] = 1080*1920
resolution_map['2160p'] = 2160*3840
resolution_map['4320p'] = 4320*7680

def read_logs(nr_resoltuions):

    measurements = dict()
    resolutions = np.zeros(nr_resoltuions)
    
    onlyfiles = [f for f in listdir(logs_folder) if isfile(join(logs_folder, f))]    
    onlyfiles = np.sort(onlyfiles)

    index = 0
    for f in onlyfiles:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        
        resolution = f.split('-')[1].split('.')[0]
        resolutions[index] = resolution_map[resolution]

        for i in range(start_line, end_line):
            vals = lines[i].split(',')
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
                measurements[func_name][lib]['median'] = np.zeros(nr_resoltuions)
                measurements[func_name][lib]['std'] = np.zeros(nr_resoltuions)
            
            measurements[func_name][lib]['median'][index] = median
            measurements[func_name][lib]['std'] = std_dev

    return measurements, resolutions

