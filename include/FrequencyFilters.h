#ifndef FREQUENCY_FILTERS_H
#define FREQUENCY_FILTERS_H

#include <wx/wx.h>
#include <vector>
#include <complex>
#include <cmath>

// Mathematical constants definition
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @class FrequencyFilters
 * @brief Handles frequency domain image processing operations
 * 
 * This class provides methods for:
 * - 2D Discrete Fourier Transform (DFT) and its inverse
 * - Low Pass and High Pass filtering in frequency domain
 * - Hybrid image creation by combining two images
 */
class FrequencyFilters {
public:
    // ============================================
    // Fourier Transform Methods
    // ============================================
    
    /**
     * @brief Computes 2D Discrete Fourier Transform
     * @param input Grayscale image as 2D matrix
     * @return Complex spectrum in frequency domain
     */
    static std::vector<std::vector<std::complex<double>>> DFT2D(
        const std::vector<std::vector<double>>& input);
    
    /**
     * @brief Computes Inverse 2D Discrete Fourier Transform
     * @param input Complex spectrum in frequency domain
     * @return Reconstructed image as 2D matrix
     */
    static std::vector<std::vector<double>> IDFT2D(
        const std::vector<std::vector<std::complex<double>>>& input);
    
    // ============================================
    // Frequency Domain Filters
    // ============================================
    
    /**
     * @brief Applies Gaussian Low Pass Filter
     * @param image Input image
     * @param cutoffFrequency Filter cutoff (lower = more blur)
     * @return Filtered image with high frequencies removed
     */
    static wxImage ApplyLowPassFilter(const wxImage& image, double cutoffFrequency);
    
    /**
     * @brief Applies Gaussian High Pass Filter
     * @param image Input image
     * @param cutoffFrequency Filter cutoff (higher = more edges)
     * @return Filtered image showing edges and details
     */
    static wxImage ApplyHighPassFilter(const wxImage& image, double cutoffFrequency);
    
    // ============================================
    // Hybrid Image Creation
    // ============================================
    
    /**
     * @brief Creates a hybrid image from two source images
     * @param lowFreqImage Image for low frequency content (visible from far)
     * @param highFreqImage Image for high frequency content (visible from close)
     * @param lowCutoff Cutoff for low pass filter
     * @param highCutoff Cutoff for high pass filter
     * @return Combined hybrid image
     */
    static wxImage CreateHybridImage(const wxImage& lowFreqImage, 
                                      const wxImage& highFreqImage,
                                      double lowCutoff, double highCutoff);
    
    // ============================================
    // Utility Methods
    // ============================================
    
    /**
     * @brief Converts wxImage to grayscale matrix
     * @param image Input color or grayscale image
     * @return 2D matrix with pixel intensities (0-255)
     */
    static std::vector<std::vector<double>> ImageToGrayMatrix(const wxImage& image);
    
    /**
     * @brief Converts grayscale matrix back to wxImage
     * @param matrix 2D matrix with pixel intensities
     * @return Grayscale wxImage
     */
    static wxImage GrayMatrixToImage(const std::vector<std::vector<double>>& matrix);
    
    /**
     * @brief Generates frequency spectrum visualization
     * @param image Input image
     * @return Magnitude spectrum image for display
     */
    static wxImage GetFrequencySpectrum(const wxImage& image);

private:
    // ============================================
    // Filter Generation Methods
    // ============================================
    
    /**
     * @brief Creates Gaussian Low Pass filter mask
     * @param rows Image height
     * @param cols Image width
     * @param cutoff Cutoff frequency
     * @return 2D filter mask
     */
    static std::vector<std::vector<double>> CreateGaussianLowPassFilter(
        int rows, int cols, double cutoff);
    
    /**
     * @brief Creates Gaussian High Pass filter mask
     * @param rows Image height
     * @param cols Image width
     * @param cutoff Cutoff frequency
     * @return 2D filter mask
     */
    static std::vector<std::vector<double>> CreateGaussianHighPassFilter(
        int rows, int cols, double cutoff);
    
    /**
     * @brief Shifts zero-frequency component to center of spectrum
     * @param spectrum Complex frequency spectrum (modified in place)
     */
    static void ShiftSpectrum(std::vector<std::vector<std::complex<double>>>& spectrum);
};

#endif // FREQUENCY_FILTERS_H