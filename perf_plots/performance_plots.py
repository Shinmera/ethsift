import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math


from architecture_config import config as arch_conf

class PerformancePlot:
    def __init__(self):
        self.pi = arch_conf['maxflops_sisd']
        self.pi_simd = arch_conf['maxflops_simd']
        self.beta = arch_conf['roofline_beta']
        self.title = "Performance "
        self.method_used = ""
        self.print_method = False
        self.x_label ="Image Resolution [pixels]"
        self.y_label = "Performance [flops/cycle]"
        self.title_font = {'fontname':'Calibri'}
        self.init_plot()

    def init_plot(self):
        self.max_performance = 0

        fig = plt.figure()
        x_offset_min = 30000
        x_offset_max = 1000*x_offset_min
        self.x_min = 427*240 - x_offset_min
        self.x_max = 7680*4320 + x_offset_max
        self.y_min = 0
        self.y_max = self.pi_simd + 0.5
        
        self.axes= fig.add_axes([0.1,0.1,0.8,0.8])

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

    def plot_pi(self, mode='all', linewidth=5):

        if(mode == "simd"):
            #Plot SIMD and SISD max performance boundary            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='r', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')
            self.axes.vlines(self.pi_simd/self.beta, self.y_min, self.pi_simd, colors='r', linewidth=linewidth, linestyles='dashed', label='Memory boundary SIMD')
        elif(mode == "sisd"):           
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors='m', linewidth=linewidth, linestyles='solid', label='Max Performance SISD')
            self.axes.vlines(self.pi/self.beta, self.y_min, self.pi, colors='m', linewidth=linewidth, linestyles='dashed', label='Memory boundary SISD')
        else:            
            #Plot SIMD and SISD max performance boundary
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors='m', linewidth=linewidth, linestyles='solid', label='Max Performance SISD')
            self.axes.vlines(self.pi/self.beta, self.y_min, self.pi, colors='m', linewidth=linewidth, linestyles='dashed', label='Memory boundary SISD')
            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='r', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')
            self.axes.vlines(self.pi_simd/self.beta, self.y_min, self.pi_simd, colors='r', linewidth=linewidth, linestyles='dashed', label='Memory boundary SIMD')

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
        if self.print_method:            
            plt.suptitle(self.title + func_name, **self.title_font, fontsize=25)
            title = "(Measured with " + self.method_used +"; Peak Performance ethSIFT: {:.2f} )".format(self.max_performance)
            plt.title(title, **self.title_font, fontsize=20)
        else:
            plt.suptitle(self.title, **self.title_font, fontsize=25)
        
        
        plt.xlabel(self.x_label, fontsize=15)
        plt.ylabel(self.y_label, fontsize=15)
        plt.show()




    

