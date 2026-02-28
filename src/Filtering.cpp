#include "Filtering.h"
#include <cmath>
#include <algorithm>
#include <vector>

wxImage Filtering::ApplySobel(const wxImage& image) {
    std::vector<std::vector<int>> kernelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    std::vector<std::vector<int>> kernelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};
    return ApplyMatrix(image, kernelX, kernelY);
}

wxImage Filtering::ApplyRoberts(const wxImage& image) {
    wxImage gray = image.ConvertToGreyscale();
    int width = gray.GetWidth();
    int height = gray.GetHeight();
    wxImage result(width, height);
    
    unsigned char* data = gray.GetData();
    unsigned char* resData = result.GetData();

    for (int y = 0; y < height - 1; y++) {
        for (int x = 0; x < width - 1; x++) {
            int p00 = data[(y * width + x) * 3];
            int p11 = data[((y + 1) * width + (x + 1)) * 3];
            int p01 = data[(y * width + (x + 1)) * 3];
            int p10 = data[((y + 1) * width + x) * 3];

            int gx = p00 - p11;
            int gy = p01 - p10;
            int mag = std::min(255, (int)std::sqrt(gx * gx + gy * gy));
            
            resData[(y * width + x) * 3] = resData[(y * width + x) * 3 + 1] = resData[(y * width + x) * 3 + 2] = (unsigned char)mag;
        }
    }
    return result;
}

wxImage Filtering::ApplyPrewitt(const wxImage& image) {
    std::vector<std::vector<int>> kernelX = {{-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1}};
    std::vector<std::vector<int>> kernelY = {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}};
    return ApplyMatrix(image, kernelX, kernelY);
}

wxImage Filtering::ApplyMatrix(const wxImage& image, const std::vector<std::vector<int>>& kernelX, const std::vector<std::vector<int>>& kernelY) {
    wxImage gray = image.ConvertToGreyscale();
    int width = gray.GetWidth();
    int height = gray.GetHeight();
    wxImage result(width, height);
    
    unsigned char* data = gray.GetData();
    unsigned char* resData = result.GetData();
    int kSize = kernelX.size();
    int offset = kSize / 2;

    for (int y = offset; y < height - offset; y++) {
        for (int x = offset; x < width - offset; x++) {
            int valX = 0;
            int valY = 0;
            for (int ky = 0; ky < kSize; ky++) {
                for (int kx = 0; kx < kSize; kx++) {
                    int pixel = data[((y + ky - offset) * width + (x + kx - offset)) * 3];
                    valX += pixel * kernelX[ky][kx];
                    valY += pixel * kernelY[ky][kx];
                }
            }
            int mag = std::min(255, (int)std::sqrt(valX * valX + valY * valY));
            resData[(y * width + x) * 3] = resData[(y * width + x) * 3 + 1] = resData[(y * width + x) * 3 + 2] = (unsigned char)mag;
        }
    }
    return result;
}

wxImage Filtering::ApplyCanny(const wxImage& image) {
    int width = image.GetWidth();
    int height = image.GetHeight();
    wxImage gray = image.ConvertToGreyscale();
    
    // 1. Gaussian Blur (simple 5x5)
    wxImage blurred = gray.Blur(2); 

    // 2. Sobel Gradients
    unsigned char* data = blurred.GetData();
    std::vector<double> magnitude(width * height, 0.0);
    std::vector<double> angle(width * height, 0.0);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int gx = -data[((y-1)*width + (x-1))*3] + data[((y-1)*width + (x+1))*3]
                     -2*data[(y*width + (x-1))*3] + 2*data[(y*width + (x+1))*3]
                     -data[((y+1)*width + (x-1))*3] + data[((y+1)*width + (x+1))*3];
            int gy = -data[((y-1)*width + (x-1))*3] - 2*data[((y-1)*width + x)*3] - data[((y-1)*width + (x+1))*3]
                     +data[((y+1)*width + (x-1))*3] + 2*data[((y+1)*width + x)*3] + data[((y+1)*width + (x+1))*3];
            
            magnitude[y*width + x] = std::sqrt(gx*gx + gy*gy);
            angle[y*width + x] = std::atan2(gy, gx) * 180.0 / M_PI;
        }
    }

    // 3. Non-Maximum Suppression
    std::vector<double> nms(width * height, 0.0);
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            double q = 255, r = 255;
            double ang = angle[y*width + x];
            if (ang < 0) ang += 180;

            if ((0 <= ang && ang < 22.5) || (157.5 <= ang && ang <= 180)) {
                q = magnitude[y*width + (x+1)];
                r = magnitude[y*width + (x-1)];
            } else if (22.5 <= ang && ang < 67.5) {
                q = magnitude[(y+1)*width + (x-1)];
                r = magnitude[(y-1)*width + (x+1)];
            } else if (67.5 <= ang && ang < 112.5) {
                q = magnitude[(y+1)*width + x];
                r = magnitude[(y-1)*width + x];
            } else if (112.5 <= ang && ang < 157.5) {
                q = magnitude[(y-1)*width + (x-1)];
                r = magnitude[(y+1)*width + (x+1)];
            }

            if (magnitude[y*width + x] >= q && magnitude[y*width + x] >= r)
                nms[y*width + x] = magnitude[y*width + x];
            else
                nms[y*width + x] = 0;
        }
    }

    // 4. Double Threshold & Hysteresis
    double lowThresh = 20, highThresh = 50;
    wxImage result(width, height);
    unsigned char* resData = result.GetData();
    std::vector<int> edgeMap(width * height, 0); // 0: none, 1: weak, 2: strong

    for (int i = 0; i < width * height; i++) {
        if (nms[i] >= highThresh) edgeMap[i] = 2;
        else if (nms[i] >= lowThresh) edgeMap[i] = 1;
    }

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            if (edgeMap[y*width + x] == 2) {
                resData[(y*width+x)*3] = resData[(y*width+x)*3+1] = resData[(y*width+x)*3+2] = 255;
            } else if (edgeMap[y*width + x] == 1) {
                bool hasStrongNeighbor = false;
                for (int i = -1; i <= 1; i++) {
                    for (int j = -1; j <= 1; j++) {
                        if (edgeMap[(y+i)*width + (x+j)] == 2) hasStrongNeighbor = true;
                    }
                }
                resData[(y*width+x)*3] = resData[(y*width+x)*3+1] = resData[(y*width+x)*3+2] = hasStrongNeighbor ? 255 : 0;
            } else {
                resData[(y*width+x)*3] = resData[(y*width+x)*3+1] = resData[(y*width+x)*3+2] = 0;
            }
        }
    }
    return result;
}

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
        double sum = 0;
        int count = 0;
        for (int j = std::max(0, i - window); j <= std::min(255, i + window); j++) {
            sum += histogram[j];
            count++;
        }
        curve[i] = sum / count;
    }
    return curve;
}
