#include "Filtering.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>
#include <numeric>
#include <random>

// OpenCV for Canny and noise RNG
#include <opencv2/opencv.hpp>

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

// Low-Frequency Filter: simply a Gaussian blur (keeps smooth content)
wxImage Filtering::FilterLowFreq(const wxImage& image, int kernelSize, double sigma) {
    return FilterGaussian(image, kernelSize, sigma);
}

// High-Frequency Filter: Original - LowFreq (keeps edges/details)
// Result is shifted by 128 so that zero-difference maps to mid-grey.
wxImage Filtering::FilterHighFreq(const wxImage& image, int kernelSize, double sigma) {
    if (!image.IsOk()) return wxImage();

    wxImage low = FilterGaussian(image, kernelSize, sigma);
    if (!low.IsOk()) return wxImage();

    int w = image.GetWidth();
    int h = image.GetHeight();
    long n = (long)w * h * 3;

    wxImage srcCopy = image.Copy();
    unsigned char* orig = srcCopy.GetData();
    unsigned char* lp   = low.GetData();

    wxImage result(w, h);
    unsigned char* dst = result.GetData();

    for (long i = 0; i < n; ++i) {
        // high = original - low, shifted by 128 for visualisation
        dst[i] = Clamp(static_cast<int>(orig[i]) - static_cast<int>(lp[i]) + 128);
    }
    return result;
}

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