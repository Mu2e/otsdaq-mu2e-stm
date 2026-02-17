import sys
import numpy as np
import argparse
import matplotlib.pyplot as plt
from matplotlib.font_manager import FontProperties
from scipy import optimize




def energy_predicted(pars, adc_values):
	return pars[0] + pars[1]*adc_values + pars[2]*np.power(adc_values, 2.0) + pars[3]*np.power(adc_values, 3.0) + pars[4]*np.power(adc_values, 4.0) + pars[5]*np.power(adc_values, 5.0)




def distance(adc_on_curve, energy_on_curve, adc_point, energy_point):
	distance_squared = pow(adc+adc_on_curve-adc_point, 2) + pow(energy_on_curve - energy_point, 2)
	# get sign correct if energy is lower than curve then give it a negative sign, else positive sign
	distance = np.sign(energy_point - energy_on_curve)*np.sqrt(distance_squared)
	return distance


def distances_squared(pars, adc_values_on_curve, adc_point, energy_point):
	x = np.copy(adc_values_on_curve)
	energy_values_on_curve = pars[0] + pars[1]*x + pars[2]*pow(x, 2) + pars[3]*pow(x, 3) + pars[4]*pow(x, 4) + pars[5]*pow(x, 5)
	return pow(x-adc_point, 2) + pow(energy_values_on_curve - energy_point, 2)


def closest_approach_point_on_curve_from_point(pars, adc_value, energy_value):
	f = lambda x: pars[0] + pars[1]*x + pars[2]*pow(x, 2) + pars[3]*pow(x, 3) + pars[4]*pow(x, 4) + pars[5]*pow(x, 5)


	# Minimise the distance squared
	p_x = 12
	p_y = -7
	min_f = lambda x: pow(x-adc_value, 2) + pow(f(x) - energy_value, 2)


	# Minimize
	min_res = optimize.minimize(min_f, 0) 


	return min_res.x[0], f(min_res.x[0])


def chi_squared(pars, adc, energy, adc_uncertainties, energy_uncertainties):
	e_predicted = energy_predicted(pars, adc)
	closest_approach_point_on_curve_from_point(pars, adc_value, energy_value)
	# loop over points
	chi_sq = 0.0
	for i,value in enumerate(adc):
		adc_value, adc_uncertainty = adc[i], adc_uncertainties[i]
		energy_value, energy_uncertainty = adc[i], energy_uncertainties[i]
		a, e = closest_approach_point_on_curve_from_point(pars, adc_value, energy_value)
		d = distance(a, e, adc_value, energy_value)
		chi = d/np.sqrt(adc_uncertainty*adc_uncertainty + energy_uncertainty*energy_uncertainty)
		chi_sq = chi_sq + chi*chi 
	return chi_sq




plt.rcParams['mathtext.fontset'] = 'custom'
plt.rcParams['mathtext.it'] = 'Arial:italic'
plt.rcParams['mathtext.rm'] = 'Arial'
fontFamily = 'Arial'
font = FontProperties()
font.set_family(fontFamily)
plt.style.use({"font.size": '14.0'})


plt.ion()
fig = plt.figure()
gs = fig.add_gridspec(ncols=1, nrows=2) 
ax1 = fig.add_subplot(gs[0, 0])
ax2 = fig.add_subplot(gs[1, 0])




# Do a quick test that the distance of closest approach is sensible


# Starting fit parameters: those on Claudita's fit
best_fit_pars_claudita = np.array([-0.5351,-0.5826,-2.816e-05,-2.546e-08,-1.124e-11,-1.901e-15])
adc_values = np.linspace(-1500.0, -1400.0, 100)
energy_values = energy_predicted(best_fit_pars_claudita, adc_values)
ax1.plot(adc_values, energy_values, 'r-')
adc_value = -1460
energy_value = 810
adc_value_on_curve, energy_value_on_curve = closest_approach_point_on_curve_from_point(best_fit_pars_claudita, adc_value, energy_value)
ax1.plot(adc_value, energy_value, 'o', markersize=4)
ax1.plot(adc_value_on_curve, energy_value_on_curve, 'o', markersize=4)
ax1.plot(np.array([adc_value, adc_value_on_curve]), np.array([energy_value, energy_value_on_curve]), 'b--')
ax1.set_xlabel('ADC')
ax1.set_ylabel('E [keV]')
d = distances_squared(best_fit_pars_claudita , adc_values, adc_value, energy_value)
print("Closest point = [%.2f, %.2f]" % (adc_value_on_curve, energy_value_on_curve))
for i, value in enumerate(d):
	print("distance squared to [%.2f, %.2f] = %.2f" % (adc_values[i], energy_values[i], d[i]))


input(...)


# Energies 
energy = np.array([661.659,40.1186,121.7817,244.6974,344.2785,411.1165,443.965,778.91,867.380,964.08,1085.837,1112.076,1408.013])                                                                                                                                                         
energy_uncertainty = np.array([0.003,0.0001,0.0003,0.0008,0.0012,0.0012,0.003,0.0024,0.003,0.018,0.010,0.003,0.003])


# ADC values
adc = np.array([-1161.64,-70.01,-211.75,-426.91,-602.16,-720.7,-778.7,-1368.78,-1525.7,-1695.5,-1910.53,-1956.92,-2472.68])
adc_uncertainty = np.array([0.03,0.03,0.03,0.11,0.04,0.3,0.3,0.1,0.4,0.11,0.23,0.11,0.13])


# Plot ADC vs energy
ax2.errorbar(x=adc, y=energy, xerr=adc_uncertainty, yerr=energy_uncertainty, fmt='o', capsize=5, ecolor='red',markeredgecolor='red',
             markerfacecolor='red', markersize=2)


# Add labels and title
ax2.set_xlabel('ADC')
ax2.set_ylabel('E [keV]')


#minimum = optimize.fmin(chi_squared, start, args=(adc, energy, adc_uncertainty, energy_uncertainty),full_output=1, ftol=1e-4, xtol=1e-4, disp=False)


plt.show()
input('....')
plt.close()
