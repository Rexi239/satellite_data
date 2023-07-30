"""
THR format

Temporal resolution
 Ti  dT  res
  s   s   ms
 -2   1   2  
  1  16   8 
 17  32  32 
 49  64  64 

Energy chammels 

Range 1 channels (Tab 1, 2 in krf-to.pdf)
N    Ei     Ef
    keV    keV
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
12 1000     -- 

Range 1 channels
N     Ei    Ef
     keV   keV
1      8   280    
2    280   400    
3    400   640    
4    640  1000   
5   1000  1600   
6   1600  2500   
7   2500  4000   
8   4000  6400   
9   6400 10000  
10 10000    --  

"""

import os
import sys
from pprint import pprint

import numpy as np

def read_thr(file_name):

    thr_data = np.loadtxt(file_name)

    return thr_data

def rebin_data(thr_data, res_ms):

    arr_counts = np.zeros((thr_data.shape[0]+1, thr_data.shape[1]-2))
    arr_counts[1:,:] = thr_data[:,2:]
    arr_counts_cum = np.cumsum(arr_counts, axis=0)

    arr_ti_tf_ms = np.zeros((thr_data.shape[0]+1, 2), dtype=int)
    arr_ti_tf_ms[1:,:] = np.rint(thr_data[:,:2]*1000).astype(int)
    arr_ti_tf_ms[0,0] = -1000
    arr_ti_tf_ms[0,1] = arr_ti_tf_ms[1,0] 
    

    #pprint(arr_ti_tf_ms[:17])
    #pprint(arr_counts_cum[:17,:])

    lst_data = []

    # при любом разрешении 2, 4б 8, 16, 32, 64, 256 мс
    # граница бина должна попадать на 1.000

    i_start = (3000 % res_ms) // 2 + 1
    i_prev = i_start - 1

    #print(i_start)
    #sys.exit()

    for i in range(i_start, arr_ti_tf_ms.shape[0]):

        dt = arr_ti_tf_ms[i,1] - arr_ti_tf_ms[i,0]
        
        if dt > res_ms:
            break

        if (arr_ti_tf_ms[i,1] - arr_ti_tf_ms[i_start,0]) % res_ms == 0:

            arr_counts = arr_counts_cum[i,:] - arr_counts_cum[i_prev,:] 
            lst_data.append([arr_ti_tf_ms[i_prev,1], arr_ti_tf_ms[i,1]] + arr_counts.tolist())
            i_prev = i

    #pprint(lst_data[:16])

    thr_data_rebin = np.array(lst_data, dtype=float)
    thr_data_rebin[:,:2] = thr_data_rebin[:,:2]/1000
    return thr_data_rebin

def write_thr(thr_data, file_name):

    with open(file_name, 'w') as f:
        for i in range(thr_data.shape[0]):
            for j in range(2):
                f.write("{:8.3f} ".format(thr_data[i,j]))
            for j in range(2, thr_data.shape[1]):
                f.write("{:5.0f} ".format(thr_data[i,j]))
            f.write('\n')

def main():

    file_name = './thr/krf20090227_26757_1_S1.thr'
    res_ms = 256

    name = os.path.basename(file_name)
    name = name.split('.')[0]
    file_name_rebin = f'{name}_{res_ms}ms.thr'

    thr_data = read_thr(file_name)
    thr_data_rebin = rebin_data(thr_data, res_ms)
    write_thr(thr_data_rebin, file_name_rebin)

if __name__ == '__main__':
    main()
