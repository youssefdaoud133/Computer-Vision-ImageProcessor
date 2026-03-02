#include "FrequencyFilters.h"
#include <algorithm>
#include <cstring>

// ============================================================================
// 2D DISCRETE FOURIER TRANSFORM (DFT)
// ============================================================================
// Transforms image from spatial domain to frequency domain
// Formula: F(u,v) = sum over x,y of f(x,y) * e^(-j*2*pi*(ux/M + vy/N))
// ============================================================================
std::vector<std::vector<std::complex<double>>> FrequencyFilters::DFT2D(
    const std::vector<std::vector<double>>& input) {
    
    int M = input.size();      // Number of rows (height)
    int N = input[0].size();   // Number of columns (width)
    
    // Initialize output matrix with zeros
    std::vector<std::vector<std::complex<double>>> output(M, 
        std::vector<std::complex<double>>(N, std::complex<double>(0, 0)));
    
    // Temporary storage for row-wise DFT results
    std::vector<std::vector<std::complex<double>>> temp(M, 
        std::vector<std::complex<double>>(N));
    
    // Step 1: Apply 1D DFT to each row
    for (int m = 0; m < M; m++) {
        for (int k = 0; k < N; k++) {
            std::complex<double> sum(0, 0);
            for (int n = 0; n < N; n++) {
                // Calculate rotation angle for this frequency component
                double angle = -2.0 * M_PI * k * n / N;
                // Multiply input by complex exponential and accumulate
                sum += input[m][n] * std::complex<double>(cos(angle), sin(angle));
            }
            temp[m][k] = sum;
        }
    }
    
    // Step 2: Apply 1D DFT to each column of the result
    for (int k = 0; k < N; k++) {
        for (int l = 0; l < M; l++) {
            std::complex<double> sum(0, 0);
            for (int m = 0; m < M; m++) {
                double angle = -2.0 * M_PI * l * m / M;
                sum += temp[m][k] * std::complex<double>(cos(angle), sin(angle));
            }
            output[l][k] = sum;
        }
    }
    
    return output;
}

// ============================================================================
// INVERSE 2D DISCRETE FOURIER TRANSFORM (IDFT)
// ============================================================================
// Transforms frequency spectrum back to spatial domain
// Formula: f(x,y) = (1/MN) * sum over u,v of F(u,v) * e^(j*2*pi*(ux/M + vy/N))
// ============================================================================
std::vector<std::vector<double>> FrequencyFilters::IDFT2D(
    const std::vector<std::vector<std::complex<double>>>& input) {
    
    int M = input.size();
    int N = input[0].size();
    
    // Output matrix for reconstructed image
    std::vector<std::vector<double>> output(M, std::vector<double>(N, 0));
    
    // Temporary storage for column-wise IDFT results
    std::vector<std::vector<std::complex<double>>> temp(M, 
        std::vector<std::complex<double>>(N));
    
    // Step 1: Apply 1D IDFT to each column
    for (int k = 0; k < N; k++) {
        for (int m = 0; m < M; m++) {
            std::complex<double> sum(0, 0);
            for (int l = 0; l < M; l++) {
                // Note: positive angle for inverse transform
                double angle = 2.0 * M_PI * m * l / M;
                sum += input[l][k] * std::complex<double>(cos(angle), sin(angle));
            }
            // Normalize by M
            temp[m][k] = sum / static_cast<double>(M);
        }
    }
    
    // Step 2: Apply 1D IDFT to each row
    for (int m = 0; m < M; m++) {
        for (int n = 0; n < N; n++) {
            std::complex<double> sum(0, 0);
            for (int k = 0; k < N; k++) {
                double angle = 2.0 * M_PI * n * k / N;
                sum += temp[m][k] * std::complex<double>(cos(angle), sin(angle));
            }
            // Normalize by N and take real part
            output[m][n] = sum.real() / static_cast<double>(N);
        }
    }
    
    return output;
}

// ============================================================================
// IMAGE TO GRAYSCALE MATRIX CONVERSION
// ============================================================================
// Converts RGB image to 2D grayscale matrix using luminosity method
// Gray = 0.299*R + 0.587*G + 0.114*B (standard ITU-R BT.601)
// ============================================================================
std::vector<std::vector<double>> FrequencyFilters::ImageToGrayMatrix(const wxImage& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    
    // Create output matrix
    std::vector<std::vector<double>> matrix(height, std::vector<double>(width));
    unsigned char* data = image.GetData();
    
    // Convert each pixel to grayscale
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate pixel index (3 bytes per pixel: R, G, B)
            int idx = (y * width + x) * 3;
            // Apply luminosity formula for perceptual grayscale
            double gray = 0.299 * data[idx] + 0.587 * data[idx + 1] + 0.114 * data[idx + 2];
            matrix[y][x] = gray;
        }
    }
    
    return matrix;
}

// ============================================================================
// GRAYSCALE MATRIX TO IMAGE CONVERSION
// ============================================================================
// Converts 2D grayscale matrix back to wxImage format
// ============================================================================
wxImage FrequencyFilters::GrayMatrixToImage(const std::vector<std::vector<double>>& matrix) {
    int height = matrix.size();
    int width = matrix[0].size();
    
    // Create new wxImage
    wxImage image(width, height);
    unsigned char* data = image.GetData();
    
    // Convert each matrix value to pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Clamp values to valid range [0, 255]
            int value = static_cast<int>(std::clamp(matrix[y][x], 0.0, 255.0));
            int idx = (y * width + x) * 3;
            // Set R, G, B to same value for grayscale
            data[idx] = data[idx + 1] = data[idx + 2] = static_cast<unsigned char>(value);
        }
    }
    
    return image;
}

// ============================================================================
// SPECTRUM SHIFT (CENTER ZERO FREQUENCY)
// ============================================================================
// Rearranges the quadrants of the spectrum so that zero frequency is at center
// This makes visualization and filtering more intuitive
// Before: DC component at corners, high frequencies at center
// After:  DC component at center, high frequencies at edges
// ============================================================================
void FrequencyFilters::ShiftSpectrum(std::vector<std::vector<std::complex<double>>>& spectrum) {
    int M = spectrum.size();
    int N = spectrum[0].size();
    int halfM = M / 2;
    int halfN = N / 2;
    
    // Swap quadrants diagonally
    // Q1 <-> Q4 (top-left with bottom-right)
    // Q2 <-> Q3 (top-right with bottom-left)
    for (int i = 0; i < halfM; i++) {
        for (int j = 0; j < halfN; j++) {
            // Swap Q1 (top-left) with Q4 (bottom-right)
            std::swap(spectrum[i][j], spectrum[i + halfM][j + halfN]);
            // Swap Q2 (top-right) with Q3 (bottom-left)
            std::swap(spectrum[i][j + halfN], spectrum[i + halfM][j]);
        }
    }
}

// ============================================================================
// GAUSSIAN LOW PASS FILTER CREATION
// ============================================================================
// Creates a Gaussian low pass filter mask
// Formula: H(u,v) = e^(-D(u,v)^2 / (2 * cutoff^2))
// Where D(u,v) is the distance from center
// Properties: Smooth transition, no ringing artifacts
// ============================================================================
std::vector<std::vector<double>> FrequencyFilters::CreateGaussianLowPassFilter(
    int rows, int cols, double cutoff) {
    
    std::vector<std::vector<double>> filter(rows, std::vector<double>(cols));
    
    // Calculate center coordinates
    int centerX = cols / 2;
    int centerY = rows / 2;
    
    // Generate filter values
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            // Calculate Euclidean distance from center
            double distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
            // Apply Gaussian formula
            // Low frequencies (near center) get values close to 1
            // High frequencies (far from center) get values close to 0
            filter[y][x] = exp(-(distance * distance) / (2 * cutoff * cutoff));
        }
    }
    
    return filter;
}

// ============================================================================
// GAUSSIAN HIGH PASS FILTER CREATION
// ============================================================================
// Creates a Gaussian high pass filter mask
// Formula: H_hp(u,v) = 1 - H_lp(u,v) = 1 - e^(-D(u,v)^2 / (2 * cutoff^2))
// High pass is complement of low pass filter
// ============================================================================
std::vector<std::vector<double>> FrequencyFilters::CreateGaussianHighPassFilter(
    int rows, int cols, double cutoff) {
    
    std::vector<std::vector<double>> filter(rows, std::vector<double>(cols));
    
    int centerX = cols / 2;
    int centerY = rows / 2;
    
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            double distance = sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
            // High Pass = 1 - Low Pass
            // Low frequencies (near center) get values close to 0
            // High frequencies (far from center) get values close to 1
            filter[y][x] = 1.0 - exp(-(distance * distance) / (2 * cutoff * cutoff));
        }
    }
    
    return filter;
}

// ============================================================================
// APPLY LOW PASS FILTER
// ============================================================================
// Removes high frequency components (edges, noise, fine details)
// Result: Blurred/smoothed image
// Steps:
// 1. Convert to grayscale matrix
// 2. Apply DFT to get frequency spectrum
// 3. Shift spectrum to center DC component
// 4. Multiply spectrum by low pass filter
// 5. Shift back and apply inverse DFT
// ============================================================================
wxImage FrequencyFilters::ApplyLowPassFilter(const wxImage& image, double cutoffFrequency) {
    // Step 1: Convert image to grayscale matrix
    auto grayMatrix = ImageToGrayMatrix(image);
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    
    // Step 2: Transform to frequency domain
    auto spectrum = DFT2D(grayMatrix);
    
    // Step 3: Center the spectrum (move DC to center)
    ShiftSpectrum(spectrum);
    
    // Step 4: Create and apply the low pass filter
    auto filter = CreateGaussianLowPassFilter(height, width, cutoffFrequency);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Multiply spectrum by filter (element-wise)
            spectrum[y][x] *= filter[y][x];
        }
    }
    
    // Step 5: Shift spectrum back to original arrangement
    ShiftSpectrum(spectrum);
    
    // Step 6: Transform back to spatial domain
    auto result = IDFT2D(spectrum);
    
    // Step 7: Convert back to image
    return GrayMatrixToImage(result);
}

// ============================================================================
// APPLY HIGH PASS FILTER
// ============================================================================
// Removes low frequency components (smooth regions, gradual changes)
// Result: Edge-enhanced image showing only details
// ============================================================================
wxImage FrequencyFilters::ApplyHighPassFilter(const wxImage& image, double cutoffFrequency) {
    // Step 1: Convert to grayscale
    auto grayMatrix = ImageToGrayMatrix(image);
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    
    // Step 2: Apply DFT
    auto spectrum = DFT2D(grayMatrix);
    
    // Step 3: Shift to center
    ShiftSpectrum(spectrum);
    
    // Step 4: Create and apply high pass filter
    auto filter = CreateGaussianHighPassFilter(height, width, cutoffFrequency);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            spectrum[y][x] *= filter[y][x];
        }
    }
    
    // Step 5: Shift back
    ShiftSpectrum(spectrum);
    
    // Step 6: Inverse transform
    auto result = IDFT2D(spectrum);
    
    // Step 7: Add DC offset for better visualization
    // High pass results are centered around 0, so we add 128 (mid-gray)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            result[y][x] += 128;
        }
    }
    
    return GrayMatrixToImage(result);
}

// ============================================================================
// CREATE HYBRID IMAGE
// ============================================================================
// Combines two images using different frequency components
// - First image contributes low frequencies (visible from far away)
// - Second image contributes high frequencies (visible up close)
// This creates an optical illusion where different images are seen
// at different viewing distances
// ============================================================================
wxImage FrequencyFilters::CreateHybridImage(const wxImage& lowFreqImage, 
                                             const wxImage& highFreqImage,
                                             double lowCutoff, double highCutoff) {
    // Ensure both images have the same dimensions
    int width = std::min(lowFreqImage.GetWidth(), highFreqImage.GetWidth());
    int height = std::min(lowFreqImage.GetHeight(), highFreqImage.GetHeight());
    
    // Resize images to match dimensions
    wxImage img1 = lowFreqImage.Scale(width, height, wxIMAGE_QUALITY_HIGH);
    wxImage img2 = highFreqImage.Scale(width, height, wxIMAGE_QUALITY_HIGH);
    
    // ========================================
    // Process first image with Low Pass Filter
    // ========================================
    auto gray1 = ImageToGrayMatrix(img1);
    auto spectrum1 = DFT2D(gray1);
    ShiftSpectrum(spectrum1);
    
    // Apply low pass filter
    auto lpFilter = CreateGaussianLowPassFilter(height, width, lowCutoff);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            spectrum1[y][x] *= lpFilter[y][x];
        }
    }
    
    ShiftSpectrum(spectrum1);
    auto lowPassed = IDFT2D(spectrum1);
    
    // ========================================
    // Process second image with High Pass Filter
    // ========================================
    auto gray2 = ImageToGrayMatrix(img2);
    auto spectrum2 = DFT2D(gray2);
    ShiftSpectrum(spectrum2);
    
    // Apply high pass filter
    auto hpFilter = CreateGaussianHighPassFilter(height, width, highCutoff);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            spectrum2[y][x] *= hpFilter[y][x];
        }
    }
    
    ShiftSpectrum(spectrum2);
    auto highPassed = IDFT2D(spectrum2);
    
    // ========================================
    // Combine both filtered images
    // ========================================
    std::vector<std::vector<double>> hybrid(height, std::vector<double>(width));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Weighted combination of both frequency components
            // Adding offset (64) to improve visibility
            hybrid[y][x] = lowPassed[y][x] * 0.5 + highPassed[y][x] * 0.5 + 64;
        }
    }
    
    return GrayMatrixToImage(hybrid);
}

// ============================================================================
// GET FREQUENCY SPECTRUM (FOR VISUALIZATION)
// ============================================================================
// Generates a visual representation of the frequency spectrum
// Uses log scaling for better visualization: log(1 + |F(u,v)|)
// Useful for reports and understanding frequency content
// ============================================================================
wxImage FrequencyFilters::GetFrequencySpectrum(const wxImage& image) {
    // Convert and transform
    auto grayMatrix = ImageToGrayMatrix(image);
    int height = grayMatrix.size();
    int width = grayMatrix[0].size();
    
    auto spectrum = DFT2D(grayMatrix);
    ShiftSpectrum(spectrum);
    
    // Calculate magnitude spectrum with log scaling
    std::vector<std::vector<double>> magnitude(height, std::vector<double>(width));
    double maxMag = 0;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Log scaling: log(1 + magnitude) for better visualization
            // Adding 1 prevents log(0)
            magnitude[y][x] = log(1 + std::abs(spectrum[y][x]));
            // Track maximum for normalization
            maxMag = std::max(maxMag, magnitude[y][x]);
        }
    }
    
    // Normalize to 0-255 range for display
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            magnitude[y][x] = (magnitude[y][x] / maxMag) * 255;
        }
    }
    
    return GrayMatrixToImage(magnitude);
}