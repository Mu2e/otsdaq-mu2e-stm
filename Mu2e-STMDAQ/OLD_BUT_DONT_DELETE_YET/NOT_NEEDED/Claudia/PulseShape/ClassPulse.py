import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm
import argparse, sys

#parser = argparse.ArgumentParser(description='...')
#parser.add_argument('--filename', type=str, required=True, dest='filename', help='binary filename')
#args = parser.parse_args()
#filename = args.filename

class Pulse:
    def __init__(self, num_sub_pulses, duration_ns, baseline):
        self.num_sub_pulses = num_sub_pulses
        self.duration_ns    = duration_ns
        self.baseline    = baseline
        self.amplitude      = np.zeros( (duration_ns, num_sub_pulses), dtype='float' ) # 2D array: each column is a sub-pulse and each row is separated by 1ns
        
    def add_subpulse(self, sub_pulse_index, start_time, duration, amplitude, tau_ns):
        error = 0
        if (sub_pulse_index >= self.num_sub_pulses or sub_pulse_index < 0):
            error = 1
            print("ERROR: sub_pulse_index = %d is not in range %d to %d" % (sub_pulse_index,0,self.num_sub_pulses-1))
        if (start_time >= self.duration_ns or start_time <= 0):
            error = 1
            print("ERROR: start_time = %d is not in range %d to %d" % (start_time,1,self.duration_ns))
        if (start_time + duration >= self.duration_ns or start_time + duration <= 0):
            error = 1
            print("ERROR: start_time + duration = %d is not in range %d to %d" % (start_time,1,self.duration_ns))

        if (error == 0):
            amplitude_per_ns = amplitude/duration
            for t in np.arange(start_time, start_time + duration, 1, dtype='int'):  # leading edge going negative
                self.amplitude[t][sub_pulse_index] = self.amplitude[t-1][sub_pulse_index] - amplitude_per_ns

            for t in np.arange(start_time + duration, self.duration_ns, 1, dtype='int'):  # rising exponential part
                self.amplitude[t][sub_pulse_index] = self.amplitude[t-1][sub_pulse_index]*(1-1/tau_ns)
        else:
            sys.exit()

    def aggregate_pulse(self):
        return self.baseline+np.sum(self.amplitude,axis=1)

    def aggregate_times(self):
        return np.arange(0.0,self.duration_ns,1.0)


tmax = 1000 # in ns
baseline = 1000
p = Pulse(2,tmax,baseline) # two sub-pulses
p.add_subpulse(0, 100, 200, 1000, 50000)
p.add_subpulse(1, 250, 200, 1000, 50000)

t = p.aggregate_times()
a = p.aggregate_pulse()

plt.ion()
fig = plt.figure()
gs = fig.add_gridspec(ncols=1, nrows=1) 
ax = fig.add_subplot(gs[0, 0])
ax.plot(t, a, 'r')
ax.set_xlabel('Time [ns]')

plt.tight_layout()
plt.savefig('model.png')
input('...')
plt.close()
