#include "ImageFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include "ImageProcessor.h"

// Event table combining existing and new tools
wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    EVT_TOOL(ID_RESET, ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE, ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR, ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT, ImageFrame::OnFilterInvert)
    // New noise tools
    EVT_TOOL(ID_NOISE_UNIFORM, ImageFrame::OnNoiseUniform)
    EVT_TOOL(ID_NOISE_GAUSSIAN, ImageFrame::OnNoiseGaussian)
    EVT_TOOL(ID_NOISE_SALT_PEPPER, ImageFrame::OnNoiseSaltPepper)
    // New filter tools
    EVT_TOOL(ID_FILTER_AVERAGE, ImageFrame::OnFilterAverage)
    EVT_TOOL(ID_FILTER_GAUSSIAN, ImageFrame::OnFilterGaussian)
    EVT_TOOL(ID_FILTER_MEDIAN, ImageFrame::OnFilterMedian)
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
    
    // Existing tools
    wxBitmap bmpReset = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR);
    wxBitmap bmpGray = wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_TOOLBAR);
    wxBitmap bmpBlur = wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR);
    wxBitmap bmpInv = wxArtProvider::GetBitmap(wxART_WARNING, wxART_TOOLBAR);

    m_toolbar->AddTool(ID_RESET, "Reset", bmpReset, "Reset to Original Image");
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", bmpGray, "Apply Grayscale Filter");
    m_toolbar->AddTool(ID_FILTER_BLUR, "Blur", bmpBlur, "Apply Blur Filter");
    m_toolbar->AddTool(ID_FILTER_INVERT, "Invert", bmpInv, "Apply Invert Image Filter");

    // Separator and new noise tools
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_NOISE_UNIFORM, "Uniform Noise", bmpGray, "Add uniform noise");
    m_toolbar->AddTool(ID_NOISE_GAUSSIAN, "Gaussian Noise", bmpGray, "Add Gaussian noise");
    m_toolbar->AddTool(ID_NOISE_SALT_PEPPER, "S&P Noise", bmpGray, "Add salt & pepper noise");

    // Separator and new filter tools
    m_toolbar->AddSeparator();
    m_toolbar->AddTool(ID_FILTER_AVERAGE, "Average Filter", bmpBlur, "Apply average filter");
    m_toolbar->AddTool(ID_FILTER_GAUSSIAN, "Gaussian Filter", bmpBlur, "Apply Gaussian filter");
    m_toolbar->AddTool(ID_FILTER_MEDIAN, "Median Filter", bmpBlur, "Apply median filter");

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
        wxImage blur = current.Blur(5);
        m_imagePanel->SetImage(blur);
        SetStatusText("Applied Blur filter.");
    }
}

void ImageFrame::OnFilterInvert(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        current = current.Copy();
        unsigned char* data = current.GetData();
        int dataSize = current.GetWidth() * current.GetHeight() * 3;
        for (int i = 0; i < dataSize; ++i) {
            data[i] = 255 - data[i];
        }
        m_imagePanel->SetImage(current);
        SetStatusText("Applied Invert Colors filter.");
    }
}

// ==================== New OpenCV-based handlers ====================

void ImageFrame::OnNoiseUniform(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::AddUniformNoise(current, 50);
        m_imagePanel->SetImage(result);
        SetStatusText("Added uniform noise.");
    }
}

void ImageFrame::OnNoiseGaussian(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::AddGaussianNoise(current, 0, 25);
        m_imagePanel->SetImage(result);
        SetStatusText("Added Gaussian noise.");
    }
}

void ImageFrame::OnNoiseSaltPepper(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::AddSaltPepperNoise(current, 0.01);
        m_imagePanel->SetImage(result);
        SetStatusText("Added salt & pepper noise.");
    }
}

void ImageFrame::OnFilterAverage(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::ApplyAverageFilter(current, 5);
        m_imagePanel->SetImage(result);
        SetStatusText("Applied average filter.");
    }
}

void ImageFrame::OnFilterGaussian(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::ApplyGaussianFilter(current, 5, 1.5);
        m_imagePanel->SetImage(result);
        SetStatusText("Applied Gaussian filter.");
    }
}

void ImageFrame::OnFilterMedian(wxCommandEvent& event) {
    if (!m_imagePanel) return;
    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        wxImage result = ImageProcessor::ApplyMedianFilter(current, 5);
        m_imagePanel->SetImage(result);
        SetStatusText("Applied median filter.");
    }
}