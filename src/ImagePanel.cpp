#include "ImagePanel.h"

wxBEGIN_EVENT_TABLE(ImagePanel, wxPanel)
    EVT_PAINT(ImagePanel::OnPaint)
    EVT_SIZE(ImagePanel::OnSize)
wxEND_EVENT_TABLE()

ImagePanel::ImagePanel(wxWindow* parent, const wxString& filePath)
    : wxPanel(parent, wxID_ANY) {
    if (m_originalImage.LoadFile(filePath, wxBITMAP_TYPE_ANY)) {
        m_currentImage = m_originalImage;
    } else {
        wxLogError("Failed to load image from %s", filePath);
    }
}

void ImagePanel::SetImage(const wxImage& newImage) {
    if (!newImage.IsOk()) {
        return;
    }

    m_currentImage = newImage;
    m_scaledBitmap = wxBitmap(); // invalidate current cache

    wxSize newSize = GetSize();
    if (newSize.x > 0 && newSize.y > 0) {
        // Automatically resize and update cache
        wxClientDC dc(this);
        wxSizeEvent emptyEvent;
        OnSize(emptyEvent);
    }
    Refresh();
}

wxImage ImagePanel::GetOriginalImage() const {
    return m_originalImage;
}

wxImage ImagePanel::GetCurrentImage() const {
    return m_currentImage;
}

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
        newHeight = newWidth / imgRatio;
    } else {
        newWidth = newHeight * imgRatio;
    }

    // Scale it down
    if (newWidth > 0 && newHeight > 0) {
        wxImage scaledImg = m_currentImage.Scale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);
        m_scaledBitmap = wxBitmap(scaledImg);
    }

    Refresh();
}
