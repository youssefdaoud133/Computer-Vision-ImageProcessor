#ifndef IMAGEFRAME_H
#define IMAGEFRAME_H

#include <wx/wx.h>
#include <wx/busyinfo.h>
#include "ImagePanel.h"
#include "FrequencyFilters.h"

/**
 * @class ImageFrame
 * @brief Window class specifically for displaying and interacting with an image.
 * 
 * This class provides a complete image editing interface with:
 * - Basic filters (grayscale, blur, invert)
 * - Frequency domain filters (low pass, high pass)
 * - Hybrid image creation capability
 * - Frequency spectrum visualization
 */
class ImageFrame : public wxFrame {
public:
    /**
     * @brief Constructs an ImageFrame from file path.
     * @param title Window title text.
     * @param imagePath Absolute path of the image file.
     */
    ImageFrame(const wxString& title, const wxString& imagePath);

    /**
     * @brief Constructs an ImageFrame from existing wxImage.
     * @param title Window title text.
     * @param image The wxImage to display.
     */
    ImageFrame(const wxString& title, const wxImage& image);

private:
    /**
     * @brief Constructs the UI Toolbar containing filters.
     */
    void CreateFilterToolbar();

    /**
     * @brief Common initialization code for both constructors.
     */
    void InitializeFrame();

    // ============================================
    // Basic Filter Event Handlers
    // ============================================

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

    // ============================================
    // Frequency Domain Filter Event Handlers
    // ============================================

    /**
     * @brief Applies Gaussian Low Pass Filter to the image.
     * Removes high frequency content, resulting in blur effect.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnLowPassFilter(wxCommandEvent& event);

    /**
     * @brief Applies Gaussian High Pass Filter to the image.
     * Removes low frequency content, showing edges and details.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnHighPassFilter(wxCommandEvent& event);

    /**
     * @brief Creates a hybrid image from current and user-selected image.
     * Combines low frequencies from one image with high frequencies from another.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnHybridImage(wxCommandEvent& event);

    /**
     * @brief Displays the frequency spectrum of the current image.
     * Shows magnitude spectrum for analysis purposes.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnShowSpectrum(wxCommandEvent& event);

    // Setup event table
    wxDECLARE_EVENT_TABLE();
    
    // UI elements
    ImagePanel* m_imagePanel;
    wxToolBar*  m_toolbar;
    wxString    m_originalPath;
};

// ============================================
// Tool / Filter IDs
// ============================================
enum {
    // Basic filters
    ID_RESET = 100,
    ID_FILTER_GRAYSCALE,
    ID_FILTER_BLUR,
    ID_FILTER_INVERT,
    
    // Frequency domain filters
    ID_LOW_PASS,
    ID_HIGH_PASS,
    ID_HYBRID,
    ID_SPECTRUM
};

#endif // IMAGEFRAME_H