// ImageFrame.cpp
#include "ImageFrame.h"
#include "Filtering.h"
#include "HistogramFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <wx/statline.h>

// ─────────────────────────────────────────────────────────────
// Event table
// ─────────────────────────────────────────────────────────────
wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET,             ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE,  ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR,       ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT,     ImageFrame::OnFilterInvert)
    EVT_TOOL(ID_FILTER_SOBEL,      ImageFrame::OnFilterSobel)
    EVT_TOOL(ID_FILTER_ROBERTS,    ImageFrame::OnFilterRoberts)
    EVT_TOOL(ID_FILTER_PREWITT,    ImageFrame::OnFilterPrewitt)
    EVT_TOOL(ID_FILTER_CANNY,      ImageFrame::OnFilterCanny)
    EVT_TOOL(ID_FILTER_EQUALIZE,   ImageFrame::OnFilterEqualize)
    EVT_TOOL(ID_FILTER_NORMALIZE,  ImageFrame::OnFilterNormalize)
    EVT_TOOL(ID_VIEW_HISTOGRAM,    ImageFrame::OnViewHistogram)
    EVT_LISTBOX(ID_HISTORY_SELECT, ImageFrame::OnHistorySelect)
wxEND_EVENT_TABLE()

// ─────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────
ImageFrame::ImageFrame(const wxString& title, const wxString& imagePath)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(1100, 650)),
      m_originalPath(imagePath),
      m_histPanel(nullptr),
      m_historyList(nullptr)
{
    CreateStatusBar();
    SetStatusText("Use the toolbar to apply filters.");
    CreateFilterToolbar();

    // ── Central content area (image + right sidebar) ──────────
    wxPanel* contentPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxHORIZONTAL);

    // Image panel (takes all remaining horizontal space)
    m_imagePanel = new ImagePanel(contentPanel, imagePath);
    mainSizer->Add(m_imagePanel, 1, wxEXPAND);

    // Right sidebar
    CreateRightSidebar(contentPanel, mainSizer);

    contentPanel->SetSizer(mainSizer);

    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(contentPanel, 1, wxEXPAND);
    SetSizer(frameSizer);

    // Seed history with the original image
    PushHistory("Original");
}

// ─────────────────────────────────────────────────────────────
// UI construction helpers
// ─────────────────────────────────────────────────────────────
void ImageFrame::CreateFilterToolbar() {
    m_toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);

    m_toolbar->AddTool(ID_RESET,            "Reset",     wxArtProvider::GetBitmap(wxART_UNDO,         wxART_TOOLBAR), "Reset to Original Image");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", wxArtProvider::GetBitmap(wxART_NEW,          wxART_TOOLBAR), "Convert to Grayscale");
    m_toolbar->AddTool(ID_FILTER_BLUR,      "Blur",      wxArtProvider::GetBitmap(wxART_REDO,         wxART_TOOLBAR), "Apply Gaussian Blur");
    m_toolbar->AddTool(ID_FILTER_INVERT,    "Invert",    wxArtProvider::GetBitmap(wxART_WARNING,      wxART_TOOLBAR), "Invert Colors");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_SOBEL,     "Sobel",     wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_TOOLBAR), "Sobel Edge Detection");
    m_toolbar->AddTool(ID_FILTER_ROBERTS,   "Roberts",   wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR), "Roberts Edge Detection");
    m_toolbar->AddTool(ID_FILTER_PREWITT,   "Prewitt",   wxArtProvider::GetBitmap(wxART_GOTO_FIRST,   wxART_TOOLBAR), "Prewitt Edge Detection");
    m_toolbar->AddTool(ID_FILTER_CANNY,     "Canny",     wxArtProvider::GetBitmap(wxART_HARDDISK,     wxART_TOOLBAR), "Canny Edge Detection (OpenCV)");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_EQUALIZE,  "Equalize",  wxArtProvider::GetBitmap(wxART_GO_FORWARD,   wxART_TOOLBAR), "Equalize Histogram");
    m_toolbar->AddTool(ID_FILTER_NORMALIZE, "Normalize", wxArtProvider::GetBitmap(wxART_GO_DOWN,      wxART_TOOLBAR), "Normalize Histogram (Contrast Stretch)");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_VIEW_HISTOGRAM,   "Histogram", wxArtProvider::GetBitmap(wxART_GO_UP,        wxART_TOOLBAR), "Open Histogram in Popup Window");

    m_toolbar->Realize();
}

void ImageFrame::CreateRightSidebar(wxWindow* parent, wxSizer* mainSizer) {
    const int SIDEBAR_W = 270;

    wxPanel* sidebar = new wxPanel(parent, wxID_ANY, wxDefaultPosition,
                                   wxSize(SIDEBAR_W, -1));
    sidebar->SetBackgroundColour(wxColour(245, 245, 248));

    wxBoxSizer* sbSizer = new wxBoxSizer(wxVERTICAL);

    // ── Histogram section ─────────────────────────────────────
    wxStaticText* histLabel = new wxStaticText(sidebar, wxID_ANY, "Live Histogram");
    histLabel->SetFont(histLabel->GetFont().Bold());
    sbSizer->Add(histLabel, 0, wxLEFT | wxTOP, 8);

    m_histPanel = new HistogramPanel(sidebar, m_imagePanel->GetCurrentImage());
    m_histPanel->SetMinSize(wxSize(SIDEBAR_W - 4, 220));
    sbSizer->Add(m_histPanel, 3, wxEXPAND | wxALL, 4);

    sbSizer->Add(new wxStaticLine(sidebar), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // ── History section ───────────────────────────────────────
    wxStaticText* historyLabel = new wxStaticText(sidebar, wxID_ANY, "History");
    historyLabel->SetFont(historyLabel->GetFont().Bold());
    sbSizer->Add(historyLabel, 0, wxLEFT | wxTOP, 8);

    wxStaticText* hint = new wxStaticText(sidebar, wxID_ANY,
        "Click any entry to restore it.");
    hint->SetFont(hint->GetFont().Smaller());
    hint->SetForegroundColour(wxColour(120, 120, 120));
    sbSizer->Add(hint, 0, wxLEFT | wxBOTTOM, 8);

    m_historyList = new wxListBox(sidebar, ID_HISTORY_SELECT,
                                  wxDefaultPosition, wxDefaultSize,
                                  0, nullptr, wxLB_SINGLE | wxLB_HSCROLL);
    sbSizer->Add(m_historyList, 2, wxEXPAND | wxALL, 4);

    sidebar->SetSizer(sbSizer);
    mainSizer->Add(sidebar, 0, wxEXPAND);
}

// ─────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────
bool ImageFrame::PanelReady() const {
    return m_imagePanel != nullptr;
}

void ImageFrame::PushHistory(const wxString& label) {
    if (!PanelReady()) return;

    wxImage snapshot = m_imagePanel->GetCurrentImage();
    if (!snapshot.IsOk()) return;

    m_historyImages.push_back(snapshot.Copy());
    m_historyLabels.push_back(label);

    // Prepend step number for clarity
    wxString entry = wxString::Format("%zu. %s", m_historyImages.size(), label);
    m_historyList->Append(entry);

    // Auto-scroll to the latest entry
    m_historyList->SetSelection((int)m_historyList->GetCount() - 1);
}

void ImageFrame::RefreshHistogram() {
    if (m_histPanel && PanelReady()) {
        m_histPanel->Update(m_imagePanel->GetCurrentImage());
    }
}

void ImageFrame::ShowEdgeWindows(const std::vector<wxImage>& results,
                                  const wxString& filterName) {
    // results[0]=magnitude, results[1]=X dir, results[2]=Y dir
    m_imagePanel->SetImage(results[0]);
    SetStatusText(filterName + " applied. X and Y direction windows opened.");
    RefreshHistogram();
    PushHistory(filterName);

    auto openWindow = [&](const wxImage& img, const wxString& title) {
        wxFrame* frame = new wxFrame(this, wxID_ANY, title,
                                     wxDefaultPosition, wxSize(600, 500));
        wxPanel* panel = new wxPanel(frame);
        wxStaticBitmap* sb = new wxStaticBitmap(panel, wxID_ANY, wxBitmap(img));
        wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
        s->Add(sb, 1, wxEXPAND);
        panel->SetSizer(s);
        frame->Show(true);
    };

    openWindow(results[1], filterName + " - X Direction");
    openWindow(results[2], filterName + " - Y Direction");
}

// ─────────────────────────────────────────────────────────────
// Toolbar event handlers
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnReset(wxCommandEvent&) {
    if (!PanelReady()) return;
    wxImage original = m_imagePanel->GetOriginalImage();
    if (!original.IsOk()) return;

    m_imagePanel->SetImage(original);
    RefreshHistogram();
    PushHistory("Reset");
    SetStatusText("Image reset to original.");
}

void ImageFrame::OnFilterGrayscale(wxCommandEvent&) {
    if (!PanelReady()) return;
    wxImage img = m_imagePanel->GetOriginalImage();
    if (!img.IsOk()) return;

    m_imagePanel->SetImage(img.ConvertToGreyscale());
    RefreshHistogram();
    PushHistory("Grayscale");
    SetStatusText("Applied Grayscale filter.");
}

void ImageFrame::OnFilterBlur(wxCommandEvent&) {
    if (!PanelReady()) return;
    wxImage img = m_imagePanel->GetCurrentImage();
    if (!img.IsOk()) return;

    m_imagePanel->SetImage(img.Blur(5));
    RefreshHistogram();
    PushHistory("Blur");
    SetStatusText("Applied Blur (radius 5).");
}

void ImageFrame::OnFilterInvert(wxCommandEvent&) {
    if (!PanelReady()) return;
    wxImage img = m_imagePanel->GetCurrentImage().Copy();
    if (!img.IsOk()) return;

    unsigned char* data = img.GetData();
    int n = img.GetWidth() * img.GetHeight() * 3;
    for (int i = 0; i < n; ++i)
        data[i] = 255 - data[i];

    m_imagePanel->SetImage(img);
    RefreshHistogram();
    PushHistory("Invert");
    SetStatusText("Applied Invert Colors filter.");
}

void ImageFrame::OnFilterSobel(wxCommandEvent&) {
    if (!PanelReady()) return;
    ShowEdgeWindows(Filtering::ApplySobel(m_imagePanel->GetCurrentImage()), "Sobel");
}

void ImageFrame::OnFilterRoberts(wxCommandEvent&) {
    if (!PanelReady()) return;
    ShowEdgeWindows(Filtering::ApplyRoberts(m_imagePanel->GetCurrentImage()), "Roberts");
}

void ImageFrame::OnFilterPrewitt(wxCommandEvent&) {
    if (!PanelReady()) return;
    ShowEdgeWindows(Filtering::ApplyPrewitt(m_imagePanel->GetCurrentImage()), "Prewitt");
}

void ImageFrame::OnFilterCanny(wxCommandEvent&) {
    if (!PanelReady()) return;
    m_imagePanel->SetImage(Filtering::ApplyCanny(m_imagePanel->GetCurrentImage()));
    RefreshHistogram();
    PushHistory("Canny");
    SetStatusText("Applied Canny Edge Detection (OpenCV).");
}

void ImageFrame::OnFilterEqualize(wxCommandEvent&) {
    if (!PanelReady()) return;
    m_imagePanel->SetImage(Filtering::EqualizeHistogram(m_imagePanel->GetCurrentImage()));
    RefreshHistogram();
    PushHistory("Equalize");
    SetStatusText("Applied Histogram Equalization.");
}

void ImageFrame::OnFilterNormalize(wxCommandEvent&) {
    if (!PanelReady()) return;
    m_imagePanel->SetImage(Filtering::NormalizeHistogram(m_imagePanel->GetCurrentImage()));
    RefreshHistogram();
    PushHistory("Normalize");
    SetStatusText("Applied Histogram Normalization.");
}

void ImageFrame::OnViewHistogram(wxCommandEvent&) {
    if (!PanelReady()) return;
    wxImage img = m_imagePanel->GetCurrentImage();
    if (!img.IsOk()) return;

    HistogramFrame* hf = new HistogramFrame(this, "RGB Histogram", img);
    hf->Show(true);
}

// ─────────────────────────────────────────────────────────────
// History sidebar: restore selected version
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnHistorySelect(wxCommandEvent& event) {
    int idx = event.GetSelection();
    if (idx < 0 || idx >= (int)m_historyImages.size()) return;

    m_imagePanel->SetImage(m_historyImages[idx]);
    RefreshHistogram();
    SetStatusText(wxString::Format("Restored: %s", m_historyLabels[idx]));
}
