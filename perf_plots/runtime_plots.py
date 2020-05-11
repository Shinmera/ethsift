import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math


from architecture_config import config as arch_conf

class RuntimePlot:
    def __init__(self, y_max):
        self.title = "Runtime "
        self.x_label ="Image Resolution [pixels]"
        self.y_label = "Runtime [\u03BCs]"
        self.title_font = {'fontname':'Calibri'}
        self.init_plot(y_max)

    def init_plot(self, y_max):
        self.max_performance = 0

        fig = plt.figure()
        x_offset_min = 30000
        x_offset_max = 1000*x_offset_min
        self.x_min = 427*240 - x_offset_min
        self.x_max = 7680*4320 + x_offset_max
        self.y_min = 0
        self.y_max = y_max + 10
        
        self.axes= fig.add_axes([0.1,0.1,0.8,0.8])

        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 
    
    def set_title(self, title, with_peak_perf=False, peak_performance=0):
        self.title = title

    def plot_points(self, x, y, marker, color='c', linestyle='dashed', point_label='', linewidth=2, markersize=8):
        self.axes.plot(x, y, color=color, marker=marker, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)

    def set_peak_performance(self, perf):
        self.max_performance =perf

    def plot_graph(self, func_name):  
        self.axes.legend()  
        self.axes.set_xscale('log', basex=2)
        # self.axes.set_yscale('log', basey=2)
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
       
        plt.suptitle(self.title, **self.title_font, fontsize=25)
        
        
        plt.xlabel(self.x_label, fontsize=15)
        plt.ylabel(self.y_label, fontsize=15)
        plt.show()




    

