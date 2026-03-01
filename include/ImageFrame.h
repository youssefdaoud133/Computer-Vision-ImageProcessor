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

    /**
     * @brief Displays the RGB histogram of the current image.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnHistogram(wxCommandEvent& event);

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
    ID_HISTOGRAM
};

#endif // IMAGEFRAME_H
