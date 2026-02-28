#include "ImageFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>

wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET, ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE, ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR, ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT, ImageFrame::OnFilterInvert)
    EVT_TOOL(ID_FILTER_SOBEL, ImageFrame::OnFilterSobel)
    EVT_TOOL(ID_FILTER_ROBERTS, ImageFrame::OnFilterRoberts)
    EVT_TOOL(ID_FILTER_PREWITT, ImageFrame::OnFilterPrewitt)
    EVT_TOOL(ID_FILTER_CANNY, ImageFrame::OnFilterCanny)
    EVT_TOOL(ID_VIEW_HISTOGRAM, ImageFrame::OnViewHistogram)
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
    m_toolbar->AddTool(ID_RESET, "Reset", wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR), "Reset to Original Image");
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR), "Apply Grayscale Filter");
    m_toolbar->AddTool(ID_FILTER_BLUR, "Blur", wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR), "Apply Blur Filter");
    m_toolbar->AddTool(ID_FILTER_INVERT, "Invert", wxArtProvider::GetBitmap(wxART_WARNING, wxART_TOOLBAR), "Apply Invert Image Filter");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_SOBEL, "Sobel", wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_TOOLBAR), "Apply Sobel Edge Detection");
    m_toolbar->AddTool(ID_FILTER_ROBERTS, "Roberts", wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR), "Apply Roberts Edge Detection");
    m_toolbar->AddTool(ID_FILTER_PREWITT, "Prewitt", wxArtProvider::GetBitmap(wxART_GOTO_FIRST, wxART_TOOLBAR), "Apply Prewitt Edge Detection");
    m_toolbar->AddTool(ID_FILTER_CANNY, "Canny", wxArtProvider::GetBitmap(wxART_HARDDISK, wxART_TOOLBAR), "Apply Canny (OpenCV) Edge Detection");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_VIEW_HISTOGRAM, "Histogram", wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR), "View Image Histogram");

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

#include "Filtering.h"
#include "HistogramFrame.h"

void ImageFrame::OnFilterSobel(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage res = Filtering::ApplySobel(m_imagePanel->GetCurrentImage());
    m_imagePanel->SetImage(res);
    SetStatusText("Applied Sobel Edge Detection.");
}

void ImageFrame::OnFilterRoberts(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage res = Filtering::ApplyRoberts(m_imagePanel->GetCurrentImage());
    m_imagePanel->SetImage(res);
    SetStatusText("Applied Roberts Edge Detection.");
}

void ImageFrame::OnFilterPrewitt(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage res = Filtering::ApplyPrewitt(m_imagePanel->GetCurrentImage());
    m_imagePanel->SetImage(res);
    SetStatusText("Applied Prewitt Edge Detection.");
}

void ImageFrame::OnFilterCanny(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage res = Filtering::ApplyCanny(m_imagePanel->GetCurrentImage());
    m_imagePanel->SetImage(res);
    SetStatusText("Applied Canny Edge Detection (OpenCV).");
}

void ImageFrame::OnViewHistogram(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    auto hist = Filtering::GetHistogram(current);
    auto curve = Filtering::GetDistributionCurve(hist);
    HistogramFrame* hFrame = new HistogramFrame(this, hist, curve);
    hFrame->Show(true);
}
