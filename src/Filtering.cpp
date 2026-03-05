#include "Filtering.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>
#include <numeric>
#include <random>
#include <complex>
// OpenCV for Canny and noise RNG
#include <opencv2/opencv.hpp>


// Helper structure for complex number operations
typedef std::complex<double> Complex;


// ──────────────────────────────────────────────────────────────────────────────
// Helper: Convert wxImage to 2D double array (grayscale for FFT)
// ──────────────────────────────────────────────────────────────────────────────
static double* ImageToDoubleArray(const wxImage& image, int& width, int& height) {
    wxImage gray = image.ConvertToGreyscale();
    width = gray.GetWidth();
    height = gray.GetHeight();
    
    double* data = new double[width * height];
    unsigned char* pixels = gray.GetData();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 3;
            data[y * width + x] = static_cast<double>(pixels[idx]);
        }
    }
    
    return data;
}

// ──────────────────────────────────────────────────────────────────────────────
// Helper: Convert double array back to wxImage
// ──────────────────────────────────────────────────────────────────────────────
static wxImage DoubleArrayToImage(double* data, int width, int height) {
    wxImage result(width, height);
    unsigned char* pixels = result.GetData();
    
    // Normalize to [0, 255] if needed
    double minVal = *std::min_element(data, data + width * height);
    double maxVal = *std::max_element(data, data + width * height);
    
    for (int i = 0; i < width * height; i++) {
        unsigned char val;
        if (maxVal > minVal) {
            val = static_cast<unsigned char>((data[i] - minVal) * 255.0 / (maxVal - minVal));
        } else {
            val = 0;
        }
        pixels[i * 3] = pixels[i * 3 + 1] = pixels[i * 3 + 2] = val;
    }
    
    return result;
}


// ─────────────────────────────────────────────────────────────
// Helper: wxImage <-> cv::Mat conversions
// ─────────────────────────────────────────────────────────────
static cv::Mat wxImageToMat(const wxImage& img) {
    int w = img.GetWidth(), h = img.GetHeight();
    cv::Mat mat(h, w, CV_8UC3, img.GetData());
    cv::Mat bgr;
    cv::cvtColor(mat, bgr, cv::COLOR_RGB2BGR);
    return bgr.clone();
}

static wxImage MatToWxImage(const cv::Mat& mat) {
    cv::Mat rgb;
    if (mat.channels() == 1)
        cv::cvtColor(mat, rgb, cv::COLOR_GRAY2RGB);
    else
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);

    wxImage img(rgb.cols, rgb.rows);
    memcpy(img.GetData(), rgb.data, rgb.cols * rgb.rows * 3);
    return img;
}

// ─────────────────────────────────────────────────────────────
// Internal: ApplyMatrix — returns {magnitude, gradX, gradY}
// ─────────────────────────────────────────────────────────────
std::vector<wxImage> Filtering::ApplyMatrix(const wxImage& image,
                                             const std::vector<std::vector<int>>& kernelX,
                                             const std::vector<std::vector<int>>& kernelY) {
    // bthwl l image l grey bec edge detection byshtghl 3la intensity msh colors                                            
    wxImage gray = image.ConvertToGreyscale();
    int width  = gray.GetWidth();
    int height = gray.GetHeight();

    wxImage resMag(width, height);
    wxImage resX  (width, height);
    wxImage resY  (width, height);

    unsigned char* data    = gray.GetData();
    unsigned char* dataMag = resMag.GetData();
    unsigned char* dataX   = resX.GetData();
    unsigned char* dataY   = resY.GetData();

    memset(dataMag, 0, width * height * 3);
    memset(dataX,   0, width * height * 3);
    memset(dataY,   0, width * height * 3);

    int kSize  = (int)kernelX.size(); // bygeb size l kernel (if 3*3 kernel so size=3)
    int offset = kSize / 2; // offset=1 hseb awl w akhr row w awl w akhr column w yb2a da l current pixel(ashan makhrogsh bara l sora)
// btlf 3la kol pixel w ttb2 convolution (h3dy 3la kol pixel w akhod 3*3 matrix hwalen l pixel)
    for (int y = offset; y < height - offset; y++) {
        for (int x = offset; x < width - offset; x++) {
            int valX = 0, valY = 0;
            for (int ky = 0; ky < kSize; ky++) {
                for (int kx = 0; kx < kSize; kx++) {
                    int pixel = data[((y + ky - offset) * width + (x + kx - offset)) * 3];
                    valX += pixel * kernelX[ky][kx];
                    valY += pixel * kernelY[ky][kx];
                }
            }

            int mag  = std::min(255, (int)std::sqrt((double)(valX * valX + valY * valY)));
            int absX = std::min(255, std::abs(valX));
            int absY = std::min(255, std::abs(valY));

            int idx = (y * width + x) * 3;
            dataMag[idx] = dataMag[idx+1] = dataMag[idx+2] = (unsigned char)mag;
            dataX[idx]   = dataX[idx+1]   = dataX[idx+2]   = (unsigned char)absX;
            dataY[idx]   = dataY[idx+1]   = dataY[idx+2]   = (unsigned char)absY;
        }
    }
    return { resMag, resX, resY }; // bnrg3 l 3 swar l magnitude w direction x w direction y
}

std::vector<wxImage> Filtering::ApplySobel(const wxImage& image) {
    std::vector<std::vector<int>> kX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    std::vector<std::vector<int>> kY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    return ApplyMatrix(image, kX, kY);
}

std::vector<wxImage> Filtering::ApplyPrewitt(const wxImage& image) {
    std::vector<std::vector<int>> kX = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
    std::vector<std::vector<int>> kY = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
    return ApplyMatrix(image, kX, kY);
}

std::vector<wxImage> Filtering::ApplyRoberts(const wxImage& image) {
    wxImage gray = image.ConvertToGreyscale();
    int width  = gray.GetWidth();
    int height = gray.GetHeight();

    wxImage resMag(width, height);
    wxImage resX  (width, height);
    wxImage resY  (width, height);

    unsigned char* data    = gray.GetData();
    unsigned char* dataMag = resMag.GetData();
    unsigned char* dataX   = resX.GetData();
    unsigned char* dataY   = resY.GetData();

    memset(dataMag, 0, width * height * 3);
    memset(dataX,   0, width * height * 3);
    memset(dataY,   0, width * height * 3);

    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            int p00 = data[(y * width + x) * 3];
            int p11 = data[((y + 1) * width + (x + 1)) * 3];
            int p01 = data[(y * width + (x + 1)) * 3];
            int p10 = data[((y + 1) * width + x) * 3];

            int gx = p00 - p11;
            int gy = p01 - p10;

            int mag  = std::min(255, (int)std::sqrt((double)(gx * gx + gy * gy)));
            int absX = std::min(255, std::abs(gx));
            int absY = std::min(255, std::abs(gy));

            int idx = (y * width + x) * 3;
            dataMag[idx] = dataMag[idx+1] = dataMag[idx+2] = (unsigned char)mag;
            dataX[idx]   = dataX[idx+1]   = dataX[idx+2]   = (unsigned char)absX;
            dataY[idx]   = dataY[idx+1]   = dataY[idx+2]   = (unsigned char)absY;
        }
    }

    return { resMag, resX, resY };
}

// ─────────────────────────────────────────────────────────────
// Canny — uses OpenCV (as required by the task)
// ─────────────────────────────────────────────────────────────
wxImage Filtering::ApplyCanny(const wxImage& image) {
    cv::Mat src = wxImageToMat(image);
    cv::Mat gray, blurred, edges;

    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.4);
    cv::Canny(blurred, edges, 50, 150);

    return MatToWxImage(edges);
}

// ─────────────────────────────────────────────────────────────
// Histogram & Distribution Curve
// ─────────────────────────────────────────────────────────────
std::vector<int> Filtering::GetHistogram(const wxImage& image) {
    std::vector<int> hist(256, 0);
    wxImage gray = image.ConvertToGreyscale();
    unsigned char* data = gray.GetData();
    int size = gray.GetWidth() * gray.GetHeight();
    for (int i = 0; i < size; i++) {
        hist[data[i * 3]]++;
    }
    return hist;
}

std::vector<double> Filtering::GetDistributionCurve(const std::vector<int>& histogram) {
    std::vector<double> curve(256);
    int window = 5;
    for (int i = 0; i < 256; i++) {
        double sum  = 0;
        int    count = 0;
        for (int j = std::max(0, i - window); j <= std::min(255, i + window); j++) {
            sum += histogram[j];
            count++;
        }
        curve[i] = sum / count;
    }
    return curve;
}

// ─────────────────────────────────────────────────────────────
// Histogram Equalization
// Spreads intensity values across the full [0, 255] range using CDF.
// Formula: new_val[i] = round( CDF[i] / n * (L-1) )
// ─────────────────────────────────────────────────────────────
wxImage Filtering::EqualizeHistogram(const wxImage& image) {
    wxImage gray = image.ConvertToGreyscale();
    int width  = gray.GetWidth();
    int height = gray.GetHeight();
    int n      = width * height;     // total number of pixels
    int L      = 256;                // number of intensity levels

    unsigned char* data = gray.GetData();

    // Step 1: compute histogram (n_i = count of pixels with intensity i)
    std::vector<int> hist(L, 0);
    for (int i = 0; i < n; i++)
        hist[data[i * 3]]++;

    // Step 2: build CDF (cumulative sum of histogram)
    std::vector<int> cdf(L, 0);
    cdf[0] = hist[0];
    for (int i = 1; i < L; i++)
        cdf[i] = cdf[i - 1] + hist[i];

    // Step 3: build lookup table — new_val = round( cdf[i] / n * (L-1) )
    std::vector<unsigned char> lut(L);
    for (int i = 0; i < L; i++)
        lut[i] = static_cast<unsigned char>(std::round(static_cast<double>(cdf[i]) / n * (L - 1)));

    // Step 4: apply lookup table to every pixel (grayscale → keep RGB equal)
    wxImage result(width, height);
    unsigned char* out = result.GetData();
    for (int i = 0; i < n; i++) {
        unsigned char eq = lut[data[i * 3]];
        out[i * 3]     = eq;
        out[i * 3 + 1] = eq;
        out[i * 3 + 2] = eq;
    }
    return result;
}

// ─────────────────────────────────────────────────────────────
// Histogram Normalization
// Stretches intensity values linearly so the darkest pixel becomes 0
// and the brightest becomes 255.
// Formula: new_val = round( (old_val - min) / (max - min) * (L-1) )
// ─────────────────────────────────────────────────────────────
wxImage Filtering::NormalizeHistogram(const wxImage& image) {
    wxImage gray = image.ConvertToGreyscale();
    int width  = gray.GetWidth();
    int height = gray.GetHeight();
    int n      = width * height;
    int L      = 256;

    unsigned char* data = gray.GetData();

    // Step 1: find min and max intensity values present in the image
    int minVal = 255, maxVal = 0;
    for (int i = 0; i < n; i++) {
        int v = data[i * 3];
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
    }

    // If all pixels are the same intensity, return a black image (avoid div-by-zero)
    if (maxVal == minVal) {
        wxImage result(width, height);
        result.SetData((unsigned char*)calloc(width * height * 3, 1), true);
        return result;
    }

    // Step 2: build lookup table
    std::vector<unsigned char> lut(L);
    for (int i = 0; i < L; i++) {
        lut[i] = static_cast<unsigned char>(
            std::round(static_cast<double>(i - minVal) / (maxVal - minVal) * (L - 1)));
    }

    // Step 3: apply lookup table
    wxImage result(width, height);
    unsigned char* out = result.GetData();
    for (int i = 0; i < n; i++) {
        unsigned char norm = lut[data[i * 3]];
        out[i * 3]     = norm;
        out[i * 3 + 1] = norm;
        out[i * 3 + 2] = norm;
    }
    return result;
}

// ─────────────────────────────────────────────────────────────
// NOISE ADDITION
// ─────────────────────────────────────────────────────────────

// Uniform Noise — adds a random offset ∈ [low, high] to every channel
wxImage Filtering::AddUniformNoise(const wxImage& image, int low, int high) {
    if (!image.IsOk()) return wxImage();
    wxImage result = image.Copy();
    unsigned char* data = result.GetData();
    if (!data) return result;
    long n = (long)result.GetWidth() * result.GetHeight() * 3;

    std::mt19937 rng(std::random_device{}());
    // ensure low <= high to avoid UB in uniform_int_distribution
    if (low > high) std::swap(low, high);
    std::uniform_int_distribution<int> dist(low, high);

    for (long i = 0; i < n; ++i)
        data[i] = Clamp(static_cast<int>(data[i]) + dist(rng));

    return result;
}

// Gaussian Noise — uses std::normal_distribution to generate N(mean, stddev^2) offsets
wxImage Filtering::AddGaussianNoise(const wxImage& image, double mean, double stddev) {
    if (!image.IsOk()) return wxImage();
    if (stddev < 0.001) stddev = 0.001;

    wxImage result = image.Copy();
    unsigned char* data = result.GetData();
    if (!data) return result;

    long n = (long)result.GetWidth() * result.GetHeight() * 3;
    if (n <= 0) return result;

    std::mt19937 rng(std::random_device{}());
    std::normal_distribution<double> dist(mean, stddev);

    for (long i = 0; i < n; ++i) {
        double noise = dist(rng);
        if (!std::isfinite(noise)) noise = 0.0;
        data[i] = Clamp(static_cast<int>(data[i]) + static_cast<int>(std::round(noise)));
    }

    return result;
}
// Salt & Pepper Noise — randomly sets pixels to 255 (salt) or 0 (pepper)
// density is the fraction of total pixels affected (split 50/50 between salt and pepper)
wxImage Filtering::AddSaltPepperNoise(const wxImage& image, double density) {
    if (!image.IsOk()) return wxImage();
    wxImage result = image.Copy();
    int width  = result.GetWidth();
    int height = result.GetHeight();
    int totalPixels = width * height;
    unsigned char* data = result.GetData();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> posDist(0, totalPixels - 1);

    int numAffected = static_cast<int>(density * totalPixels);

    // Salt (white)
    for (int i = 0; i < numAffected / 2; ++i) {
        int idx = posDist(rng) * 3;
        data[idx] = data[idx+1] = data[idx+2] = 255;
    }
    // Pepper (black)
    for (int i = 0; i < numAffected / 2; ++i) {
        int idx = posDist(rng) * 3;
        data[idx] = data[idx+1] = data[idx+2] = 0;
    }

    return result;
}

// ─────────────────────────────────────────────────────────────
// LOW-PASS FILTERS  (fully manual — no library convolution)
// ─────────────────────────────────────────────────────────────

// Helper: pads image with edge-replication (clamp) so output is same size
// Applies a precomputed float kernel of size kSize×kSize to each RGB channel
static wxImage ApplyKernel(const wxImage& image,
                            const std::vector<float>& kernel,
                            int kSize) {
    if (!image.IsOk()) return wxImage();
    int w = image.GetWidth();
    int h = image.GetHeight();
    if (w <= 0 || h <= 0) return wxImage();
    int half = kSize / 2;

    // Make an independent copy so GetData() gives a stable, writable pointer
    wxImage srcImg = image.Copy();
    unsigned char* src = srcImg.GetData();
    if (!src) return wxImage();
    wxImage result(w, h);
    unsigned char* dst = result.GetData();
    if (!dst) return wxImage();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            float sumR = 0, sumG = 0, sumB = 0;
            for (int ky = 0; ky < kSize; ++ky) {
                for (int kx = 0; kx < kSize; ++kx) {
                    // clamp-to-edge padding
                    int sy = std::clamp(y + ky - half, 0, h - 1);
                    int sx = std::clamp(x + kx - half, 0, w - 1);
                    int si = (sy * w + sx) * 3;
                    float k = kernel[ky * kSize + kx];
                    sumR += src[si    ] * k;
                    sumG += src[si + 1] * k;
                    sumB += src[si + 2] * k;
                }
            }
            int di = (y * w + x) * 3;
            dst[di    ] = Filtering::Clamp(static_cast<int>(std::round(sumR)));
            dst[di + 1] = Filtering::Clamp(static_cast<int>(std::round(sumG)));
            dst[di + 2] = Filtering::Clamp(static_cast<int>(std::round(sumB)));
        }
    }
    return result;
}

// Average (Box) Filter — uniform kernel: all weights = 1/(kSize*kSize)
wxImage Filtering::FilterAverage(const wxImage& image, int kernelSize) {
    if (kernelSize % 2 == 0) kernelSize++;   // enforce odd
    int n = kernelSize * kernelSize;
    std::vector<float> kernel(n, 1.0f / n);
    return ApplyKernel(image, kernel, kernelSize);
}

// Gaussian Filter — builds the 2D Gaussian kernel analytically, then convolves
wxImage Filtering::FilterGaussian(const wxImage& image, int kernelSize, double sigma) {
    if (!image.IsOk()) return wxImage();
    if (kernelSize % 2 == 0) kernelSize++;
    if (sigma < 0.1) sigma = 0.1;

    int half = kernelSize / 2;
    std::vector<float> kernel(kernelSize * kernelSize);

    double sum = 0.0;
    for (int ky = -half; ky <= half; ++ky) {
        for (int kx = -half; kx <= half; ++kx) {
            double val = std::exp(-(static_cast<double>(kx)*kx + static_cast<double>(ky)*ky) / (2.0 * sigma * sigma));
            kernel[(ky + half) * kernelSize + (kx + half)] = static_cast<float>(val);
            sum += val;
        }
    }
    
    // Normalise so kernel sums to 1
    if (sum > 0) {
        for (float& v : kernel) v /= static_cast<float>(sum);
    } else {
        // Fallback to center point if kernel is somehow zero-sum
        std::fill(kernel.begin(), kernel.end(), 0.0f);
        kernel[half * kernelSize + half] = 1.0f;
    }

    return ApplyKernel(image, kernel, kernelSize);
}

// Median Filter — for each pixel takes the median of the neighbourhood per channel
wxImage Filtering::FilterMedian(const wxImage& image, int kernelSize) {
    if (!image.IsOk()) return wxImage();
    if (kernelSize % 2 == 0) kernelSize++;
    int half = kernelSize / 2;
    int w = image.GetWidth();
    int h = image.GetHeight();
    if (w <= 0 || h <= 0) return wxImage();

    wxImage srcImg = image.Copy();
    unsigned char* src = srcImg.GetData();
    if (!src) return wxImage();
    wxImage result(w, h);
    unsigned char* dst = result.GetData();
    if (!dst) return wxImage();

    std::vector<unsigned char> winR, winG, winB;
    winR.reserve(kernelSize * kernelSize);
    winG.reserve(kernelSize * kernelSize);
    winB.reserve(kernelSize * kernelSize);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            winR.clear(); winG.clear(); winB.clear();

            for (int ky = -half; ky <= half; ++ky) {
                for (int kx = -half; kx <= half; ++kx) {
                    int sy = std::clamp(y + ky, 0, h - 1);
                    int sx = std::clamp(x + kx, 0, w - 1);
                    int si = (sy * w + sx) * 3;
                    winR.push_back(src[si    ]);
                    winG.push_back(src[si + 1]);
                    winB.push_back(src[si + 2]);
                }
            }

            std::sort(winR.begin(), winR.end());
            std::sort(winG.begin(), winG.end());
            std::sort(winB.begin(), winB.end());

            int mid = (int)winR.size() / 2;
            int di = (y * w + x) * 3;
            dst[di    ] = winR[mid];
            dst[di + 1] = winG[mid];
            dst[di + 2] = winB[mid];
        }
    }
    return result;
}

// -----------------------------------------------------------------
// FREQUENCY-DOMAIN-STYLE FILTERS (spatial approximation)
// -----------------------------------------------------------------

// ──────────────────────────────────────────────────────────────────────────────
// FFT-BASED FREQUENCY DOMAIN FILTERS
// Complete Implementation with Mathematical Notes
// Requires FFTW library or similar FFT implementation
// ──────────────────────────────────────────────────────────────────────────────




// ──────────────────────────────────────────────────────────────────────────────
// FFT-BASED LOW-PASS FILTER
// 
// Mathematical Foundation:
// ------------------------
// In frequency domain, low-pass filtering is multiplication:
//   G(u,v) = F(u,v) * H(u,v)
// 
// Where:
//   F(u,v) = DFT{image}  (Fourier Transform of image)
//   H(u,v) = Low-pass filter transfer function
//   G(u,v) = Filtered result in frequency domain
// 
// Then: filtered_image = IDFT{G(u,v)}
//
// Ideal Low-Pass Filter (ILPF):
//   H(u,v) = 1 if D(u,v) ≤ D0
//   H(u,v) = 0 if D(u,v) > D0
//   
// Where D(u,v) = sqrt(u² + v²) is distance from center in frequency domain
// and D0 is cutoff frequency
//
// Butterworth Low-Pass Filter (BLPF) - smoother transition:
//   H(u,v) = 1 / (1 + [D(u,v)/D0]^(2n))
//   
// Where n is filter order (higher n = sharper cutoff)
//
// Gaussian Low-Pass Filter (GLPF) - smooth, no ringing:
//   H(u,v) = exp(-D²(u,v)/(2D0²))
// ──────────────────────────────────────────────────────────────────────────────
wxImage Filtering::FilterLowFreqFFT(const wxImage& image, double cutoffFreq, int filterType) {
    if (!image.IsOk()) return wxImage();
    
    int width, height;
    double* imageData = ImageToDoubleArray(image, width, height);
    int N = width * height;
    
    // Allocate FFT arrays
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* filter = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    // Prepare input: copy image data to complex array
    for (int i = 0; i < N; i++) {
        in[i][0] = imageData[i];  // Real part
        in[i][1] = 0.0;            // Imaginary part
    }
    
    // Create FFT plans
    fftw_plan forward = fftw_plan_dft_2d(height, width, in, out, 
                                          FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan inverse = fftw_plan_dft_2d(height, width, filter, in, 
                                          FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // Step 1: Forward FFT - transform image to frequency domain
    fftw_execute(forward);
    
    // Step 2: Create and apply low-pass filter in frequency domain
    
    // Center coordinates for frequency shifting
    int centerY = height / 2;
    int centerX = width / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            
            // Calculate distance from center in frequency domain
            // Shift coordinates so center is (0,0)
            int shiftedY = y - centerY;
            int shiftedX = x - centerX;
            double D = sqrt(shiftedX * shiftedX + shiftedY * shiftedY);
            
            // Calculate filter response H(u,v)
            double H;
            
            switch (filterType) {
                case 0: // Ideal Low-Pass Filter
                    H = (D <= cutoffFreq) ? 1.0 : 0.0;
                    break;
                    
                case 1: // Butterworth Low-Pass Filter (order n=2)
                    {
                        int n = 2; // Filter order
                        H = 1.0 / (1.0 + pow(D / cutoffFreq, 2 * n));
                    }
                    break;
                    
                case 2: // Gaussian Low-Pass Filter
                default:
                    H = exp(-D * D / (2.0 * cutoffFreq * cutoffFreq));
                    break;
            }
            
            // Apply filter: G(u,v) = F(u,v) * H(u,v)
            filter[idx][0] = out[idx][0] * H;
            filter[idx][1] = out[idx][1] * H;
        }
    }
    
    // Step 3: Inverse FFT - transform back to spatial domain
    fftw_execute(inverse);
    
    // Step 4: Normalize and extract real part
    double* resultData = new double[N];
    for (int i = 0; i < N; i++) {
        resultData[i] = in[i][0] / N; // FFTW doesn't normalize by default
    }
    
    // Convert back to wxImage
    wxImage result = DoubleArrayToImage(resultData, width, height);
    
    // Cleanup
    delete[] imageData;
    delete[] resultData;
    fftw_destroy_plan(forward);
    fftw_destroy_plan(inverse);
    fftw_free(in);
    fftw_free(out);
    fftw_free(filter);
    
    return result;
}

// ──────────────────────────────────────────────────────────────────────────────
// FFT-BASED HIGH-PASS FILTER
//
// Mathematical Foundation:
// ------------------------
// High-pass filter in frequency domain:
//   H_hp(u,v) = 1 - H_lp(u,v)
//
// Where H_lp(u,v) is the low-pass filter transfer function
//
// This gives us:
//   Ideal High-Pass: H = 0 if D(u,v) ≤ D0, else 1
//   Butterworth High-Pass: H = 1 / (1 + [D0/D(u,v)]^(2n))
//   Gaussian High-Pass: H = 1 - exp(-D²(u,v)/(2D0²))
//
// Note: For visualization, we still add the 128 shift as in spatial version
// ──────────────────────────────────────────────────────────────────────────────
wxImage Filtering::FilterHighFreqFFT(const wxImage& image, double cutoffFreq, int filterType) {
    if (!image.IsOk()) return wxImage();
    
    int width, height;
    double* imageData = ImageToDoubleArray(image, width, height);
    int N = width * height;
    
    // Allocate FFT arrays
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* filter = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    // Prepare input
    for (int i = 0; i < N; i++) {
        in[i][0] = imageData[i];
        in[i][1] = 0.0;
    }
    
    // Create FFT plans
    fftw_plan forward = fftw_plan_dft_2d(height, width, in, out, 
                                          FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan inverse = fftw_plan_dft_2d(height, width, filter, in, 
                                          FFTW_BACKWARD, FFTW_ESTIMATE);
    
    // Forward FFT
    fftw_execute(forward);
    
    // Create and apply high-pass filter
    int centerY = height / 2;
    int centerX = width / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int shiftedY = y - centerY;
            int shiftedX = x - centerX;
            double D = sqrt(shiftedX * shiftedX + shiftedY * shiftedY);
            
            // Calculate high-pass filter response
            double H;
            
            switch (filterType) {
                case 0: // Ideal High-Pass Filter
                    H = (D > cutoffFreq) ? 1.0 : 0.0;
                    break;
                    
                case 1: // Butterworth High-Pass Filter
                    {
                        int n = 2;
                        H = 1.0 / (1.0 + pow(cutoffFreq / D, 2 * n));
                        if (D == 0) H = 0; // DC component (zero frequency)
                    }
                    break;
                    
                case 2: // Gaussian High-Pass Filter
                default:
                    H = 1.0 - exp(-D * D / (2.0 * cutoffFreq * cutoffFreq));
                    break;
            }
            
            // Apply filter
            filter[idx][0] = out[idx][0] * H;
            filter[idx][1] = out[idx][1] * H;
        }
    }
    
    // Inverse FFT
    fftw_execute(inverse);
    
    // Extract and normalize
    double* resultData = new double[N];
    for (int i = 0; i < N; i++) {
        resultData[i] = in[i][0] / N;
    }
    
    // Convert to image and add 128 shift for visualization
    wxImage result = DoubleArrayToImage(resultData, width, height);
    
    // Add 128 shift to visualize both positive and negative frequencies
    unsigned char* pixels = result.GetData();
    for (int i = 0; i < width * height; i++) {
        int val = pixels[i * 3] + 128;
        pixels[i * 3] = pixels[i * 3 + 1] = pixels[i * 3 + 2] = Clamp(val);
    }
    
    // Cleanup
    delete[] imageData;
    delete[] resultData;
    fftw_destroy_plan(forward);
    fftw_destroy_plan(inverse);
    fftw_free(in);
    fftw_free(out);
    fftw_free(filter);
    
    return result;
}

// ──────────────────────────────────────────────────────────────────────────────
// FFT-BAND-PASS FILTER
//
// Mathematical Foundation:
// ------------------------
// Band-pass filter passes frequencies within a certain range:
//   H_bp(u,v) = 1 if D_low ≤ D(u,v) ≤ D_high
//   H_bp(u,v) = 0 otherwise
//
// Or as combination of low and high-pass:
//   H_bp = H_high * H_low
//   where H_high is high-pass with cutoff D_low
//   and H_low is low-pass with cutoff D_high
// ──────────────────────────────────────────────────────────────────────────────
wxImage Filtering::FilterBandPassFFT(const wxImage& image, 
                                      double lowCutoff, double highCutoff) {
    if (!image.IsOk()) return wxImage();
    if (lowCutoff >= highCutoff) {
        wxLogError("Band-pass filter: lowCutoff must be less than highCutoff");
        return wxImage();
    }
    
    int width, height;
    double* imageData = ImageToDoubleArray(image, width, height);
    int N = width * height;
    
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* filter = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    for (int i = 0; i < N; i++) {
        in[i][0] = imageData[i];
        in[i][1] = 0.0;
    }
    
    fftw_plan forward = fftw_plan_dft_2d(height, width, in, out, 
                                          FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan inverse = fftw_plan_dft_2d(height, width, filter, in, 
                                          FFTW_BACKWARD, FFTW_ESTIMATE);
    
    fftw_execute(forward);
    
    int centerY = height / 2;
    int centerX = width / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int shiftedY = y - centerY;
            int shiftedX = x - centerX;
            double D = sqrt(shiftedX * shiftedX + shiftedY * shiftedY);
            
            // Band-pass filter: pass frequencies between lowCutoff and highCutoff
            double H = (D >= lowCutoff && D <= highCutoff) ? 1.0 : 0.0;
            
            filter[idx][0] = out[idx][0] * H;
            filter[idx][1] = out[idx][1] * H;
        }
    }
    
    fftw_execute(inverse);
    
    double* resultData = new double[N];
    for (int i = 0; i < N; i++) {
        resultData[i] = in[i][0] / N;
    }
    
    wxImage result = DoubleArrayToImage(resultData, width, height);
    
    delete[] imageData;
    delete[] resultData;
    fftw_destroy_plan(forward);
    fftw_destroy_plan(inverse);
    fftw_free(in);
    fftw_free(out);
    fftw_free(filter);
    
    return result;
}

// ──────────────────────────────────────────────────────────────────────────────
// FFT-BAND-STOP (NOTCH) FILTER
//
// Mathematical Foundation:
// ------------------------
// Band-stop filter attenuates frequencies within a certain range:
//   H_bs(u,v) = 0 if D_low ≤ D(u,v) ≤ D_high
//   H_bs(u,v) = 1 otherwise
//
// Useful for removing periodic noise patterns
// ──────────────────────────────────────────────────────────────────────────────
wxImage Filtering::FilterBandStopFFT(const wxImage& image, 
                                      double lowCutoff, double highCutoff) {
    if (!image.IsOk()) return wxImage();
    
    int width, height;
    double* imageData = ImageToDoubleArray(image, width, height);
    int N = width * height;
    
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* filter = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    for (int i = 0; i < N; i++) {
        in[i][0] = imageData[i];
        in[i][1] = 0.0;
    }
    
    fftw_plan forward = fftw_plan_dft_2d(height, width, in, out, 
                                          FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_plan inverse = fftw_plan_dft_2d(height, width, filter, in, 
                                          FFTW_BACKWARD, FFTW_ESTIMATE);
    
    fftw_execute(forward);
    
    int centerY = height / 2;
    int centerX = width / 2;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            int shiftedY = y - centerY;
            int shiftedX = x - centerX;
            double D = sqrt(shiftedX * shiftedX + shiftedY * shiftedY);
            
            // Band-stop filter: stop frequencies between lowCutoff and highCutoff
            double H = (D >= lowCutoff && D <= highCutoff) ? 0.0 : 1.0;
            
            filter[idx][0] = out[idx][0] * H;
            filter[idx][1] = out[idx][1] * H;
        }
    }
    
    fftw_execute(inverse);
    
    double* resultData = new double[N];
    for (int i = 0; i < N; i++) {
        resultData[i] = in[i][0] / N;
    }
    
    wxImage result = DoubleArrayToImage(resultData, width, height);
    
    delete[] imageData;
    delete[] resultData;
    fftw_destroy_plan(forward);
    fftw_destroy_plan(inverse);
    fftw_free(in);
    fftw_free(out);
    fftw_free(filter);
    
    return result;
}

// ──────────────────────────────────────────────────────────────────────────────
// VISUALIZE FREQUENCY SPECTRUM (for debugging/education)
//
// Shows the magnitude spectrum |F(u,v)| after FFT
// Useful for understanding frequency content
// ──────────────────────────────────────────────────────────────────────────────
wxImage Filtering::VisualizeFrequencySpectrum(const wxImage& image) {
    if (!image.IsOk()) return wxImage();
    
    int width, height;
    double* imageData = ImageToDoubleArray(image, width, height);
    int N = width * height;
    
    fftw_complex* in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex* out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    
    for (int i = 0; i < N; i++) {
        in[i][0] = imageData[i];
        in[i][1] = 0.0;
    }
    
    fftw_plan forward = fftw_plan_dft_2d(height, width, in, out, 
                                          FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(forward);
    
    // Compute magnitude spectrum and shift for display
    double* magnitude = new double[N];
    int centerY = height / 2;
    int centerX = width / 2;
    
    // Find max magnitude for normalization
    double maxMag = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = y * width + x;
            double real = out[idx][0];
            double imag = out[idx][1];
            double mag = log(1 + sqrt(real*real + imag*imag)); // Log scale for visibility
            
            // Shift: move low frequencies to center
            int shiftedY = (y + centerY) % height;
            int shiftedX = (x + centerX) % width;
            int shiftedIdx = shiftedY * width + shiftedX;
            
            magnitude[shiftedIdx] = mag;
            if (mag > maxMag) maxMag = mag;
        }
    }
    
    // Normalize to [0, 255] for display
    for (int i = 0; i < N; i++) {
        magnitude[i] = (magnitude[i] / maxMag) * 255.0;
    }
    
    wxImage result = DoubleArrayToImage(magnitude, width, height);
    
    delete[] imageData;
    delete[] magnitude;
    fftw_destroy_plan(forward);
    fftw_free(in);
    fftw_free(out);
    
    return result;
}

// ──────────────────────────────────────────────────────────────────────────────
// MATHEMATICAL NOTES FOR FFT-BASED FILTERING
// ──────────────────────────────────────────────────────────────────────────────

/**
 * WHY FFT-BASED FILTERING IS MATHEMATICALLY SUPERIOR:
 * 
 * 1. Convolution Theorem:
 *    f * g  ←→  F · G
 *    
 *    Spatial domain convolution (O(N²)) becomes frequency domain 
 *    multiplication (O(N log N))
 * 
 * 2. Perfect Frequency Separation:
 *    - Can achieve ideal (brick-wall) filters impossible in spatial domain
 *    - No approximation errors from finite kernel sizes
 * 
 * 3. Direct Cutoff Control:
 *    D0 parameter directly corresponds to frequency in cycles/pixel:
 *    D0 = 0.1 → retains frequencies below 0.1 cycles/pixel
 *    D0 = 0.5 → retains all frequencies (Nyquist limit)
 * 
 * 4. Filter Types Comparison:
 * 
 *    Ideal Filter:
 *      Pros: Perfect separation
 *      Cons: Ringing artifacts (Gibbs phenomenon)
 *      
 *    Butterworth Filter:
 *      Pros: Adjustable roll-off, minimal ringing
 *      Cons: Phase distortion
 *      
 *    Gaussian Filter:
 *      Pros: No ringing, smooth transitions
 *      Cons: Less sharp cutoff
 * 
 * 5. DC Component (Zero Frequency):
 *    - Located at corners before shift, center after shift
 *    - Represents average image intensity
 *    - Must be handled specially in high-pass filters
 * 
 * 6. Visualization Shift:
 *    The +128 shift in high-pass visualization maps:
 *    - Negative frequencies (edges) → darker grey
 *    - Positive frequencies (edges) → lighter grey
 *    - Zero (flat regions) → mid-grey (128)
 */


// Hybrid Image: low-freq content of one image + high-freq content of another
// Both images are resized to the same dimensions before mixing.
wxImage Filtering::HybridImage(const wxImage& lowSrc, const wxImage& highSrc,
                                int kernelSize, double sigma) {
    if (!lowSrc.IsOk() || !highSrc.IsOk()) return wxImage();

    // Use the dimensions of the first (low) image as the target size
    int w = lowSrc.GetWidth();
    int h = lowSrc.GetHeight();

    // Resize high-freq source to match
    wxImage highResized = highSrc.Scale(w, h, wxIMAGE_QUALITY_HIGH);

    // Compute the low-frequency component of the low source
    wxImage lowPart = FilterGaussian(lowSrc, kernelSize, sigma);
    if (!lowPart.IsOk()) return wxImage();

    // Compute the high-frequency component of the high source
    wxImage highBlurred = FilterGaussian(highResized, kernelSize, sigma);
    if (!highBlurred.IsOk()) return wxImage();

    long n = (long)w * h * 3;

    unsigned char* lpData   = lowPart.GetData();
    unsigned char* origHigh = highResized.GetData();
    unsigned char* hpBlur   = highBlurred.GetData();

    wxImage result(w, h);
    unsigned char* dst = result.GetData();

    for (long i = 0; i < n; ++i) {
        // hybrid = low_component + (high_original - high_blurred)
        int val = static_cast<int>(lpData[i])
                + (static_cast<int>(origHigh[i]) - static_cast<int>(hpBlur[i]));
        dst[i] = Clamp(val);
    }
    return result;
}