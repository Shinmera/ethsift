import numpy as np
import math

from os import listdir
from os.path import isfile, join

logs_folder = "../logs"

start_line = 1
end_line = 21

nr_resoltuions = 6

def read_logs():
    x_axis = np.zeros(20)
    y_axis = np.zeros(20)

    measurements = dict()
    
    onlyfiles = [f for f in listdir(logs_folder) if isfile(join(logs_folder, f))]    
    onlyfiles = np.sort(onlyfiles)

    index = 0
    for f in onlyfiles:                
        stream = open(logs_folder + f,"r")
        lines = stream.readlines()
        resolution = int(f.split('-')[1].split('p')[0])
        for i in range(start_line, end_line):
            vals = lines[i].split(',')
            method_name_split = vals[0].split('_')
            lib = method_name_split[0]
            func_name = method_name_split[1]
            cycles = int(vals[1])
            std_dev = float(vals[2])
                        
            if func_name in measurements:
                pass
            else:                
                measurements[func_name] = dict()

            if lib in measurements[func_name]:
                pass
            else:
                measurements[func_name][lib] = dict()
                measurements[func_name][lib]['cycles'] = np.zeros(nr_resoltuions)
                measurements[func_name][lib]['res'] = np.zeros(nr_resoltuions)
                measurements[func_name][lib]['std'] = np.zeros(nr_resoltuions)
            
            measurements[func_name][lib]['cycles'][index] = cycles
            measurements[func_name][lib]['std'] = std_dev
            measurements[func_name][lib]['res']  = resolution

        if index == nr_resoltuions:
            index = 0
            


    return 

