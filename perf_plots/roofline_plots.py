

class RooflinePlot:
  
  def __init__(self, beta, meas_method):
    self.title = "Roofline "
    self.x_label ="Operational Intensity [flops/bytes]"
    if meas_method == 'rdtsc':
        self.y_label = "Performance [flops/cycle]"
    else:
        self.y_label = "Performance [flops/\u03BCs]"

    self.title_font = {'fontname':'Calibri'}
    self.beta = beta
    self.init_plot()

  def init_plot(self):
    self.fig = plt.figure( figsize=arch_conf['figure_size'])
    self.x_min = 0
    self.x_max = 16
    self.y_min = 0
    self.y_max = 20
    self.handles = []
    self.bar_names = []
    self.axes = self.fig.add_axes([0.1,0.1,0.8,0.8])

    self.axes.set_xlim([self.x_min,self.x_max])
    self.axes.set_ylim([self.y_min,self.y_max]) 
    self.axes.set_xscale("log", basex=2)
    self.axes.ax.set_yscale("log", basey=2)
