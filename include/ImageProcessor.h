#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <wx/image.h>
#include <opencv2/opencv.hpp>

class ImageProcessor {
public:
    // Convert wxImage to cv::Mat (RGB -> BGR)
    static cv::Mat WxToCv(const wxImage& wxImg);

    // Convert cv::Mat back to wxImage (BGR -> RGB)
    static wxImage CvToWx(const cv::Mat& cvMat);

    // Noise addition
    static wxImage AddUniformNoise(const wxImage& input, double amplitude = 50);
    static wxImage AddGaussianNoise(const wxImage& input, double mean = 0, double stddev = 25);
    static wxImage AddSaltPepperNoise(const wxImage& input, double prob = 0.01);

    // Low-pass filters
    static wxImage ApplyAverageFilter(const wxImage& input, int kernelSize = 5);
    static wxImage ApplyGaussianFilter(const wxImage& input, int kernelSize = 5, double sigma = 1.5);
    static wxImage ApplyMedianFilter(const wxImage& input, int kernelSize = 5);
};

#endif // IMAGEPROCESSOR_H