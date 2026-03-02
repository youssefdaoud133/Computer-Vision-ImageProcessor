#ifndef IMAGEFRAME_H
#define IMAGEFRAME_H

#include <wx/wx.h>
#include <vector>
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

    /**
     * @brief Returns true if m_imagePanel is valid and ready to use.
     *        Used as a guard at the top of every event handler.
     */
    bool PanelReady() const;

    /**
     * @brief Opens two popup windows showing the X and Y direction gradients,
     *        and displays the magnitude in the main panel.
     * @param results Vector of {magnitude, gradX, gradY} images from edge detectors.
     * @param filterName Name shown in the popup window titles (e.g. "Sobel").
     */
    void ShowEdgeWindows(const std::vector<wxImage>& results, const wxString& filterName);

    /**
     * @brief Restores the image back to its original unmodified state.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnReset(wxCommandEvent& event);

    /**
     * @brief Applies a grayscale transformation to the image.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnFilterGrayscale(wxCommandEvent& event);

    /**
     * @brief Applies a blur filter to the image.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnFilterBlur(wxCommandEvent& event);

    /**
     * @brief Applies an invert colors filter to the image.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnFilterInvert(wxCommandEvent& event);

    void OnFilterSobel(wxCommandEvent& event);
    void OnFilterRoberts(wxCommandEvent& event);
    void OnFilterPrewitt(wxCommandEvent& event);
    void OnFilterCanny(wxCommandEvent& event);
    void OnViewHistogram(wxCommandEvent& event);
    void OnFilterEqualize(wxCommandEvent& event);
    void OnFilterNormalize(wxCommandEvent& event);

    // Setup event table
    wxDECLARE_EVENT_TABLE();

    // UI elements
    ImagePanel* m_imagePanel;
    wxToolBar*  m_toolbar;
    wxString    m_originalPath;
};

// Tool / Filter IDs
enum {
    ID_RESET = 100,
    ID_FILTER_GRAYSCALE,
    ID_FILTER_BLUR,
    ID_FILTER_INVERT,
    ID_FILTER_SOBEL,
    ID_FILTER_ROBERTS,
    ID_FILTER_PREWITT,
    ID_FILTER_CANNY,
    ID_VIEW_HISTOGRAM,
    ID_FILTER_EQUALIZE,
    ID_FILTER_NORMALIZE
};

#endif // IMAGEFRAME_H