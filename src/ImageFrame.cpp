// ImageFrame.cpp
#include "ImageFrame.h"
#include "Filtering.h"
#include "HistogramFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <wx/statline.h>
#include <wx/button.h>
#include <algorithm>

// ─────────────────────────────────────────────────────────────
// Event table
// ─────────────────────────────────────────────────────────────
wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET,              ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE,   ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR,        ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT,      ImageFrame::OnFilterInvert)
    EVT_TOOL(ID_FILTER_SOBEL,       ImageFrame::OnFilterSobel)
    EVT_TOOL(ID_FILTER_ROBERTS,     ImageFrame::OnFilterRoberts)
    EVT_TOOL(ID_FILTER_PREWITT,     ImageFrame::OnFilterPrewitt)
    EVT_TOOL(ID_FILTER_CANNY,       ImageFrame::OnFilterCanny)
    EVT_TOOL(ID_FILTER_EQUALIZE,    ImageFrame::OnFilterEqualize)
    EVT_TOOL(ID_FILTER_NORMALIZE,   ImageFrame::OnFilterNormalize)
    EVT_TOOL(ID_VIEW_HISTOGRAM,     ImageFrame::OnViewHistogram)
    EVT_TOOL(ID_NOISE_UNIFORM,      ImageFrame::OnNoiseUniform)
    EVT_TOOL(ID_NOISE_GAUSSIAN,     ImageFrame::OnNoiseGaussian)
    EVT_TOOL(ID_NOISE_SALTPEPPER,   ImageFrame::OnNoiseSaltPepper)
    EVT_TOOL(ID_LOWPASS_AVERAGE,    ImageFrame::OnLowPassAverage)
    EVT_TOOL(ID_LOWPASS_GAUSSIAN,   ImageFrame::OnLowPassGaussian)
    EVT_TOOL(ID_LOWPASS_MEDIAN,     ImageFrame::OnLowPassMedian)
    EVT_TOOL(ID_FILTER_LOWFREQ,     ImageFrame::OnFilterLowFreq)
    EVT_TOOL(ID_FILTER_HIGHFREQ,    ImageFrame::OnFilterHighFreq)
    EVT_LISTBOX(ID_HISTORY_SELECT,  ImageFrame::OnHistorySelect)
    EVT_BUTTON(ID_HISTORY_RESTORE,  ImageFrame::OnHistoryRestore)
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

    // Image panel (takes 3 parts of horizontal space)
    m_imagePanel = new ImagePanel(contentPanel, imagePath);
    mainSizer->Add(m_imagePanel, 3, wxEXPAND);

    // Right sidebar (takes 1 part — grows proportionally in fullscreen)
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

    // ── Basic filters ──────────────────────────────────────────
    m_toolbar->AddTool(ID_RESET,            "Reset",     wxArtProvider::GetBitmap(wxART_UNDO,         wxART_TOOLBAR), "Reset to Original Image");
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", wxArtProvider::GetBitmap(wxART_NEW,          wxART_TOOLBAR), "Convert to Grayscale");
    m_toolbar->AddTool(ID_FILTER_BLUR,      "Blur",      wxArtProvider::GetBitmap(wxART_REDO,         wxART_TOOLBAR), "Apply Gaussian Blur (wxWidgets)");
    m_toolbar->AddTool(ID_FILTER_INVERT,    "Invert",    wxArtProvider::GetBitmap(wxART_WARNING,      wxART_TOOLBAR), "Invert Colors");
    m_toolbar->AddSeparator();

    // ── Edge detection ────────────────────────────────────────
    m_toolbar->AddTool(ID_FILTER_SOBEL,     "Sobel",     wxArtProvider::GetBitmap(wxART_ADD_BOOKMARK, wxART_TOOLBAR), "Sobel Edge Detection");
    m_toolbar->AddTool(ID_FILTER_ROBERTS,   "Roberts",   wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR), "Roberts Edge Detection");
    m_toolbar->AddTool(ID_FILTER_PREWITT,   "Prewitt",   wxArtProvider::GetBitmap(wxART_GOTO_FIRST,   wxART_TOOLBAR), "Prewitt Edge Detection");
    m_toolbar->AddTool(ID_FILTER_CANNY,     "Canny",     wxArtProvider::GetBitmap(wxART_HARDDISK,     wxART_TOOLBAR), "Canny Edge Detection (OpenCV)");
    m_toolbar->AddSeparator();

    // ── Histogram tools ───────────────────────────────────────
    m_toolbar->AddTool(ID_FILTER_EQUALIZE,  "Equalize",  wxArtProvider::GetBitmap(wxART_GO_FORWARD,   wxART_TOOLBAR), "Equalize Histogram");
    m_toolbar->AddTool(ID_FILTER_NORMALIZE, "Normalize", wxArtProvider::GetBitmap(wxART_GO_DOWN,      wxART_TOOLBAR), "Normalize Histogram (Contrast Stretch)");
    m_toolbar->AddTool(ID_VIEW_HISTOGRAM,   "Histogram", wxArtProvider::GetBitmap(wxART_GO_UP,        wxART_TOOLBAR), "Open Histogram in Popup Window");
    m_toolbar->AddSeparator();

    // ── Noise ───────────────────────────────────────────────
    m_toolbar->AddTool(ID_NOISE_UNIFORM,    "Unif.Noise",  wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR), "Add Uniform Noise (set range)");
    m_toolbar->AddTool(ID_NOISE_GAUSSIAN,   "Gauss.Noise", wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR), "Add Gaussian Noise (set mean & stddev)");
    m_toolbar->AddTool(ID_NOISE_SALTPEPPER, "S&P Noise",   wxArtProvider::GetBitmap(wxART_MISSING_IMAGE, wxART_TOOLBAR), "Add Salt & Pepper Noise (set density)");
    m_toolbar->AddSeparator();

    // ── Low-pass filters ───────────────────────────────────────
    m_toolbar->AddTool(ID_LOWPASS_AVERAGE,  "Avg.Filter",  wxArtProvider::GetBitmap(wxART_GO_BACK,      wxART_TOOLBAR), "Average (Box) Filter — set kernel size");
    m_toolbar->AddTool(ID_LOWPASS_GAUSSIAN, "Gauss.Flt.",  wxArtProvider::GetBitmap(wxART_GOTO_LAST,    wxART_TOOLBAR), "Gaussian Low-pass Filter — set kernel & sigma");
    m_toolbar->AddTool(ID_LOWPASS_MEDIAN,   "Med.Filter",  wxArtProvider::GetBitmap(wxART_TICK_MARK,    wxART_TOOLBAR), "Median Filter — set kernel size");
    m_toolbar->AddSeparator();

    // ── Frequency-domain filters ───────────────────────────────
    m_toolbar->AddTool(ID_FILTER_LOWFREQ,   "LowFreq",    wxArtProvider::GetBitmap(wxART_GO_DOWN,      wxART_TOOLBAR), "Low-Frequency Filter (Gaussian blur)");
    m_toolbar->AddTool(ID_FILTER_HIGHFREQ,  "HighFreq",   wxArtProvider::GetBitmap(wxART_GO_UP,        wxART_TOOLBAR), "High-Frequency Filter (edge detail)");

    m_toolbar->Realize();
}

void ImageFrame::CreateRightSidebar(wxWindow* parent, wxSizer* mainSizer) {
    // No fixed pixel width — sidebar grows proportionally with the window
    wxPanel* sidebar = new wxPanel(parent, wxID_ANY);
    sidebar->SetMinSize(wxSize(240, -1));
    sidebar->SetBackgroundColour(wxColour(245, 245, 248));

    wxBoxSizer* sbSizer = new wxBoxSizer(wxVERTICAL);

    // ── Histogram section ─────────────────────────────────────
    wxStaticText* histLabel = new wxStaticText(sidebar, wxID_ANY, "Live Histogram");
    histLabel->SetFont(histLabel->GetFont().Bold());
    sbSizer->Add(histLabel, 0, wxLEFT | wxTOP, 8);

    m_histPanel = new HistogramPanel(sidebar, m_imagePanel->GetCurrentImage());
    m_histPanel->SetMinSize(wxSize(220, 220));
    sbSizer->Add(m_histPanel, 3, wxEXPAND | wxALL, 4);

    sbSizer->Add(new wxStaticLine(sidebar), 0, wxEXPAND | wxLEFT | wxRIGHT, 6);

    // ── History section ───────────────────────────────────────
    wxStaticText* historyLabel = new wxStaticText(sidebar, wxID_ANY, "History");
    historyLabel->SetFont(historyLabel->GetFont().Bold());
    sbSizer->Add(historyLabel, 0, wxLEFT | wxTOP, 8);

    wxStaticText* hint = new wxStaticText(sidebar, wxID_ANY,
        "Click an entry to restore it.\nFuture edits after it will be removed.");
    hint->SetFont(hint->GetFont().Smaller());
    hint->SetForegroundColour(wxColour(120, 120, 120));
    sbSizer->Add(hint, 0, wxLEFT | wxBOTTOM, 8);

    m_historyList = new wxListBox(sidebar, ID_HISTORY_SELECT,
                                  wxDefaultPosition, wxDefaultSize,
                                  0, nullptr, wxLB_SINGLE | wxLB_HSCROLL);
    sbSizer->Add(m_historyList, 2, wxEXPAND | wxALL, 4);

    sidebar->SetSizer(sbSizer);
    mainSizer->Add(sidebar, 1, wxEXPAND);
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

    // Check if we are currently viewing an older version (not the last one)
    int currentIdx = m_historyList->GetSelection();
    if (currentIdx != wxNOT_FOUND && currentIdx < (int)m_historyImages.size() - 1) {
        // Truncate future branch
        m_historyImages.erase(m_historyImages.begin() + currentIdx + 1, m_historyImages.end());
        m_historyLabels.erase(m_historyLabels.begin() + currentIdx + 1, m_historyLabels.end());
        
        // Refresh ListBox
        m_historyList->Clear();
        for (size_t i = 0; i < m_historyLabels.size(); ++i) {
            m_historyList->Append(wxString::Format("%zu. %s", i + 1, m_historyLabels[i]));
        }
    }

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
// History sidebar: click = restore + truncate future entries
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnHistorySelect(wxCommandEvent&) {
    int idx = m_historyList->GetSelection();
    if (idx == wxNOT_FOUND || idx >= (int)m_historyImages.size()) return;

    // Restore the selected image
    m_imagePanel->SetImage(m_historyImages[idx]);
    RefreshHistogram();
    SetStatusText(wxString::Format("Restored to: %s", m_historyLabels[idx]));

    // DO NOT truncate here anymore. 
    // Truncation will happen in PushHistory if we are not at the end.
}

// Restore button (kept for compatibility but history click already does it)
void ImageFrame::OnHistoryRestore(wxCommandEvent&) {
    wxCommandEvent dummy;
    OnHistorySelect(dummy);
}

// ─────────────────────────────────────────────────────────────
// Noise handlers — each shows a parameter dialog before applying
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnNoiseUniform(wxCommandEvent&) {
    if (!PanelReady()) return;

    // Parameter dialog — low and high offsets
    wxDialog dlg(this, wxID_ANY, "Uniform Noise Parameters",
                 wxDefaultPosition, wxSize(300, 180));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(2, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcLow  = new wxTextCtrl(&dlg, wxID_ANY, "-30");
    wxTextCtrl* tcHigh = new wxTextCtrl(&dlg, wxID_ANY,  "30");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Low offset [-255..0]:"),  0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcLow,  1, wxEXPAND);
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "High offset [0..255]:"),  0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcHigh, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long low = -30, high = 30;
    tcLow->GetValue().ToLong(&low);
    tcHigh->GetValue().ToLong(&high);
    low  = std::clamp((int)low,  -255, 0);
    high = std::clamp((int)high,   0, 255);

    m_imagePanel->SetImage(Filtering::AddUniformNoise(m_imagePanel->GetCurrentImage(), (int)low, (int)high));
    RefreshHistogram();
    PushHistory(wxString::Format("Uniform Noise [%ld,%ld]", low, high));
    SetStatusText(wxString::Format("Applied Uniform Noise [%ld, %ld].", low, high));
}

void ImageFrame::OnNoiseGaussian(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Gaussian Noise Parameters",
                 wxDefaultPosition, wxSize(300, 180));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(2, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcMean   = new wxTextCtrl(&dlg, wxID_ANY, "0");
    wxTextCtrl* tcStddev = new wxTextCtrl(&dlg, wxID_ANY, "25");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Mean:"),   0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcMean,   1, wxEXPAND);
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Std Dev:"),0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcStddev, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    double mean = 0, stddev = 25;
    tcMean->GetValue().ToDouble(&mean);
    tcStddev->GetValue().ToDouble(&stddev);
    stddev = std::max(0.1, stddev);

    m_imagePanel->SetImage(Filtering::AddGaussianNoise(m_imagePanel->GetCurrentImage(), mean, stddev));
    RefreshHistogram();
    PushHistory(wxString::Format("Gaussian Noise (s=%.1f)", stddev));
    SetStatusText(wxString::Format("Applied Gaussian Noise (mean=%.1f, s=%.1f).", mean, stddev));
}

void ImageFrame::OnNoiseSaltPepper(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Salt & Pepper Noise Parameters",
                 wxDefaultPosition, wxSize(300, 150));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(1, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcDensity = new wxTextCtrl(&dlg, wxID_ANY, "0.05");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Density (0-1):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcDensity, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    double density = 0.05;
    tcDensity->GetValue().ToDouble(&density);
    density = std::clamp(density, 0.0, 1.0);

    m_imagePanel->SetImage(Filtering::AddSaltPepperNoise(m_imagePanel->GetCurrentImage(), density));
    RefreshHistogram();
    PushHistory(wxString::Format("S&P Noise (d=%.2f)", density));
    SetStatusText(wxString::Format("Applied Salt & Pepper Noise (density=%.2f).", density));
}

// ─────────────────────────────────────────────────────────────
// Low-pass filter handlers — each shows a parameter dialog
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnLowPassAverage(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Average Filter",
                 wxDefaultPosition, wxSize(280, 140));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(1, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcSize = new wxTextCtrl(&dlg, wxID_ANY, "5");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Kernel size (odd):"  ), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSize, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long kSize = 5;
    tcSize->GetValue().ToLong(&kSize);
    kSize = std::clamp((int)kSize, 3, 31);

    m_imagePanel->SetImage(Filtering::FilterAverage(m_imagePanel->GetCurrentImage(), (int)kSize));
    RefreshHistogram();
    PushHistory(wxString::Format("Average Filter (%ldx%ld)", kSize, kSize));
    SetStatusText(wxString::Format("Applied Average Filter (%ldx%ld).", kSize, kSize));
}

void ImageFrame::OnLowPassGaussian(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Gaussian Low-pass Filter",
                 wxDefaultPosition, wxSize(300, 180));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(2, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcSize  = new wxTextCtrl(&dlg, wxID_ANY, "5");
    wxTextCtrl* tcSigma = new wxTextCtrl(&dlg, wxID_ANY, "1.4");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Kernel size (odd):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSize,  1, wxEXPAND);
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Sigma:"),              0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSigma, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long kSize = 5;
    double sigma = 1.4;
    tcSize->GetValue().ToLong(&kSize);
    tcSigma->GetValue().ToDouble(&sigma);
    kSize = std::clamp((int)kSize, 3, 31);
    sigma = std::max(0.1, sigma);

    m_imagePanel->SetImage(Filtering::FilterGaussian(m_imagePanel->GetCurrentImage(), (int)kSize, sigma));
    RefreshHistogram();
    PushHistory(wxString::Format("Gaussian Filter (%ldx%ld, s=%.1f)", kSize, kSize, sigma));
    SetStatusText(wxString::Format("Applied Gaussian LPF (%ldx%ld, s=%.1f).", kSize, kSize, sigma));
}

void ImageFrame::OnLowPassMedian(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Median Filter",
                 wxDefaultPosition, wxSize(280, 140));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(1, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcSize = new wxTextCtrl(&dlg, wxID_ANY, "5");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Kernel size (odd):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSize, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long kSize = 5;
    tcSize->GetValue().ToLong(&kSize);
    kSize = std::clamp((int)kSize, 3, 31);

    SetStatusText("Applying Median Filter — please wait...");
    Update(); // flush status text before the (potentially slow) operation
    m_imagePanel->SetImage(Filtering::FilterMedian(m_imagePanel->GetCurrentImage(), (int)kSize));
    RefreshHistogram();
    PushHistory(wxString::Format("Median Filter (%ldx%ld)", kSize, kSize));
    SetStatusText(wxString::Format("Applied Median Filter (%ldx%ld).", kSize, kSize));
}

// ─────────────────────────────────────────────────────────────
// Frequency-domain filter handlers
// ─────────────────────────────────────────────────────────────
void ImageFrame::OnFilterLowFreq(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "Low-Frequency Filter",
                 wxDefaultPosition, wxSize(300, 180));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(2, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcSize  = new wxTextCtrl(&dlg, wxID_ANY, "5");
    wxTextCtrl* tcSigma = new wxTextCtrl(&dlg, wxID_ANY, "2.0");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Kernel size (odd):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSize,  1, wxEXPAND);
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Sigma:"),              0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSigma, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long kSize = 5;
    double sigma = 2.0;
    tcSize->GetValue().ToLong(&kSize);
    tcSigma->GetValue().ToDouble(&sigma);
    kSize = std::clamp((int)kSize, 3, 31);
    sigma = std::max(0.1, sigma);

    m_imagePanel->SetImage(Filtering::FilterLowFreq(m_imagePanel->GetCurrentImage(), (int)kSize, sigma));
    RefreshHistogram();
    PushHistory(wxString::Format("LowFreq (%ldx%ld, s=%.1f)", kSize, kSize, sigma));
    SetStatusText(wxString::Format("Applied Low-Frequency Filter (%ldx%ld, s=%.1f).", kSize, kSize, sigma));
}

void ImageFrame::OnFilterHighFreq(wxCommandEvent&) {
    if (!PanelReady()) return;

    wxDialog dlg(this, wxID_ANY, "High-Frequency Filter",
                 wxDefaultPosition, wxSize(300, 180));
    wxBoxSizer* s = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gs = new wxFlexGridSizer(2, 2, 6, 10);
    gs->AddGrowableCol(1);

    wxTextCtrl* tcSize  = new wxTextCtrl(&dlg, wxID_ANY, "5");
    wxTextCtrl* tcSigma = new wxTextCtrl(&dlg, wxID_ANY, "2.0");
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Kernel size (odd):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSize,  1, wxEXPAND);
    gs->Add(new wxStaticText(&dlg, wxID_ANY, "Sigma:"),              0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcSigma, 1, wxEXPAND);

    s->Add(gs, 0, wxEXPAND | wxALL, 12);
    s->Add(dlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    dlg.SetSizer(s);
    dlg.Centre();

    if (dlg.ShowModal() != wxID_OK) return;

    long kSize = 5;
    double sigma = 2.0;
    tcSize->GetValue().ToLong(&kSize);
    tcSigma->GetValue().ToDouble(&sigma);
    kSize = std::clamp((int)kSize, 3, 31);
    sigma = std::max(0.1, sigma);

    m_imagePanel->SetImage(Filtering::FilterHighFreq(m_imagePanel->GetCurrentImage(), (int)kSize, sigma));
    RefreshHistogram();
    PushHistory(wxString::Format("HighFreq (%ldx%ld, s=%.1f)", kSize, kSize, sigma));
    SetStatusText(wxString::Format("Applied High-Frequency Filter (%ldx%ld, s=%.1f).", kSize, kSize, sigma));
}
