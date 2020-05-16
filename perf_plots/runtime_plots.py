import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math

from read_logs import get_resolutions_in_pixels, get_resolutions_in_labels

from architecture_config import config as arch_conf

class RuntimePlot:
    def __init__(self, y_max, meas_method):
        self.title = "Runtime "
        self.x_label ="Image Resolution [pixels]"
        if meas_method == 'rdtsc':
            self.y_label = "Runtime [cycles]"
        else:
            self.y_label = "Runtime [\u03BCs]"

        self.title_font = {'fontname':'Calibri'}
        self.init_plot(y_max)

    def init_plot(self, y_max):
        self.max_performance = 0

        self.fig = plt.figure( figsize=arch_conf['figure_size'])
        x_offset_min = 30000
        x_offset_max = 1000*x_offset_min
        self.x_min = 427*240 - x_offset_min
        self.x_max = 7680*4320 + x_offset_max
        self.y_min = 0
        self.y_max = y_max + 10
        
        self.axes= self.fig.add_axes([0.1,0.1,0.8,0.8])

        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 
    
    def set_title(self, title, with_peak_perf=False, peak_performance=0):
        self.title = title


    def plot_points(self, x, y, marker, color='c', linestyle='dashed', point_label='', linewidth=2, capsize=10, markersize=8, error=None):
        if(error is None):
            self.axes.plot(x, y, color=color, marker=marker, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)
        else:
            self.axes.errorbar(x, y, yerr=error, color=color, marker=marker, capsize=capsize, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)
        

    def plot_graph(self, func_name, autosave=False, img_format='svg'):  
        self.axes.legend()  
        self.axes.set_xscale('log', basex=2)
        # self.axes.set_yscale('log', basey=2)
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
       
        self.fig.suptitle(self.title, **self.title_font, fontsize=25)
        
        plt.rcParams['axes.facecolor'] = 'xkcd:light grey'
        self.axes.grid(color='w', linestyle='-', linewidth=0.5)
        self.axes.xaxis.grid() # only showing horizontal lines
        self.axes.set_xlabel(self.x_label, fontsize=15)
        
        self.axes.set_xticks(get_resolutions_in_pixels())
        self.axes.set_xticklabels(get_resolutions_in_labels())
        self.axes.set_ylabel(self.y_label, fontsize=15, rotation=0, labelpad=45)

        if autosave:
            self.fig.savefig("runtimeplot_"+func_name.lower().replace(' ', '_') + '.' + img_format,
                        dpi=None, facecolor='w', edgecolor='w',
                        orientation='portrait', papertype=None, format=img_format,
                        transparent=False, bbox_inches=None, pad_inches=0.1,
                        frameon=None, metadata=None)
        else:
            self.fig.show()




    

