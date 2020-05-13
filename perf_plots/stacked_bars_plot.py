import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math


from architecture_config import config as arch_conf

class StackedPlot:
    def __init__(self, nr_resolutions):
        self.title = "Runtime "
        self.x_label ="Image Resolution [pixels]"
        self.y_label = "Runtime [\u03BCs]"
        self.title_font = {'fontname':'Calibri'}
        
        self.nr_resolutions = nr_resolutions
        self.init_plot()

    def init_plot(self):
        fig = plt.figure( figsize=arch_conf['figure_size'])
        x_offset_min = 30000
        x_offset_max = 1000*x_offset_min
        self.x_min = 427*240 - x_offset_min
        self.x_max = 7680*4320 + x_offset_max
        self.y_min = 0
        self.y_max = 1
        
        self.axes= fig.add_axes([0.1,0.1,0.8,0.8])

        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 
    
    def set_title(self, title, with_peak_perf=False, peak_performance=0):
        self.title = title

    def plot_points(self, y, std=None, func_name='', width=0.5):
        ind = np.arange(self.nr_resolutions)
        if std is None:
            std = np.zeros(self.nr_resolutions)
        b = plt.bar(ind, y, width, yerr=std)
        plt.legend((b[0]), (func_name))

    def plot_graph(self, graph_name, show=True, autosave=False, format='svg'):  
        self.axes.legend()  
        self.axes.set_xscale('log', basex=2)
        # self.axes.set_yscale('log', basey=2)
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
       
        plt.suptitle(self.title, **self.title_font, fontsize=25)
        
        
        plt.xlabel(self.x_label, fontsize=15)
        plt.ylabel(self.y_label, fontsize=15)
                
        if autosave:
            plt.savefig("stackedplot_"+graph_name.lower().replace(' ', '_') + '.' + format,
                        dpi=None, facecolor='w', edgecolor='w',
                        orientation='portrait', papertype=None, format=format,
                        transparent=False, bbox_inches=None, pad_inches=0.1,
                        frameon=None, metadata=None)
        if show:
            plt.show()




    

