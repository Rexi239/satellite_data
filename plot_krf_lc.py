import os
import sys
import re
import numpy as np

import matplotlib as mpl
mpl.use('Agg')

import matplotlib.pyplot as pl
from matplotlib.ticker import  MultipleLocator 

mpl.rcParams['xtick.direction'] = 'in'
mpl.rcParams['ytick.direction'] = 'in'
 
mpl.rcParams['xtick.top'] = True
mpl.rcParams['ytick.right'] = True 
mpl.rcParams['xtick.labelsize'] = 12
mpl.rcParams['ytick.labelsize'] = 12
mpl.rcParams['axes.facecolor'] = 'w'  
mpl.rcParams['axes.edgecolor'] = 'k' 

mpl.rcParams['axes.labelsize'] = 14
mpl.rcParams['axes.titlesize'] = 18

mpl.rcParams['xtick.major.size'] = 8
mpl.rcParams['xtick.minor.size'] = 4

mpl.rcParams['figure.figsize'] = (8.27, 11.69) #A4

mpl.rcParams['font.family'] = 'serif'
mpl.rcParams['font.serif'] = 'Arial'


def sod2hhmmss(seconds, use_codes=False):
    hours = int(seconds / 3600)
    seconds -= 3600.0 * hours
    minutes = int(seconds / 60.0)
    seconds -= int(60.0 * minutes)

    if use_codes:
        return "{:02d}%3A{:02d}%3A{:06.3f}".format(hours, minutes, seconds)
    else:
        return "{:02d}:{:02d}:{:06.3f}".format(hours, minutes, seconds)

class Plot:

    def __init__(self, str_title, n_panels):

        self.fig = pl.figure()
        
        self.n_panels = n_panels
        self.lst_axis = []

        self._set_layout()

        self.fig.suptitle(str_title, fontsize=18)
        

    def _set_layout(self,):

        lst_bands="""\
            1     8     10
            2    10     16 
            3    16     25 
            4    25     40 
            5    40     64 
            6    64    100
            7   100    160
            8   160    250
            9   250    400 
            10  400    640 
            11  640   1000 
            12 1000   10000""".split('\n')

        dic_bands = {int(s.split()[0]): "{:s}-{:s} keV".format(s.split()[1], s.split()[2]) for s in lst_bands}
        dic_bands[12] = ">1000 keV"

        # Set panel parameters
        left, width = 0.10, 0.8
        heigt_channels = 0.07
        heigt_tot = 0.85
        
        left_ch_names = 0.8
        
        # rect [left, bottom, width, height] 
        bottom = heigt_tot - self.n_panels * heigt_channels
        
        lst_rect = []
        for i in range(self.n_panels):
            lst_rect.append([left, bottom + (self.n_panels - i) * heigt_channels, width, heigt_channels])

        for i in range(len(lst_rect)):
            if i > 0:
                self.lst_axis.append(self.fig.add_axes(lst_rect[i],  sharex=self.lst_axis[0]))
            else:
                self.lst_axis.append(self.fig.add_axes(lst_rect[i]))

            self.lst_axis[-1].text(0.9, 0.9, dic_bands[i+1], 
                horizontalalignment='right', verticalalignment='top', transform=self.lst_axis[-1].transAxes, fontsize=14)

    def get_delta_y(self, y_min, y_max):

        dy = y_max - y_min

        lst_num = [5, 10, 20,]

        delta_y = lst_num[0]
        n_ticks = y_max / delta_y

        i = 0
        while(n_ticks > 4):
            for n in lst_num:
               delta_y = int(n * 10**i)
               n_ticks = dy // delta_y + 2
               if n_ticks <=4:
                   break
            i = i + 1

        return delta_y

    def set_x_minor_ticks(self):

        lst_ax[1].set_ylabel(str_label)
        if lst_ax[-1]:
            lst_ax[-1].set_xlabel(r'T-T$_{0}$ (s)')
        else:
            lst_ax[-2].set_xlabel(r'T-T$_{0}$ (s)')

        #ax1.set_xlim(dic_x_ticks[scale_ms][0], dic_x_ticks[scale_ms][-1])
        #ax1.set_xticks(dic_x_ticks[scale_ms])
        ax1.xaxis.set_minor_locator(minorLocator_x)
        ax1.yaxis.set_minor_locator(minorLocator_y_sum)  # y minor ticks
        ax1.set_xlim(arr_begin_end[0], arr_begin_end[1])

    def add_plot(self, 
        idx_panel,
        arr_ti, 
        arr_counts, 
        bg_level,  
        arr_begin_end, 
        str_y_label
    ):
        ax = self.lst_axis[idx_panel]
        ax.plot(arr_ti, arr_counts, drawstyle='steps-post',color='k', linewidth=0.5)
        ax.axhline(bg_level, color='k', linestyle ='--', linewidth=0.5)

        arr_bool = np.logical_and(arr_ti >= arr_begin_end[0], arr_ti <= arr_begin_end[1])

        cnt_max = int(max(arr_counts[arr_bool]))
        cnt_min = int(min(arr_counts[arr_bool]))
        
        delta_y = self.get_delta_y(cnt_min, cnt_max)

        if idx_panel > 0:
            y_max = cnt_max // delta_y * delta_y + 3/2 * delta_y
        else:
            y_max = cnt_max // delta_y * delta_y + delta_y

        y_min = cnt_min // delta_y * delta_y
    
    
        ax.set_yticks(np.arange(y_min, max(arr_counts) + delta_y, delta_y))
        ax.set_ylim(y_min, y_max)
        ax.set_xlim(arr_begin_end[0], arr_begin_end[1])
        
        ax.set_ylabel(str_y_label, fontsize=14)

        if idx_panel < len(self.lst_axis) -1:
            pl.setp(ax.get_xticklabels(), visible=False)

    def save(self, file_name):

        self.lst_axis[-1].set_xlabel(r'T-T$_{0}$ (s)')

        pl.savefig(file_name, format='png', dpi=100)
        pl.close()

def plot_krf_lc(
    file_name, 
    t_i, t_f, 
    t_i_bg, t_f_bg, 
    str_title, 
    fig_name,
    show=False):

    path, file_name_ = os.path.split(file_name)


    x_range = np.array([t_i, t_f])
    dt  = x_range[1] - x_range[0]

    lc_data = np.loadtxt(file_name)


    plot = Plot(str_title, 12)

    res = 8
    arr_bool = np.logical_and(lc_data[:,0] > t_i_bg, lc_data[:,0] <t_f_bg)

    for i in range(1, 13):

        #str_y_label = "cnts / {:.0f} ms".format(res)
        str_y_label = ""

        bg_cnt = np.mean(lc_data[arr_bool,i+1])
        plot.add_plot(i-1, lc_data[:,0],  lc_data[:,i+1], bg_cnt, x_range, str_y_label)

    if show:
        pl.show()

    plot.save(fig_name)
    
def main():

    lc_file = 'krf20090605_74448_1_S1_8ms.thr'
    t_i, t_f = -1, 1
    t_i_bg, t_f_bg = -2, -1
    
    #str_title = "{:s} GRB {:s}\nT$_0$={:.3f} s UT ({:s})".format('Konus-RF', lc.date, lc.time, sod2hhmmss(lc.time))
    str_title = "{:s} bursts".format('Konus-RF')

    plot_krf_lc(lc_file, t_i, t_f, t_i_bg, t_f_bg, str_title, 'test.png')

if __name__ == "__main__":
    main()

