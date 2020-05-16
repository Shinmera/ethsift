import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math

from read_logs import get_resolution_in_indices, get_resolutions_in_labels

from architecture_config import config as arch_conf

class StackedPlot:
    def __init__(self, nr_resolutions):
        self.title = "Stacked Proportional Runtime "
        self.x_label ="Image Resolution [pixels]"
        self.y_label = "Runtime [%]"
        self.title_font = {'fontname':'Calibri'}
        
        self.nr_resolutions = nr_resolutions
        self.init_plot()

    def init_plot(self):
        self.fig = plt.figure( figsize=arch_conf['figure_size'])
        self.x_min = 0
        self.x_max = 8
        self.y_min = 0
        self.y_max = 1
        self.handles = []
        self.bar_names = []
        self.axes = self.fig.add_axes([0.1,0.1,0.8,0.8])

        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 
    
    def set_title(self, title, with_peak_perf=False, peak_performance=0):
        self.title = title

    def plot_points(self, y, std=None, bottom=None, func_name='', width=0.5):
        ind = np.arange(self.nr_resolutions) + np.ones(self.nr_resolutions)
        if std is None:
            std = np.zeros(self.nr_resolutions)

        if bottom is None:
            handle = self.axes.bar(ind, y, width, yerr=std)[0]
        else:    
            handle = self.axes.bar(ind, y, width, yerr=std, bottom = bottom)[0]

        self.handles.append(handle)
        self.bar_names.append(func_name)

    def plot_graph(self, graph_name, show=True, autosave=False, img_format='svg'):  
        self.axes.legend(tuple(self.handles), tuple(self.bar_names))  
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
       
        self.fig.suptitle(graph_name, **self.title_font, fontsize=25)
        
        
        self.axes.xaxis.grid() # only showing horizontal lines
        self.axes.set_xlabel(self.x_label, fontsize=15)
        
        self.axes.set_xticks(get_resolution_in_indices())
        self.axes.set_xticklabels(get_resolutions_in_labels())
        self.axes.set_ylabel(self.y_label, fontsize=15, rotation=0, labelpad=45)
                
        if autosave:
            self.fig.savefig("stackedplot_"+graph_name.lower().replace(' ', '_') + '.' + img_format,
                        dpi=None, facecolor='w', edgecolor='w',
                        orientation='portrait', papertype=None, format=img_format,
                        transparent=False, bbox_inches=None, pad_inches=0.1,
                        frameon=None, metadata=None)
        else:
            self.fig.show()




    

