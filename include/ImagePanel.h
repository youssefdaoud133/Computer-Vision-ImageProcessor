#ifndef IMAGEPANEL_H
#define IMAGEPANEL_H

#include <wx/wx.h>
#include <wx/dcclient.h>

/**
 * @class ImagePanel
 * @brief Custom wxPanel handling the drawing of an image with resizing behavior.
 *
 * It stores an internal wxImage and handles OnPaint events.
 */
class ImagePanel : public wxPanel {
public:
    /**
     * @brief Constructs an ImagePanel to view the given image.
     * @param parent The parent wxWindow containing the panel.
     * @param filePath The local path to the image file to be loaded.
     */
    ImagePanel(wxWindow* parent, const wxString& filePath);

    /**
     * @brief Updates the currently displayed image.
     * @param newImage The modified wxImage to replace the current display.
     */
    void SetImage(const wxImage& newImage);

    /**
     * @brief Returns the original image without any modifications.
     * @return The original wxImage loaded from file.
     */
    wxImage GetOriginalImage() const;

    /**
     * @brief Returns the currently displayed image.
     * @return The modified wxImage currently displayed.
     */
    wxImage GetCurrentImage() const;

private:
    /**
     * @brief Event handler to draw the bitmap whenever a paint event occurs.
     * @param event The triggered wxPaintEvent.
     */
    void OnPaint(wxPaintEvent& event);
    
    /**
     * @brief Helper to deal with preserving aspect ratio when resizing
     * @param event The triggered wxSizeEvent.
     */
    void OnSize(wxSizeEvent& event);

    wxImage m_originalImage;    // The original unmodified image
    wxImage m_currentImage;     // The currently displayed (potentially filtered) image
    wxBitmap m_scaledBitmap;    // The cached resized bitmap to display

    // Setup event table
    wxDECLARE_EVENT_TABLE();
};

#endif // IMAGEPANEL_H
