import os
import re

from astropy.io import ascii

import plot_krf_lc
import krf_rebin

def hhmmss_to_sod(str_hhmmss):

    m = re.search(r'(\d+):(\d+):(\d+(\.\d+)?)', str_hhmmss)
    sod = int(m.group(1))*3600.0 + int(m.group(2))*60.0 + float(m.group(3))
    return sod 

def rebin(file_name, file_name_rebin, res_ms):

    thr_data = krf_rebin.read_thr(file_name)
    thr_data_rebin = krf_rebin.rebin_data(thr_data, res_ms)
    krf_rebin.write_thr(thr_data_rebin, file_name_rebin)

def main():

    tab = ascii.read('krf_grbs_for_test.txt', fill_values=[('--', '0',)])
   
    #print(tab)

    #временное разрешение в милисекундах для создания данных с загрубленным временным разрешением
    res_ms = 64

    for i in range(len(tab)):
    #for i in range(3):

        #if tab['TrigNumber'].mask[i]:
        #    continue

        lst_det = tab['Det'][i].split(',')
        lst_time = tab['Time'][i].split(',')
        lst_trig_time = tab['TrigTime'][i].split(',')

        for det, time, trig_time in zip(lst_det, lst_time, lst_trig_time):

            sod = hhmmss_to_sod(time)
            sod_trig = hhmmss_to_sod(trig_time)

            # исходный файл данных
            lc_file = 'data/krf20{:06d}_{:d}_1_{:s}.thr'.format(tab['Date'][i], int(sod), det)

            # файл данных с загрубленным временным разрешением 
            lc_file_reb = 'data_reb_{:d}/krf20{:06d}_{:d}_1_{:s}_{:d}ms.thr'.format(res_ms, tab['Date'][i], int(sod), det, res_ms)

            # функция для создания данных с загрубленным временным разрешением
            rebin(lc_file, lc_file_reb, res_ms)
            
            # временной интервал для отображения данных 
            t_i, t_f = -2, 50
            
            # временной интервал для оценки фона для рисунка
            t_i_bg, t_f_bg = 20, 50

            str_title = "Konus-RF {:s} bursts\n20{:06d} T$_0$={:.3f} s UT ({:s})".format(det, tab['Date'][i], sod_trig, trig_time)

            fig_name = os.path.basename(lc_file_reb)
            fig_name = "fig_{:d}/{:s}.png".format(res_ms, fig_name.split('.')[0])

            #if os.path.isfile(fig_name):
            #    continue
            
            # функция для отображения данных
            plot_krf_lc.plot_krf_lc(lc_file_reb, t_i, t_f, t_i_bg, t_f_bg, str_title, fig_name)
            
            
        
if __name__ == '__main__':
    main()