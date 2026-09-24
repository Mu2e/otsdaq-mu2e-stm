import numpy as np
import matplotlib.pyplot as plt
from astropy.visualization import hist

# generate some complicated data
rng = np.random.default_rng(0)
t = np.concatenate([-5 + 1.8 * rng.standard_cauchy(500),
                    -4 + 0.8 * rng.standard_cauchy(2000),
                    -1 + 0.3 * rng.standard_cauchy(500),
                    2 + 0.8 * rng.standard_cauchy(1000),
                    4 + 1.5 * rng.standard_cauchy(1000)])

# truncate to a reasonable range
t = t[(t > -15) & (t < 15)]

# draw histograms with two different bin widths
fig, ax = plt.subplots(1, 2, figsize=(10, 4))

fig.subplots_adjust(left=0.1, right=0.95, bottom=0.15)
for i, bins in enumerate(['scott', 'freedman']):
    hist(t, bins=bins, ax=ax[i], histtype='stepfilled',
         alpha=0.2, density=True)
    ax[i].set_xlabel('t')
    ax[i].set_ylabel('P(t)')
    ax[i].set_title(f'hist(t, bins="{bins}")',
                    fontdict=dict(family='monospace'))
