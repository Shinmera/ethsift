import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

from read_logs import get_resolutions_in_pixels, get_resolutions_in_labels

from architecture_config import config as arch_conf

class PerformancePlot:
    def __init__(self):
        self.pi = arch_conf['maxflops_sisd']
        self.pi_simd = arch_conf['maxflops_simd_fma']
        self.pi_fma = arch_conf['maxflops_sisd']
        self.pi_simd_fma = arch_conf['maxflops_simd_fma']
        self.beta = arch_conf['roofline_beta']
        self.title = "Performance "
        self.method_used = ""
        self.print_method = False
        self.x_label ="Image Resolution \n [pixels]"
        self.y_label = "Performance \n [flops/cycle]"
        self.title_font = {'fontname':'sans serif'}
        self.init_plot()

    def init_plot(self):
        self.max_performance = 0

        self.fig = plt.figure( figsize=arch_conf['figure_size'])
        x_offset_min = 30000
        x_offset_max = 1000*x_offset_min
        self.x_min = 427*240 - x_offset_min
        self.x_max = 7680*4320 + x_offset_max
        self.y_min = 1/32
        self.y_max = self.pi_simd_fma + 0.5
        
        self.axes= self.fig.add_axes([0.1,0.1,0.8,0.8])

        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 

    def set_method_used(self, method_used):
        self.method_used = method_used
        self.print_method = True
    
    def set_title(self, title, with_peak_perf=False, peak_performance=0):
        self.title = title

    def plot_memory_bound(self):        
        # Plot memory bound
        plt_mesh = '-b'
        x_axis = [self.x_min, self.x_max]
        y_axis = [x * self.beta for x in x_axis]
        self.axes.plot(x_axis, y_axis, plt_mesh, label="Roofline")
        

    def plot_performance_bound(self, bound, color, bound_label=''):        
        self.axes.hlines(bound, self.x_min, self.x_max, colors=color, linestyles='solid', label=bound_label)

    def plot_pi(self, mode='all', linewidth=1):

        if(mode == "sisd_fma"):
            #Plot SIMD and SISD max performance boundary            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='#7b0323', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')
        elif(mode == "sisd"):           
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors="#c44240", linewidth=linewidth, linestyles='solid', label='Max Performance SISD')
        else:            
            #Plot SIMD and SISD max performance boundary
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors="#c44240", linewidth=linewidth, linestyles='solid', label='Max Performance SISD')            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='#7b0323', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')
            
            #Plot SIMD_fma and SISD_fma max performance boundary
            self.axes.hlines(self.pi_fma, self.x_min, self.x_max, colors="#c44240", linewidth=linewidth, linestyles='solid', label='Max Performance SISD')            
            self.axes.hlines(self.pi_simd_fma, self.x_min, self.x_max, colors='#7b0323', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')

    def plot_points(self, x, y, marker, color='c', linestyle='dashed', point_label='', linewidth=2, capsize=10, markersize=8, error=None):
        if(error is None):
            self.axes.plot(x, y, color=color, marker=marker, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)
        else:
            self.axes.errorbar(x, y, yerr=error, color=color, marker=marker, capsize=capsize, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)

    def set_peak_performance(self, perf):
        self.max_performance = perf

    def plot_graph(self, func_name, autosave=False, img_format='svg'):  
        self.axes.legend()  
        self.axes.set_xscale('log', basex=2)
        self.axes.set_yscale('log', basey=2)
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        if self.print_method:            
            self.fig.suptitle(self.title + func_name, **self.title_font, fontsize=25)
            title = "(Measured with " + self.method_used +"; Peak Performance ethSIFT: {:.2f} )".format(self.max_performance)
            self.axes.set_title(title, **self.title_font, fontsize=20)
        else:
            self.fig.suptitle(self.title, **self.title_font, fontsize=25)
        
        plt.rcParams['axes.facecolor'] = 'xkcd:light grey'
        self.axes.grid(color='w', linestyle='-', linewidth=0.5)
        self.axes.xaxis.grid() # only showing horizontal lines
        self.axes.set_xlabel(self.x_label, fontsize=15)
        
        self.axes.set_xticks(get_resolutions_in_pixels())
        self.axes.set_xticklabels(get_resolutions_in_labels())
        self.axes.set_ylabel(self.y_label, fontsize=15, rotation=0, labelpad=45)
        
        if autosave:
            plt.savefig("perfplot_"+func_name.lower().replace(' ', '_') + '.' + img_format,
                        dpi=None, facecolor='w', edgecolor='w',
                        orientation='portrait', papertype=None, format=img_format,
                        transparent=False, bbox_inches=None, pad_inches=0.1,
                        frameon=None, metadata=None)
        else:
            plt.show()



    

