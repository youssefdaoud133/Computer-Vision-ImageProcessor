#ifndef FILTERING_H
#define FILTERING_H

#include <wx/wx.h>
#include <vector>

class Filtering {
public:
    // ── Edge Detection (from scratch) ─────────────────────────
    // Returns {magnitude, gradX, gradY}
    static std::vector<wxImage> ApplySobel(const wxImage& image);
    static std::vector<wxImage> ApplyRoberts(const wxImage& image);
    static std::vector<wxImage> ApplyPrewitt(const wxImage& image);

    // ── Edge Detection (OpenCV) ───────────────────────────────
    static wxImage ApplyCanny(const wxImage& image);

    // ── Histogram Utilities ───────────────────────────────────
    static std::vector<int>    GetHistogram(const wxImage& image);
    static std::vector<double> GetDistributionCurve(const std::vector<int>& histogram);

    // ── Histogram Enhancement ─────────────────────────────────
    static wxImage EqualizeHistogram(const wxImage& image);
    static wxImage NormalizeHistogram(const wxImage& image);

    // ── Noise Addition (using OpenCV RNG) ─────────────────────
    /** Adds uniform random noise in the range [low, high]. */
    static wxImage AddUniformNoise(const wxImage& image, int low, int high);

    /** Adds Gaussian noise with given mean and standard deviation. */
    static wxImage AddGaussianNoise(const wxImage& image, double mean, double stddev);

    /**
     * Adds salt-and-pepper noise.
     * @param density Fraction of pixels affected (0-1), split equally between salt and pepper.
     */
    static wxImage AddSaltPepperNoise(const wxImage& image, double density);

    // ── Low-pass Filters (manual, no library) ─────────────────
    /** Average (box) filter with a kernelSize × kernelSize kernel. */
    static wxImage FilterAverage(const wxImage& image, int kernelSize);

    /**
     * Gaussian low-pass filter built from scratch.
     * @param kernelSize Must be odd.
     * @param sigma      Standard deviation of the Gaussian.
     */
    static wxImage FilterGaussian(const wxImage& image, int kernelSize, double sigma);

    /** Median filter with a kernelSize × kernelSize neighbourhood. */
    static wxImage FilterMedian(const wxImage& image, int kernelSize);
    // ── FFT-based Frequency-Domain Filters ─────────────────────
    /**
     * FFT-based low-pass filter.
     * @param cutoffFreq  Cutoff radius in frequency-domain pixels (e.g. 30).
     * @param filterType  0 = Ideal, 1 = Butterworth (n=2), 2 = Gaussian (default).
     */
    static wxImage FilterLowFreqFFT(const wxImage& image, double cutoffFreq, int filterType = 2);

    /**
     * FFT-based high-pass filter.
     * @param cutoffFreq  Cutoff radius in frequency-domain pixels.
     * @param filterType  0 = Ideal, 1 = Butterworth (n=2), 2 = Gaussian (default).
     */
    static wxImage FilterHighFreqFFT(const wxImage& image, double cutoffFreq, int filterType = 2);

    /** FFT-based band-pass filter — passes frequencies in [lowCutoff, highCutoff]. */
    static wxImage FilterBandPassFFT(const wxImage& image, double lowCutoff, double highCutoff);

    /** FFT-based band-stop (notch) filter — rejects frequencies in [lowCutoff, highCutoff]. */
    static wxImage FilterBandStopFFT(const wxImage& image, double lowCutoff, double highCutoff);

    /** Visualises the log-scale magnitude spectrum of the image. */
    static wxImage VisualizeFrequencySpectrum(const wxImage& image);

    /**
     * Hybrid image: combines the low-frequency content of one image
     * with the high-frequency content of another using FFT-based filters.
     * Both images are resized to the same dimensions before mixing.
     * @param lowCutoff   Cutoff radius (freq-grid pixels) for the low-pass filter.
     * @param highCutoff  Cutoff radius (freq-grid pixels) for the high-pass filter.
     * @param filterType  0 = Ideal (hard circle), 1 = Gaussian (smooth).
     */
    static wxImage HybridImage(const wxImage& lowSrc, const wxImage& highSrc,
                                double lowCutoff, double highCutoff, int filterType = 1);
    /** Clamp a value to [0, 255]. */
    static inline unsigned char Clamp(int v) {
        return static_cast<unsigned char>(v < 0 ? 0 : v > 255 ? 255 : v);
    }

private:
    // ── Internal helpers ──────────────────────────────────────
    // Returns {magnitude, gradX, gradY}
    static std::vector<wxImage> ApplyMatrix(const wxImage& image,
                                             const std::vector<std::vector<int>>& kernelX,
                                             const std::vector<std::vector<int>>& kernelY);
};

#endif // FILTERING_H
