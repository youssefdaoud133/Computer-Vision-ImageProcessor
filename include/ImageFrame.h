#ifndef IMAGEFRAME_H
#define IMAGEFRAME_H

#include <wx/wx.h>
#include "ImagePanel.h"

/**
 * @class ImageFrame
 * @brief Window class specifically for displaying and interacting with an image.
 */
class ImageFrame : public wxFrame {
public:
    /**
     * @brief Constructs an ImageFrame.
     * @param title Window title text.
     * @param imagePath Absolute path of the image image.
     */
    ImageFrame(const wxString& title, const wxString& imagePath);

private:
    /**
     * @brief Constructs the UI Toolbar containing filters.
     */
    void CreateFilterToolbar();

    // Existing handlers
    void OnReset(wxCommandEvent& event);
    void OnFilterGrayscale(wxCommandEvent& event);
    void OnFilterBlur(wxCommandEvent& event);
    void OnFilterInvert(wxCommandEvent& event);

    // --- NEW OpenCV handlers ---
    void OnNoiseUniform(wxCommandEvent& event);
    void OnNoiseGaussian(wxCommandEvent& event);
    void OnNoiseSaltPepper(wxCommandEvent& event);
    void OnFilterAverage(wxCommandEvent& event);
    void OnFilterGaussian(wxCommandEvent& event);
    void OnFilterMedian(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();
    
    // UI elements
    ImagePanel* m_imagePanel;
    wxToolBar*  m_toolbar;
    wxString    m_originalPath;
};

// Unified tool/filter IDs
enum {
    ID_RESET = 100,
    ID_FILTER_GRAYSCALE,
    ID_FILTER_BLUR,
    ID_FILTER_INVERT,
    ID_NOISE_UNIFORM,
    ID_NOISE_GAUSSIAN,
    ID_NOISE_SALT_PEPPER,
    ID_FILTER_AVERAGE,
    ID_FILTER_GAUSSIAN,
    ID_FILTER_MEDIAN
};

#endif // IMAGEFRAME_H