// filtering.cpp — FIXED to satisfy requirements 3 & 4
#include "Filtering.h"
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>

// OpenCV for Canny
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

    int kSize  = (int)kernelX.size();
    int offset = kSize / 2;

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

    return { resMag, resX, resY };
}

// ─────────────────────────────────────────────────────────────
// Public edge detectors — return {magnitude, gradX, gradY}
// ─────────────────────────────────────────────────────────────
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