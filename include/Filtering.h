#ifndef FILTERING_H
#define FILTERING_H

#include <wx/wx.h>
#include <vector>

class Filtering {
public:
    // Edge Detection Algorithms (Scratch)
    // Returns {magnitude, gradX, gradY}
    static std::vector<wxImage> ApplySobel(const wxImage& image);
    static std::vector<wxImage> ApplyRoberts(const wxImage& image);
    static std::vector<wxImage> ApplyPrewitt(const wxImage& image);

    // Edge Detection (OpenCV) — single result, no X/Y split required
    static wxImage ApplyCanny(const wxImage& image);

    // Histogram Utilities
    static std::vector<int>    GetHistogram(const wxImage& image);
    static std::vector<double> GetDistributionCurve(const std::vector<int>& histogram);

private:
    // Returns {magnitude, gradX, gradY}
    static std::vector<wxImage> ApplyMatrix(const wxImage& image,
                                             const std::vector<std::vector<int>>& kernelX,
                                             const std::vector<std::vector<int>>& kernelY);
};

#endif // FILTERING_H