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
    // -- Frequency-domain-style filters (spatial approximation) ---
    /** Low-frequency filter: keeps smooth/low-frequency content (Gaussian blur). */
    static wxImage FilterLowFreq(const wxImage& image, int kernelSize, double sigma);

    /** High-frequency filter: keeps edges/details = Original - LowFreq. */
    static wxImage FilterHighFreq(const wxImage& image, int kernelSize, double sigma);

    /**
     * Hybrid image: combines the low-frequency content of one image
     * with the high-frequency content of another.
     * Both images are resized to the same dimensions before mixing.
     */
    static wxImage HybridImage(const wxImage& lowSrc, const wxImage& highSrc,
                                int kernelSize, double sigma);
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
