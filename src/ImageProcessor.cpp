#include "ImageProcessor.h"
#include <wx/log.h>

cv::Mat ImageProcessor::WxToCv(const wxImage& wxImg) {
    if (!wxImg.IsOk()) return cv::Mat();

    int w = wxImg.GetWidth();
    int h = wxImg.GetHeight();
    // wxImage stores data as RGB RGB RGB ... (3 bytes per pixel)
    cv::Mat cvImg(h, w, CV_8UC3, const_cast<unsigned char*>(wxImg.GetData()));
    // OpenCV uses BGR order, so convert
    cv::cvtColor(cvImg, cvImg, cv::COLOR_RGB2BGR);
    return cvImg.clone(); // clone to avoid sharing data with wxImage
}

wxImage ImageProcessor::CvToWx(const cv::Mat& cvMat) {
    if (cvMat.empty()) return wxImage();

    cv::Mat rgbMat;
    if (cvMat.channels() == 3) {
        cv::cvtColor(cvMat, rgbMat, cv::COLOR_BGR2RGB);
    } else if (cvMat.channels() == 1) {
        // Grayscale: convert to 3-channel for wxImage (wxImage expects RGB)
        cv::cvtColor(cvMat, rgbMat, cv::COLOR_GRAY2RGB);
    } else {
        wxLogError("Unsupported number of channels in OpenCV Mat");
        return wxImage();
    }

    wxImage wxImg(rgbMat.cols, rgbMat.rows, rgbMat.data, true);
    // wxImage makes an internal copy, so we can let rgbMat go out of scope
    return wxImg.Copy(); // ensure self-contained
}

wxImage ImageProcessor::AddUniformNoise(const wxImage& input, double amplitude) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat noise(cvImg.size(), CV_16SC3); // signed 16-bit to handle negative
    cv::randu(noise, -amplitude, amplitude);

    cv::Mat result;
    cv::addWeighted(cvImg, 1.0, noise, 1.0, 0, result, CV_8U);

    return CvToWx(result);
}

wxImage ImageProcessor::AddGaussianNoise(const wxImage& input, double mean, double stddev) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat noise(cvImg.size(), CV_16SC3);
    cv::randn(noise, mean, stddev);

    cv::Mat result;
    cv::addWeighted(cvImg, 1.0, noise, 1.0, 0, result, CV_8U);

    return CvToWx(result);
}

wxImage ImageProcessor::AddSaltPepperNoise(const wxImage& input, double prob) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat result = cvImg.clone();
    int totalPixels = result.rows * result.cols;
    int numSaltPepper = static_cast<int>(totalPixels * prob);

    // Salt (white)
    for (int i = 0; i < numSaltPepper; ++i) {
        int row = rand() % result.rows;
        int col = rand() % result.cols;
        if (result.channels() == 3) {
            result.at<cv::Vec3b>(row, col) = cv::Vec3b(255, 255, 255);
        } else {
            result.at<uchar>(row, col) = 255;
        }
    }
    // Pepper (black)
    for (int i = 0; i < numSaltPepper; ++i) {
        int row = rand() % result.rows;
        int col = rand() % result.cols;
        if (result.channels() == 3) {
            result.at<cv::Vec3b>(row, col) = cv::Vec3b(0, 0, 0);
        } else {
            result.at<uchar>(row, col) = 0;
        }
    }

    return CvToWx(result);
}

wxImage ImageProcessor::ApplyAverageFilter(const wxImage& input, int kernelSize) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat result;
    cv::blur(cvImg, result, cv::Size(kernelSize, kernelSize));
    return CvToWx(result);
}

wxImage ImageProcessor::ApplyGaussianFilter(const wxImage& input, int kernelSize, double sigma) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat result;
    cv::GaussianBlur(cvImg, result, cv::Size(kernelSize, kernelSize), sigma);
    return CvToWx(result);
}

wxImage ImageProcessor::ApplyMedianFilter(const wxImage& input, int kernelSize) {
    cv::Mat cvImg = WxToCv(input);
    if (cvImg.empty()) return input;

    cv::Mat result;
    cv::medianBlur(cvImg, result, kernelSize);
    return CvToWx(result);
}