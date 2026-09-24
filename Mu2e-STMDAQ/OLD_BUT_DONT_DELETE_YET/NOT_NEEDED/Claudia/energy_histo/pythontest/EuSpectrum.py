import numpy as np
import matplotlib
from matplotlib import pyplot as plt
from scipy.stats import norm
from astropy.visualization import hist
import csv

data = []
filename = 'Europium_ADCpeaks.csv'

with open(filename, 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        data.append(row)

#draw histograms with two different bin widths
#plt.subplot(2, 1, 1) # (rows, columns, panel number)
## ax will be an array of two Axes objects
fig, ax = plt.subplots(1, 2, figsize=(10, 4))

fig.subplots_adjust(left=0.1, right=0.95, bottom=0.15)
for i, bins in enumerate(['scott', 'freedman']):
    hist(data, bins=bins, ax=ax[i], histtype='stepfilled',alpha=0.2, density=True)
    ax[i].set_xlabel('ADC Counts')
    ax[i].set_ylabel('')
    #ax[i].set_xlim([-3000, 0])
    #ax[i].set_ylim([0, 800])
    ax[i].set_title(f'hist(t, bins="{bins}")',fontdict=dict(family='monospace'))


#data = norm.rvs(10.0, 2.5, size=500)
plt.ion()
#plt.hist(data, bins=1000, density=True, alpha=0.6, color='g')
#plt.xlim(-3000, 0)
#plt.ylim(0, 800);
#plt.show()
plt.savefig('blah.png',dpi=400)



