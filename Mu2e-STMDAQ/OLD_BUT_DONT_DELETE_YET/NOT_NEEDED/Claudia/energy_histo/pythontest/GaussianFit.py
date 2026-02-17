import numpy as np
from scipy.stats import norm
import matplotlib.pyplot as plt
import matplotlib

backends = matplotlib.rcsetup.interactive_bk
print(backends)
for b in backends:
    print(b)
    try:
        plt.switch.backend(b)
        print("Supported backend = ",b)
    except:
        print("NOT Supported backend = ",b)
        continue



# Generate some data for this demonstration.
data = norm.rvs(10.0, 2.5, size=500)

# Fit a normal distribution to the data:
mu, std = norm.fit(data)

# Plot the histogram.
plt.ion()
plt.hist(data, bins=25, density=True, alpha=0.6, color='g')

# Plot the PDF.
xmin, xmax = plt.xlim()
x = np.linspace(xmin, xmax, 100)
p = norm.pdf(x, mu, std)
plt.plot(x, p, 'k', linewidth=2)
title = "Fit results: mu = %.2f,  std = %.2f" % (mu, std)
plt.title(title)

plt.show()
