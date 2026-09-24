import math
import numpy as np
from numpy import *     
from scipy.optimize import curve_fit
from scipy.stats import norm
from lmfit.models import StepModel, LinearModel, ConstantModel
import matplotlib.pyplot as plt

# Truncate edge of string
def truncate_from_last(string, char):
    last_index = string.rfind(char)
    if last_index != -1:
        return string[last_index+1:]
    return string

# Twos compliment
def twos_comp(n, w):
    if n & (1 << (w - 1)): n = n - (1 << w)
    return n

# Rising edge function
def rising_edge(t, offset,amplitude, rise_time, t0):
    return offset + amplitude * (1 - np.exp(-(t - t0) / rise_time)) * (t >= t0)

# Fit the rising edge
def fit_rising_edge(x,y,pulse_time):

    # Function guess values
    offset_guess = min(y) # offset
    amp_guess = max(y) # amplitude
    # Find the middle of the pulse (same as middle of data)
    middle = int((len(x)-1)/2)
    t0_guess = x[middle]
    rise_time_guess = 4*1e-9 # rising edge rise time
    
    # # Find the middle of the pulse (same as middle of data)
    # middle = int(len(x)/2)
    # # x values to middle 
    # fit_x = x[0:middle]
    # # y values to middle 
    # fit_y = y[0:middle]

    # Initial guesses for the paramater
    p0 = [offset_guess,amp_guess, rise_time_guess, t0_guess]

    # Fit the rising edge
    popt, _ = curve_fit(rising_edge, x , y, p0)
    offset,amplitude, rise_time, t0 = popt

    # Calculate rising edge
    rising_edge_fit = (t0+rise_time)

    # Return rising edge and fit parameters
    return rising_edge_fit,popt
