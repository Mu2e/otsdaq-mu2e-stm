import sys
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path
from scipy.stats import norm
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec


def _overlay_gauss_exp_on_axis(ax, bin_centers, counts, mu0, sigma0, t, lam, w0=None, w1=None,
                                label_prefix="", draw_hist=True):
    # Convert inputs
    bin_centers = np.asarray(bin_centers)
    counts = np.asarray(counts)
    assert bin_centers.shape == counts.shape

    # Derive a stable bin width (assume ~uniform)
    if bin_centers.size > 1:
        widths = np.diff(np.r_[bin_centers[0] - (bin_centers[1]-bin_centers[0]), bin_centers])
        binw = np.median(np.abs(widths))
    else:
        binw = 1.0
    total = counts.sum()

    # PDFs
    def gaussian_pdf(x, mu, sigma):
        sigma = max(1e-300, sigma)
        return np.exp(-0.5*((x-mu)/sigma)**2) / (np.sqrt(2*np.pi)*sigma)
    def onesided_exp_pdf(x, lam, t):
        y = np.zeros_like(x, dtype=float)
        mask = x <= t
        y[mask] = lam * np.exp(lam*(x[mask] - t))
        return y

    g = gaussian_pdf(bin_centers, mu0, sigma0)
    e = onesided_exp_pdf(bin_centers, lam, t)

    # Convert to "counts per bin" for unit mixture weight
    G = g * (total * binw)
    E = e * (total * binw)

    # If weights not provided, estimate by least squares then renormalize
    if (w0 is None) or (w1 is None):
        A = np.vstack([G, E]).T
        # Solve least squares
        try:
            w_hat, *_ = np.linalg.lstsq(A, counts, rcond=None)
        except Exception:
            w_hat = np.array([0.5, 0.5])
        w_hat = np.clip(w_hat, 0.0, None)
        s = w_hat.sum()
        if s > 0:
            w0, w1 = (w_hat / s).tolist()
        else:
            w0, w1 = 0.6, 0.4

    # Components
    Gc = w0 * G
    Ec = w1 * E
    Mix = Gc + Ec

    # Draw
    base = f"{label_prefix}: " if label_prefix else ""
    if draw_hist:
        ax.bar(bin_centers, counts, width=binw, align='center', alpha=0.3, edgecolor='k', linewidth=0.5, label='Histogram')
    ax.plot(bin_centers, Mix, lw=2, label=f'{base}Mixture (w0={w0:.3f}, w1={w1:.3f})')
    ax.plot(bin_centers, Gc, lw=1.75, linestyle='--', label=f'{base}Gaussian μ={mu0:.3f}, σ={sigma0:.3f}')
    ax.plot(bin_centers, Ec, lw=1.75, linestyle='-.', label=f'{base}Exp tail t={t:.3f}, λ={lam:.5f}')
    ax.grid(True, alpha=0.2)
    return w0, w1

# Main function
def main():
   
    # Check number of arguments
    if len(sys.argv) < 3:
        print("Usage: python plot_hist_and_adc.py <histogram.txt> <adc_file.bin>")
        sys.exit(1)

    # Load ADC file    
    adc_file  = Path(sys.argv[2])

    # Create output filename:
    out_file = adc_file.stem

    # Get data size to load
    MB_to_load = 10
    
    # --- Load ADC data ---
    samples = (MB_to_load * 1024 * 1024) // 2
    
    adc = np.fromfile(adc_file, dtype=np.int16, count=samples)
    x_adc = np.arange(len(adc))
        
    # Load histogram file       
    hist_file = Path(sys.argv[1])

    # Load only first two columns
    hist_data = np.loadtxt(hist_file, delimiter=",", usecols=(0,1))
    
    # Load the last two lines separately because they have 3 columns
    with open(hist_file, "r") as f:
        lines = f.readlines()
    baseline = np.fromstring(lines[-2], sep=",")
    pulses   = np.fromstring(lines[-1], sep=",")

    # --- histogram data ---    
    centres = hist_data[:-2, 0]
    counts  = hist_data[:-2, 1]
    
    # Baseline: (w0, mu0, sigma0)
    w0, mu0, sigma0 = baseline
    
    # Pulses: (w1, t, lam)
    w1, t, lam = pulses

    # Check enough bin centres
    if len(centres) < 2:
        print("Error: need at least 2 bin centres to define edges.")
        sys.exit(1)
        
    # --- Compute bin edges from centres (midpoints) ---
    mid = 0.5 * (centres[1:] + centres[:-1]) # midpoints between centres
    left_edge  = centres[0]  - 0.5 * (centres[1]  - centres[0])
    right_edge = centres[-1] + 0.5 * (centres[-1] - centres[-2])
    edges = np.concatenate([[left_edge], mid, [right_edge]])  # N+1 edges

    # --- Auto-range for nonzero bins ---
    nonzero = np.nonzero(counts)[0]
    if len(nonzero) > 0:
        xmin, xmax = edges[nonzero[0]], edges[nonzero[-1] + 1]
    else:
        xmin, xmax = edges[0], edges[-1]

    # --- Produce gaussian to plot ---
        
    # How many sigmas to show (4–5 is typical)
    k = 6.0
        
    # Build a combined x-range from the two Gaussians
    baseline_min = mu0 - k*sigma0
    baseline_max = mu0 + k*sigma0
    
    # Clip to histogram domain
    g_x_min = max(baseline_min, centres.min())
    g_x_max = min(baseline_max, centres.max())
    
    # Safety if the clip collapses the range
    if g_x_max <= g_x_min:
        g_x_min, g_x_max = centres.min(), centres.max()
    x = np.linspace(g_x_min, g_x_max, 1000)

    # 1) Peak-height scaling (simple visual overlay)
    g1 = norm.pdf(x, mu0, sigma0)
    scale = counts.max() / g1.max()
    g1_plot = g1 * scale

    # --- One-sided exponential overlay for `pulses` ---

    # Domain for the curve (match your histogram range)
    x_exp = np.linspace(centres.min(), mu0-sigma0, 1000)
    
    # One-sided exponential PDF: f(x) = λ exp[λ(x - t)] for x ≤ t, else 0
    pdf = np.where(x_exp <= t, lam * np.exp(lam * (x_exp - t)), 0.0)
    
    # Find the bin at (or nearest to) the exponential mean (which is t)
    i0 = int(np.argmin(np.abs(centres - t)))
    bin_height_at_mean = counts[i0]

    # PDF value at the mean: with t = mean + σ and λ = 1/σ, this is λ/e
    pdf_at_mean = lam / np.e
    
    # Scale the curve so that its value at the mean equals the bin height at the mean
    A = bin_height_at_mean / pdf_at_mean
    y_exp = A * pdf
        
    # --- Create figure and subplots ---
    
    # Create a figure with two columns: left (3 stacked subplots), right (1 subplot)
    fig = plt.figure(figsize=(18, 8))  # adjust as needed
    fig.suptitle(f"{out_file}", fontsize=15, weight='bold')
    
    # The outer GridSpec has 1 row × 2 columns
    gs = gridspec.GridSpec(1, 2, width_ratios=[2, 1], wspace=0.25)
    
    # Left column → subdivide into 3 rows (stacked vertically)
    gs_left = gridspec.GridSpecFromSubplotSpec(4, 1, subplot_spec=gs[0], hspace=0.35)
    
    # Create the 3 left subplots
    ax0 = fig.add_subplot(gs_left[0])
    ax1 = fig.add_subplot(gs_left[1])
    ax2 = fig.add_subplot(gs_left[2])
    ax3 = fig.add_subplot(gs_left[3])
    
    # Right column → 1 subplot spanning the full height
    ax4 = fig.add_subplot(gs[1])
    
    axes = [ax0, ax1, ax2, ax3, ax4]  # optional convenience list

    # Histogram subplot (using bin edges)
    # Histogram + Gaussian + One-sided exponential overlay
    axes[0].cla()
    _overlay_gauss_exp_on_axis(axes[0], centres, counts, mu0, sigma0, t, lam, w0, w1)
    axes[0].set_xlim(xmin, xmax)
    axes[0].set_ylabel("Counts")
    axes[0].set_title("Histogram")
    axes[0].legend(loc="best")
    
    # Left tail (log scale) with overlay
    axes[1].cla()
    _overlay_gauss_exp_on_axis(axes[1], centres, counts, mu0, sigma0, t, lam, w0, w1)
    axes[1].set_xlim(xmin, mu0)
    axes[1].set_ylim(10,3e7)
    axes[1].set_yscale('log')
    axes[1].set_ylabel("Counts (log)")
    axes[1].set_title("Histogram (log y)")
    axes[1].legend(loc="upper left")
    
    # ADC waveform (full range)
    axes[2].plot(x_adc, adc, color="tab:green", linewidth=0.8)
    axes[2].set_ylabel("ADC value")
    axes[2].set_title(f"Raw data. First {MB_to_load} MB.")
    axes[2].grid(True, linestyle="--", alpha=0.5)

    # ADC waveform (zoomed around zero)
    zoom_range = (-200, 200)  # adjust to your ADC scale
    axes[3].plot(x_adc, adc, color="tab:orange", linewidth=0.8)
    axes[3].set_ylim(*zoom_range)
    axes[3].set_xlabel("Sample index")
    axes[3].set_ylabel("ADC value")
    axes[3].set_title(f"Raw data. First {MB_to_load} MB. Zoomed around zero.")
    axes[3].grid(True, linestyle="--", alpha=0.5)

    # --- Fourth subfigure: Zoom with overlay around Gaussian region ---
    ax_zoom = axes[4]
    k_sigma = 5.0
    x_min = mu0 - k_sigma * sigma0
    x_max = mu0 + k_sigma * sigma0
    ax_zoom.cla()
    _overlay_gauss_exp_on_axis(ax_zoom, centres, counts, mu0, sigma0, t, lam, w0, w1)
    ax_zoom.set_xlim(x_min, x_max)
    ax_zoom.set_xlabel("Value")
    ax_zoom.set_ylabel("Counts")
    ax_zoom.set_title("Histogram: Gaussian region (zoom).")
    ax_zoom.legend(loc="best")
    plt.tight_layout()
    plt.savefig(f"{out_file}_baseline.pdf")

if __name__ == "__main__":
    main()

