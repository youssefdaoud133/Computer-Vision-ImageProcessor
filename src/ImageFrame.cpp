// imageframe.cpp — Updated to show X direction, Y direction, and magnitude separately
#include "ImageFrame.h"
#include "Filtering.h"
#include "HistogramFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>

wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET,            ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE, ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR,      ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT,    ImageFrame::OnFilterInvert)
    EVT_TOOL(ID_FILTER_SOBEL,     ImageFrame::OnFilterSobel)
    EVT_TOOL(ID_FILTER_ROBERTS,   ImageFrame::OnFilterRoberts)
    EVT_TOOL(ID_FILTER_PREWITT,   ImageFrame::OnFilterPrewitt)
    EVT_TOOL(ID_FILTER_CANNY,     ImageFrame::OnFilterCanny)
    EVT_TOOL(ID_VIEW_HISTOGRAM,   ImageFrame::OnViewHistogram)
wxEND_EVENT_TABLE()

ImageFrame::ImageFrame(const wxString& title, const wxString& imagePath)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_originalPath(imagePath) {

    CreateStatusBar();
    SetStatusText("Use the toolbar to apply filters.");
    CreateFilterToolbar();

    m_imagePanel = new ImagePanel(this, imagePath);

    wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_imagePanel, 1, wxEXPAND | wxALL, 0);
    SetSizerAndFit(pSizer);
    SetSize(wxSize(800, 600));
}

void ImageFrame::CreateFilterToolbar() {
    m_toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);
    m_toolbar->AddTool(ID_RESET,            "Reset",     wxArtProvider::GetBitmap(wxART_UNDO,          wxART_TOOLBAR), "Reset to Original Image");
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", wxArtProvider::GetBitmap(wxART_NEW,           wxART_TOOLBAR), "Apply Grayscale Filter");
    m_toolbar->AddTool(ID_FILTER_BLUR,      "Blur",      wxArtProvider::GetBitmap(wxART_REDO,          wxART_TOOLBAR), "Apply Blur Filter");
    m_toolbar->AddTool(ID_FILTER_INVERT,    "Invert",    wxArtProvider::GetBitmap(wxART_WARNING,       wxART_TOOLBAR), "Apply Invert Filter");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_SOBEL,     "Sobel",     wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK,  wxART_TOOLBAR), "Sobel Edge Detection (X, Y, Magnitude)");
    m_toolbar->AddTool(ID_FILTER_ROBERTS,   "Roberts",   wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS,  wxART_TOOLBAR), "Roberts Edge Detection (X, Y, Magnitude)");
    m_toolbar->AddTool(ID_FILTER_PREWITT,   "Prewitt",   wxArtProvider::GetBitmap(wxART_GOTO_FIRST,    wxART_TOOLBAR), "Prewitt Edge Detection (X, Y, Magnitude)");
    m_toolbar->AddTool(ID_FILTER_CANNY,     "Canny",     wxArtProvider::GetBitmap(wxART_HARDDISK,      wxART_TOOLBAR), "Canny Edge Detection (OpenCV)");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_VIEW_HISTOGRAM,   "Histogram", wxArtProvider::GetBitmap(wxART_GO_UP,         wxART_TOOLBAR), "View Image Histogram");
    m_toolbar->Realize();
}

// ─────────────────────────────────────────────────────────────
// Helper: opens two frames showing X and Y direction gradients
// ─────────────────────────────────────────────────────────────
void ImageFrame::ShowEdgeWindows(const std::vector<wxImage>& results,
                                  const wxString& filterName) {
    // results[0] = magnitude, results[1] = X direction, results[2] = Y direction
    m_imagePanel->SetImage(results[0]);
    SetStatusText(filterName + " applied. Separate X/Y windows opened.");

    auto openWindow = [&](const wxImage& img, const wxString& title) {
        wxFrame* frame = new wxFrame(this, wxID_ANY, title,
                                     wxDefaultPosition, wxSize(600, 500));
        wxPanel* panel = new wxPanel(frame);

        wxBitmap bmp(img);
        wxStaticBitmap* sb = new wxStaticBitmap(panel, wxID_ANY, bmp,
                                                 wxDefaultPosition, wxDefaultSize,
                                                 wxFULL_REPAINT_ON_RESIZE);
        wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
        s->Add(sb, 1, wxEXPAND);
        panel->SetSizer(s);
        frame->Show(true);
    };

    openWindow(results[1], filterName + " - X Direction");
    openWindow(results[2], filterName + " - Y Direction");
}

// ─────────────────────────────────────────────────────────────
// Event handlers
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnReset(wxCommandEvent&) {
    if (!m_imagePanel) return;
    wxImage original = m_imagePanel->GetOriginalImage();
    if (original.IsOk()) {
        m_imagePanel->SetImage(original);
        SetStatusText("Image reset to original.");
    }
}

void ImageFrame::OnFilterGrayscale(wxCommandEvent&) {
    if (!m_imagePanel) return;
    wxImage original = m_imagePanel->GetOriginalImage();
    if (original.IsOk()) {
        m_imagePanel->SetImage(original.ConvertToGreyscale());
        SetStatusText("Applied Grayscale filter.");
    }
}

void ImageFrame::OnFilterBlur(wxCommandEvent&) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        m_imagePanel->SetImage(current.Blur(5));
        SetStatusText("Applied Blur filter.");
    }
}

void ImageFrame::OnFilterInvert(wxCommandEvent&) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        current = current.Copy();
        unsigned char* data = current.GetData();
        int dataSize = current.GetWidth() * current.GetHeight() * 3;
        for (int i = 0; i < dataSize; ++i)
            data[i] = 255 - data[i];
        m_imagePanel->SetImage(current);
        SetStatusText("Applied Invert Colors filter.");
    }
}

void ImageFrame::OnFilterSobel(wxCommandEvent&) {
    if (!m_imagePanel) return;
    auto results = Filtering::ApplySobel(m_imagePanel->GetCurrentImage());
    ShowEdgeWindows(results, "Sobel");
}

void ImageFrame::OnFilterRoberts(wxCommandEvent&) {
    if (!m_imagePanel) return;
    auto results = Filtering::ApplyRoberts(m_imagePanel->GetCurrentImage());
    ShowEdgeWindows(results, "Roberts");
}

void ImageFrame::OnFilterPrewitt(wxCommandEvent&) {
    if (!m_imagePanel) return;
    auto results = Filtering::ApplyPrewitt(m_imagePanel->GetCurrentImage());
    ShowEdgeWindows(results, "Prewitt");
}

void ImageFrame::OnFilterCanny(wxCommandEvent&) {
    if (!m_imagePanel) return;
    wxImage res = Filtering::ApplyCanny(m_imagePanel->GetCurrentImage());
    m_imagePanel->SetImage(res);
    SetStatusText("Applied Canny Edge Detection (OpenCV).");
}

void ImageFrame::OnViewHistogram(wxCommandEvent&) {
    if (!m_imagePanel) return;

    // Always use the original image regardless of applied filters
    wxImage original = m_imagePanel->GetOriginalImage();
    if (!original.IsOk()) return;

    auto hist  = Filtering::GetHistogram(original);
    auto curve = Filtering::GetDistributionCurve(hist);
    HistogramFrame* hFrame = new HistogramFrame(this, hist, curve);
    hFrame->Show(true);
}