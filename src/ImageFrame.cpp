#include "ImageFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>

wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET, ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE, ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR, ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT, ImageFrame::OnFilterInvert)
wxEND_EVENT_TABLE()

ImageFrame::ImageFrame(const wxString& title, const wxString& imagePath)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_originalPath(imagePath) {

    // Create a status bar to show tips
    CreateStatusBar();
    SetStatusText("Double-check filters in the toolbar above!");

    // Create Toolbar for filters
    CreateFilterToolbar();

    // Create Image Panel
    m_imagePanel = new ImagePanel(this, imagePath);

    // Initial configuration for resizing
    wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_imagePanel, 1, wxEXPAND | wxALL, 0);
    SetSizerAndFit(pSizer);

    // Apply basic default size (will trigger resize to fit on open)
    SetSize(wxSize(800, 600));
}

void ImageFrame::CreateFilterToolbar() {
    m_toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);
    
    // Add tools (Reset, Grayscale, Blur) - note adding simple string text tools
    // wxArtProvider gives us some built-in icons to use.
    wxBitmap bmpReset = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR);
    wxBitmap bmpGray = wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_TOOLBAR);
    wxBitmap bmpBlur = wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR);
    wxBitmap bmpInv = wxArtProvider::GetBitmap(wxART_WARNING, wxART_TOOLBAR);

    m_toolbar->AddTool(ID_RESET, "Reset", bmpReset, "Reset to Original Image");
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", bmpGray, "Apply Grayscale Filter");
    m_toolbar->AddTool(ID_FILTER_BLUR, "Blur", bmpBlur, "Apply Blur Filter");
    m_toolbar->AddTool(ID_FILTER_INVERT, "Invert", bmpInv, "Apply Invert Image Filter");

    m_toolbar->Realize();
}

void ImageFrame::OnReset(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    
    wxImage original = m_imagePanel->GetOriginalImage();
    if (original.IsOk()) {
        m_imagePanel->SetImage(original);
        SetStatusText("Image reset to original state.");
    }
}

void ImageFrame::OnFilterGrayscale(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    // Apply on current image iteratively, or original? Let's apply on original!
    wxImage original = m_imagePanel->GetOriginalImage();
    if (original.IsOk()) {
        wxImage gray = original.ConvertToGreyscale();
        m_imagePanel->SetImage(gray);
        SetStatusText("Applied Grayscale filter.");
    }
}

void ImageFrame::OnFilterBlur(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage blur = current.Blur(5); // radius = 5
        m_imagePanel->SetImage(blur);
        SetStatusText("Applied Blur filter.");
    }
}

void ImageFrame::OnFilterInvert(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        // Explicitly copy to avoid modifying shared reference-counted data (like original image)
        current = current.Copy();

        // Simple manual Invert (loop through all pixels)
        unsigned char* data = current.GetData();
        int dataSize = current.GetWidth() * current.GetHeight() * 3;
        for (int i = 0; i < dataSize; ++i) {
            data[i] = 255 - data[i];
        }
        
        m_imagePanel->SetImage(current);
        SetStatusText("Applied Invert Colors filter.");
    }
}
