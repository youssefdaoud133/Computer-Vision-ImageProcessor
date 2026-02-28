#ifndef FILTERING_H
#define FILTERING_H

#include <wx/wx.h>
#include <vector>

class Filtering {
public:
    // Edge Detection Algorithms (Scratch)
    static wxImage ApplySobel(const wxImage& image);
    static wxImage ApplyRoberts(const wxImage& image);
    static wxImage ApplyPrewitt(const wxImage& image);

    // Edge Detection (OpenCV)
    static wxImage ApplyCanny(const wxImage& image);

    // Histogram Utilities
    static std::vector<int> GetHistogram(const wxImage& image);
    static std::vector<double> GetDistributionCurve(const std::vector<int>& histogram);

private:
    static wxImage ApplyMatrix(const wxImage& image, const std::vector<std::vector<int>>& kernelX, const std::vector<std::vector<int>>& kernelY);
};

#endif // FILTERING_H
