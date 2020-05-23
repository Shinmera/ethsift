import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

from architecture_config import config as arch_conf

from read_logs import get_resolutions_in_pixels, get_resolutions_in_labels

class RooflinePlot:
  
  def __init__(self):
    self.title = "Roofline "

    self.x_label ="Operational Intensity [flops/bytes]"
    self.y_label = "Performance\n[flops/cycle]"
    
    self.pi = arch_conf['maxflops_sisd']
    self.pi_simd = arch_conf['maxflops_simd']
    self.pi_sisd_fma = arch_conf['maxflops_sisd_fma']
    self.pi_simd_fma = arch_conf['maxflops_simd_fma']
    self.beta = arch_conf['roofline_beta']

    self.title_font = {'fontname':'Calibri'}
    self.init_plot()


  def init_plot(self):
    self.fig = plt.figure( figsize=(20,9))
    self.x_min = 1/66
    self.x_max = 18
    self.y_min = 1/16
    self.y_max = self.pi_simd_fma + 4
    self.handles = []
    self.bar_names = []
    self.axes = self.fig.add_axes([0.1,0.1,0.8,0.8])

    self.axes.set_xlim([self.x_min,self.x_max])
    self.axes.set_ylim([self.y_min,self.y_max]) 


  def plot_bounds(self, linewidth = 1):
    self.axes.hlines(self.pi, self.x_min, self.x_max, colors="#c44240", linewidth=linewidth, linestyles='solid', label='Max Performance SISD')
    self.axes.vlines(self.pi/self.beta, self.y_min, self.pi, colors="#c44240", linewidth=linewidth, linestyles='dashed', label='Memory boundary SISD')
    
    self.axes.hlines(self.pi_simd, self.x_min, self.x_max, colors='#7b0323', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD')
    self.axes.vlines(self.pi_simd/self.beta, self.y_min, self.pi_simd, colors='#7b0323', linewidth=linewidth, linestyles='dashed', label='Memory boundary SIMD')
 
    self.axes.hlines(self.pi_sisd_fma, self.x_min, self.x_max, colors='#009900', linewidth=linewidth, linestyles='solid', label='Max Performance SISD with FMA')
    self.axes.vlines(self.pi_sisd_fma/self.beta, self.y_min, self.pi_sisd_fma, colors='#009900', linewidth=linewidth, linestyles='dashed', label='Memory boundary SISD with FMA')

    self.axes.hlines(self.pi_simd_fma, self.x_min, self.x_max, colors='#559900', linewidth=linewidth, linestyles='solid', label='Max Performance SIMD with FMA')
    self.axes.vlines(self.pi_simd_fma/self.beta, self.y_min, self.pi_simd_fma, colors='#559900', linewidth=linewidth, linestyles='dashed', label='Memory boundary SIMD with FMA')

    point1 = [1/8, self.beta * 1/8]
    point2 = [4.0, self.beta * 4.0]
    self.plot_line(point1, point2, 'Memory Bound')


  def plot_line(self, point1, point2, label):
    xmin, xmax = self.axes.get_xbound()

    if(point2[0] == point1[0]):
      xmin = xmax = point1[0]
      ymin, ymax = self.axes.get_ybound()
    else:
      ymax = point1[1]+(point2[1]-point1[1])/(point2[0]-point1[0])*(xmax-point1[0])
      ymin = point1[1]+(point2[1]-point1[1])/(point2[0]-point1[0])*(xmin-point1[0])

    l = mpl.lines.Line2D([xmin,xmax], [ymin,ymax], label=label)
    self.axes.add_line(l)
    return l


  def plot_points(self, x, y, marker, res='something', color='c', linestyle='dashed', point_label='', linewidth=2, capsize=10, markersize=8):
    self.axes.plot(x, y, color=color, marker=marker, linestyle=linestyle, linewidth=linewidth, markersize=markersize, label=point_label)
    
    labels = get_resolutions_in_labels()
    for i in range(len(x)):
      self.axes.annotate('{}'.format(labels[i]), xy = (x[i], y[i]), color=color)


  def plot_graph(self, func_name, autosave=False, img_format='svg'):  
    self.axes.legend()  
    
    self.axes.set_xscale("log", basex=2)
    self.axes.set_yscale("log", basey=2)
    
    self.axes.xaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
    self.axes.yaxis.set_major_formatter(mpl.ticker.FuncFormatter(lambda x, _: '{:g}'.format(x)))
    
    self.fig.suptitle(self.title + func_name, **self.title_font, fontsize=25)
    
    plt.rcParams['axes.facecolor'] = 'xkcd:light grey'
    self.axes.grid(color='w', linestyle='-', linewidth=0.5)
    self.axes.xaxis.grid() # only showing horizontal lines
    
    self.axes.set_xlabel(self.x_label, fontsize=15)
    self.axes.set_ylabel(self.y_label, fontsize=15, rotation=0, labelpad=45)

    if autosave:
      self.fig.savefig("rooflineplot_"+func_name.lower().replace(' ', '_') + '.' + img_format,
                  dpi=None, facecolor='w', edgecolor='w',
                  orientation='portrait', papertype=None, format=img_format,
                  transparent=False, bbox_inches=None, pad_inches=0.1,
                  frameon=None, metadata=None)
    else:
      self.fig.show()
