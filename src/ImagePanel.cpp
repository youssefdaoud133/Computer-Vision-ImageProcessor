#include "ImagePanel.h"

wxBEGIN_EVENT_TABLE(ImagePanel, wxPanel)
    EVT_PAINT(ImagePanel::OnPaint)
    EVT_SIZE(ImagePanel::OnSize)
wxEND_EVENT_TABLE()

// ============================================================================
// CONSTRUCTOR
// ============================================================================
// Modified to handle empty file path for programmatically created images
// ============================================================================
ImagePanel::ImagePanel(wxWindow* parent, const wxString& filePath)
    : wxPanel(parent, wxID_ANY) {
    
    // Only attempt to load if path is not empty
    if (!filePath.IsEmpty()) {
        if (m_originalImage.LoadFile(filePath, wxBITMAP_TYPE_ANY)) {
            m_currentImage = m_originalImage;
        } else {
            wxLogError("Failed to load image from %s", filePath);
        }
    }
    // If filePath is empty, image will be set later via SetImage()
}

// ============================================================================
// SET IMAGE
// ============================================================================
// Modified to set original image if not already set
// This allows hybrid images and spectrum to work properly
// ============================================================================
void ImagePanel::SetImage(const wxImage& newImage) {
    if (!newImage.IsOk()) {
        return;
    }

    // If original image is not set yet, set it now
    // This is important for images created programmatically (hybrid, spectrum)
    if (!m_originalImage.IsOk()) {
        m_originalImage = newImage.Copy();
    }

    m_currentImage = newImage;
    m_scaledBitmap = wxBitmap(); // Invalidate current cache

    wxSize newSize = GetSize();
    if (newSize.x > 0 && newSize.y > 0) {
        // Automatically resize and update cache
        wxSizeEvent emptyEvent;
        OnSize(emptyEvent);
    }
    Refresh();
}

// ============================================================================
// GET ORIGINAL IMAGE
// ============================================================================
wxImage ImagePanel::GetOriginalImage() const {
    return m_originalImage;
}

// ============================================================================
// GET CURRENT IMAGE
// ============================================================================
wxImage ImagePanel::GetCurrentImage() const {
    return m_currentImage;
}

// ============================================================================
// ON PAINT EVENT
// ============================================================================
// Draws the scaled bitmap centered in the panel
// ============================================================================
void ImagePanel::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);

    if (m_scaledBitmap.IsOk()) {
        // Center the image if it doesn't take the full panel
        wxSize panelSize = GetSize();
        wxSize imgSize = m_scaledBitmap.GetSize();
        
        int xPos = (panelSize.GetWidth() - imgSize.GetWidth()) / 2;
        int yPos = (panelSize.GetHeight() - imgSize.GetHeight()) / 2;

        dc.DrawBitmap(m_scaledBitmap, xPos, yPos, false);
    }
}

// ============================================================================
// ON SIZE EVENT
// ============================================================================
// Rescales the image to fit the panel while maintaining aspect ratio
// ============================================================================
void ImagePanel::OnSize(wxSizeEvent& event) {
    if (!m_currentImage.IsOk()) return;

    // Get the new panel dimensions
    wxSize parentSize = GetClientSize();

    // Prevent rendering issues due to zero dimensions
    if (parentSize.x == 0 || parentSize.y == 0) return;

    // Calculate aspect ratios
    double imgRatio = (double)m_currentImage.GetWidth() / (double)m_currentImage.GetHeight();
    double pnlRatio = (double)parentSize.GetWidth() / (double)parentSize.GetHeight();

    int newWidth = parentSize.GetWidth();
    int newHeight = parentSize.GetHeight();

    // Fit with aspect ratio preserved
    if (imgRatio > pnlRatio) {
        // Image is wider than panel ratio - fit to width
        newHeight = static_cast<int>(newWidth / imgRatio);
    } else {
        // Image is taller than panel ratio - fit to height
        newWidth = static_cast<int>(newHeight * imgRatio);
    }

    // Scale the image
    if (newWidth > 0 && newHeight > 0) {
        wxImage scaledImg = m_currentImage.Scale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);
        m_scaledBitmap = wxBitmap(scaledImg);
    }

    Refresh();
}