import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import math

from os import listdir
from os.path import isfile, join


class RooflinePlot:
    def __init__(self, pi, pi_simd, beta):
        self.pi = pi
        self.pi_simd = pi_simd
        self.beta = beta
        self.x_loglim = 3
        self.title = "Roofline plot"
        self.method_used = ""
        self.print_method = False
        self.x_label = "I(n) [flops/byte]"
        self.y_label = "P(n) [flops/cycle]"
        self.title_font = {'fontname':'Calibri'}
        self.init_plot()

    def set_method_used(self, method_used):
        self.method_used = method_used
    
    def set_title(self, title):
        self.title = title

    def set_x_loglim(self, x_loglim):
        self.x_loglim = x_loglim

    def plot_performance_bound(self, bound, color, bound_label=''):        
        self.axes.hlines(bound, self.x_min, self.x_max, colors=color, linestyles='solid', label=bound_label)

    def plot_pi(self, mode='all'):
        if(mode == "simd"):
            #Plot SIMD and SISD max performance boundary            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='r', linestyles='solid', label='Max Performance SIMD')
            self.axes.vlines(self.pi_simd/self.beta, self.y_min, self.pi_simd, colors='r', linestyles='dashed', label='Memory boundary SIMD')
        elif(mode == "sisd"):           
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors='m', linestyles='solid', label='Max Performance SISD')
            self.axes.vlines(self.pi/self.beta, self.y_min, self.pi, colors='m', linestyles='dashed', label='Memory boundary SISD')
        else:            
            #Plot SIMD and SISD max performance boundary
            self.axes.hlines(self.pi, self.x_min, self.x_max, colors='m', linestyles='solid', label='Max Performance SISD')
            self.axes.vlines(self.pi/self.beta, self.y_min, self.pi, colors='m', linestyles='dashed', label='Memory boundary SISD')
            
            self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='r', linestyles='solid', label='Max Performance SIMD')
            self.axes.vlines(self.pi_simd/self.beta, self.y_min, self.pi_simd, colors='r', linestyles='dashed', label='Memory boundary SIMD')
    

    def init_plot(self):
        self.max_performance = 0

        fig = plt.figure()
        r = math.ceil(math.log2(self.pi_simd))
        self.x_min = pow(2, -self.x_loglim)
        self.x_max = pow(2, self.x_loglim)
        self.y_min = pow(2, -1)
        self.y_max = pow(2, r)
        
        self.axes= fig.add_axes([0.1,0.1,0.8,0.8])

        # Plot memory bound
        plt_mesh = '-b'
        x_axis = [self.x_min, self.x_max]
        y_axis = [x * self.beta for x in x_axis]
        self.axes.plot(x_axis, y_axis, plt_mesh, label="Roofline")
        
        self.axes.set_xlim([self.x_min,self.x_max])
        self.axes.set_ylim([self.y_min,self.y_max]) 
    
    def plot_point(self, x, y, marker, color='c', point_label='', linewidth=2, markersize=8):
        self.axes.plot(x, y, color=color, marker=marker, linestyle='dashed', linewidth=linewidth, markersize=markersize, label=point_label)

    def plot_roofline(self):  
        self.axes.legend()  
        self.axes.set_xscale('log', basex=2)
        self.axes.set_yscale('log', basey=2)
        self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
        
        plt.suptitle(self.title, **self.title_font, fontsize=25)
        if self.print_method:
            title = "(Based on" + self.method_used +" Function, Peak Performance: {:.2f} )".format(self.max_performance)
            plt.title(title, **self.title_font, fontsize=20)
        
        plt.xlabel(self.x_label, fontsize=15)
        plt.ylabel(self.y_label, fontsize=15)
        plt.show()

        print("Should have printed")



def subtask3a():
    #Task 3.a)
    rplt = RooflinePlot(5, 20, 32)
    rplt.plot_pi()
    rplt.plot_roofline()

def subtask3b():
    #Task 3.b)
    rplt = RooflinePlot(5, 20, 32)
    rplt.set_title("Roofline 3.b)")
    rplt.plot_point(0.21, 2, 'o', '#208020', 'Computation 1', markersize=10)
    rplt.plot_point(0.21, 2, 'o', '#e0e050', 'Computation 2', markersize=5)
    rplt.plot_point(0.21, 3, 'o', '#701010', 'Computation 3', markersize=10)
    rplt.plot_pi('sisd')
    rplt.plot_roofline()

def subtask3c():
    #Task 3.c)
    rplt = RooflinePlot(5, 20, 32)
    rplt.set_title("Roofline 3.c)")
    rplt.plot_point(0.21, 8, 'o', '#207020', 'Computation 1', markersize=10)
    rplt.plot_point(0.21, 8, 'o', '#e0e070', 'Computation 2', markersize=5)
    rplt.plot_point(0.21, 12, 'o', '#701030', 'Computation 3', markersize=10)
    rplt.plot_pi('simd')
    rplt.plot_roofline()

def subtask3d():
    #Task 3.c)
    rplt = RooflinePlot(5, 20, 32)  
    rplt.set_title("Roofline 3.d)") 
    rplt.plot_point(0.21, 3, 'o', '#207020', 'Computation 1', markersize=10)
    rplt.plot_point(0.21, 2, 'o', '#e0e070', 'Computation 2', markersize=10)
    rplt.plot_point(0.21, 4, 'o', '#701030', 'Computation 3', markersize=10)
    rplt.plot_pi('sisd')
    rplt.plot_roofline()

def plot_all():
    # Plot all subtasks
    rplt = RooflinePlot(5, 20, 32)
    rplt.plot_point(0.21, 2, '*', '#208020', 'Computation 1 SISD', markersize=7)
    rplt.plot_point(0.21, 2, '*', '#e0e050', 'Computation 2 SISD', markersize=8)
    rplt.plot_point(0.21, 3, '*', '#701010', 'Computation 3 SISD', markersize=7)

    rplt.plot_point(0.21, 8, 'o', '#207020', 'Computation 1 SIMD', markersize=10)
    rplt.plot_point(0.21, 8, 'o', '#e0e070', 'Computation 2 SIMD', markersize=10)
    rplt.plot_point(0.21, 12, 'o', '#701030', 'Computation 3 SIMD', markersize=10)
    
    rplt.plot_point(0.21, 3, 'x', '#207020', 'Computation 1 SISD', markersize=10)
    rplt.plot_point(0.21, 2, 'x', '#e0e070', 'Computation 2 SISD', markersize=10)
    rplt.plot_point(0.21, 4, 'x', '#701030', 'Computation 3 SISD', markersize=10)
    rplt.plot_pi()
    rplt.plot_roofline()

subtask3b()
subtask3c()
subtask3d()