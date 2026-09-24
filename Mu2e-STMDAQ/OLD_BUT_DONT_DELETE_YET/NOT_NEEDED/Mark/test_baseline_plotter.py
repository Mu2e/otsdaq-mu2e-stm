import sys
import os
import Baseline as bl
from datetime import datetime
from datetime import timedelta
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdate

def plot(fig,ax,x,y,label):
    ax.scatter(x,y,label=label,marker='o', color='red', s=1.0)
    date_fmt = '%d/%m %H:%M'
    date_formatter = mdate.DateFormatter(date_fmt)
    fig.autofmt_xdate()
    ax.xaxis.set_major_formatter(date_formatter)
    ax.set_ylabel(label,fontsize=12)


bll = bl.Baseline()

WRITE = 0
if (WRITE == 1):
    start_time = datetime(2022, 4, 10, 12, 1, 1)
    end_time = datetime(2022, 4, 10, 12, 1, 1)
    nsubruns = 10
    for run in range(100):
        run = run + 1
        for subrun in range(nsubruns):
            start_time = end_time + timedelta(seconds=1)
            end_time   = start_time + timedelta(minutes=1)
            bll.write_run_subrun(run,subrun,start_time,end_time)
            mean = np.random.normal(100, 10)
            rms = np.abs(np.random.normal(10, 1))
            bll.write_data(run,subrun,mean,rms)
            start_time = start_time + timedelta(minutes=15)


fig = plt.figure()
gs = fig.add_gridspec(ncols=1, nrows=2) 
old_epoch = '0000-12-31T00:00:00'
new_epoch = '1970-01-01T00:00:00'
mdate.set_epoch(old_epoch) 

means,rmss,times = bll.get_data_run() # no argument all runs, else argument is run
#print(means,rmss,times)
tt = mdate.date2num(list(times))
ax1 = fig.add_subplot(gs[0, 0])
plot(fig,ax1,tt,means,'Baseline Mean')
ax2 = fig.add_subplot(gs[1, 0])
plot(fig,ax2,tt,rmss,'Baseline RMS')

plt.tight_layout()
plt.savefig('test_baseline_plotter.png')

bll.close();
