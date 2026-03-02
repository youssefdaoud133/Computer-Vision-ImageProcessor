#include "ImageFrame.h"
#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/artprov.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>
#include <wx/busyinfo.h>
#include <algorithm>

// ============================================================================
// EVENT TABLE
// ============================================================================
wxBEGIN_EVENT_TABLE(ImageFrame, wxFrame)
    // Basic filter events
    EVT_TOOL(ID_RESET, ImageFrame::OnReset)
    EVT_TOOL(ID_FILTER_GRAYSCALE, ImageFrame::OnFilterGrayscale)
    EVT_TOOL(ID_FILTER_BLUR, ImageFrame::OnFilterBlur)
    EVT_TOOL(ID_FILTER_INVERT, ImageFrame::OnFilterInvert)
    // Frequency domain filter events
    EVT_TOOL(ID_LOW_PASS, ImageFrame::OnLowPassFilter)
    EVT_TOOL(ID_HIGH_PASS, ImageFrame::OnHighPassFilter)
    EVT_TOOL(ID_HYBRID, ImageFrame::OnHybridImage)
    EVT_TOOL(ID_SPECTRUM, ImageFrame::OnShowSpectrum)
wxEND_EVENT_TABLE()

// ============================================================================
// CONSTRUCTOR - From file path
// ============================================================================
ImageFrame::ImageFrame(const wxString& title, const wxString& imagePath)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_originalPath(imagePath) {

    // Create a status bar to show tips
    CreateStatusBar();
    SetStatusText("Use the toolbar above to apply filters to your image.");

    // Create Toolbar for filters
    CreateFilterToolbar();

    // Create Image Panel from file path
    m_imagePanel = new ImagePanel(this, imagePath);

    // Initialize frame layout
    InitializeFrame();
}

// ============================================================================
// CONSTRUCTOR - From wxImage object
// ============================================================================
ImageFrame::ImageFrame(const wxString& title, const wxImage& image)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
      m_originalPath("") {

    // Create a status bar
    CreateStatusBar();
    SetStatusText("Processed image - use toolbar for additional filters.");

    // Create Toolbar for filters
    CreateFilterToolbar();

    // Create Image Panel from wxImage
    m_imagePanel = new ImagePanel(this, "");
    m_imagePanel->SetImage(image);

    // Initialize frame layout
    InitializeFrame();
}

// ============================================================================
// COMMON INITIALIZATION
// ============================================================================
void ImageFrame::InitializeFrame() {
    // Setup sizer for proper resizing behavior
    wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);
    pSizer->Add(m_imagePanel, 1, wxEXPAND | wxALL, 0);
    SetSizerAndFit(pSizer);

    // Apply default window size
    SetSize(wxSize(800, 600));
}

// ============================================================================
// CREATE FILTER TOOLBAR
// ============================================================================
void ImageFrame::CreateFilterToolbar() {
    m_toolbar = CreateToolBar(wxTB_HORIZONTAL | wxNO_BORDER | wxTB_FLAT | wxTB_TEXT);
    
    // Get standard icons from wxArtProvider
    wxBitmap bmpReset = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR);
    wxBitmap bmpGray = wxArtProvider::GetBitmap(wxART_TICK_MARK, wxART_TOOLBAR);
    wxBitmap bmpBlur = wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_TOOLBAR);
    wxBitmap bmpInv = wxArtProvider::GetBitmap(wxART_WARNING, wxART_TOOLBAR);
    wxBitmap bmpFreq = wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_TOOLBAR);
    wxBitmap bmpHybrid = wxArtProvider::GetBitmap(wxART_PLUS, wxART_TOOLBAR);

    // ========================================
    // Basic Filter Tools
    // ========================================
    m_toolbar->AddTool(ID_RESET, "Reset", bmpReset, "Reset to Original Image");
    m_toolbar->AddTool(ID_FILTER_GRAYSCALE, "Grayscale", bmpGray, "Apply Grayscale Filter");
    m_toolbar->AddTool(ID_FILTER_BLUR, "Blur", bmpBlur, "Apply Blur Filter");
    m_toolbar->AddTool(ID_FILTER_INVERT, "Invert", bmpInv, "Apply Invert Colors Filter");

    // Add separator between basic and frequency filters
    m_toolbar->AddSeparator();

    // ========================================
    // Frequency Domain Filter Tools
    // ========================================
    m_toolbar->AddTool(ID_LOW_PASS, "Low Pass", bmpFreq, 
        "Apply Low Pass Filter (Frequency Domain) - Removes high frequencies, blurs image");
    m_toolbar->AddTool(ID_HIGH_PASS, "High Pass", bmpFreq, 
        "Apply High Pass Filter (Frequency Domain) - Removes low frequencies, shows edges");
    m_toolbar->AddTool(ID_HYBRID, "Hybrid", bmpHybrid, 
        "Create Hybrid Image - Combine two images using frequency components");
    m_toolbar->AddTool(ID_SPECTRUM, "Spectrum", bmpFreq, 
        "Show Frequency Spectrum - Visualize frequency content of image");

    m_toolbar->Realize();
}

// ============================================================================
// BASIC FILTERS
// ============================================================================

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
        SetStatusText("Applied Blur filter (spatial domain).");
    }
}

void ImageFrame::OnFilterInvert(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    wxImage current = m_imagePanel->GetCurrentImage();
    if (current.IsOk()) {
        // Create a copy to avoid modifying shared data
        current = current.Copy();

        // Invert all pixel values
        unsigned char* data = current.GetData();
        int dataSize = current.GetWidth() * current.GetHeight() * 3;
        for (int i = 0; i < dataSize; ++i) {
            data[i] = 255 - data[i];
        }
        
        m_imagePanel->SetImage(current);
        SetStatusText("Applied Invert Colors filter.");
    }
}

// ============================================================================
// FREQUENCY DOMAIN FILTERS
// ============================================================================

// ----------------------------------------------------------------------------
// LOW PASS FILTER
// ----------------------------------------------------------------------------
// Applies Gaussian Low Pass Filter to remove high frequency content
// User specifies cutoff frequency - lower values produce more blur
// ----------------------------------------------------------------------------
void ImageFrame::OnLowPassFilter(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    // Prompt user for cutoff frequency
    wxTextEntryDialog dialog(this, 
        "Enter cutoff frequency (10-100):\n"
        "Lower values produce more blur\n"
        "Recommended: 20-50",
        "Low Pass Filter Parameters", 
        "30");
    
    if (dialog.ShowModal() == wxID_OK) {
        double cutoff;
        if (dialog.GetValue().ToDouble(&cutoff)) {
            // Clamp cutoff to valid range
            cutoff = std::clamp(cutoff, 5.0, 200.0);
            
            wxImage currentImage = m_imagePanel->GetCurrentImage();
            
            if (currentImage.IsOk()) {
                // Show busy cursor and info during processing
                wxBusyCursor wait;
                wxBusyInfo info("Applying Low Pass Filter...\n"
                               "Processing DFT transformation...", this);
                
                // Apply the filter
                wxImage filtered = FrequencyFilters::ApplyLowPassFilter(currentImage, cutoff);
                
                // Update display
                m_imagePanel->SetImage(filtered);
                SetStatusText(wxString::Format("Applied Low Pass Filter (cutoff: %.1f)", cutoff));
            }
        } else {
            wxMessageBox("Invalid cutoff value. Please enter a number.", 
                        "Input Error", wxICON_WARNING);
        }
    }
}

// ----------------------------------------------------------------------------
// HIGH PASS FILTER
// ----------------------------------------------------------------------------
// Applies Gaussian High Pass Filter to extract edges and details
// User specifies cutoff frequency - higher values show more edges
// ----------------------------------------------------------------------------
void ImageFrame::OnHighPassFilter(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    // Prompt user for cutoff frequency
    wxTextEntryDialog dialog(this,
        "Enter cutoff frequency (10-100):\n"
        "Higher values show more edge detail\n"
        "Recommended: 20-60",
        "High Pass Filter Parameters", 
        "30");
    
    if (dialog.ShowModal() == wxID_OK) {
        double cutoff;
        if (dialog.GetValue().ToDouble(&cutoff)) {
            cutoff = std::clamp(cutoff, 5.0, 200.0);
            
            wxImage currentImage = m_imagePanel->GetCurrentImage();
            
            if (currentImage.IsOk()) {
                wxBusyCursor wait;
                wxBusyInfo info("Applying High Pass Filter...\n"
                               "Extracting edge information...", this);
                
                wxImage filtered = FrequencyFilters::ApplyHighPassFilter(currentImage, cutoff);
                
                m_imagePanel->SetImage(filtered);
                SetStatusText(wxString::Format("Applied High Pass Filter (cutoff: %.1f)", cutoff));
            }
        } else {
            wxMessageBox("Invalid cutoff value. Please enter a number.", 
                        "Input Error", wxICON_WARNING);
        }
    }
}

// ----------------------------------------------------------------------------
// HYBRID IMAGE CREATION
// ----------------------------------------------------------------------------
// Creates a hybrid image by combining:
// - Low frequency content from current image (visible from far)
// - High frequency content from second image (visible from close)
// This creates an optical illusion effect
// ----------------------------------------------------------------------------
void ImageFrame::OnHybridImage(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    // Step 1: Let user select the second image
    wxFileDialog openDialog(this, 
        "Select Second Image for Hybrid Effect",
        "", "", 
        "Image files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    
    if (openDialog.ShowModal() == wxID_CANCEL) {
        return;
    }
    
    // Load the second image
    wxImage secondImage;
    if (!secondImage.LoadFile(openDialog.GetPath())) {
        wxMessageBox("Failed to load the selected image!", 
                    "Error", wxICON_ERROR);
        return;
    }
    
    // Step 2: Get Low Pass cutoff for first image
    wxTextEntryDialog lowDialog(this,
        "Enter LOW pass cutoff for current image (10-50):\n"
        "This image will be visible from FAR away\n"
        "Lower value = smoother blend",
        "Hybrid Image - Low Frequency Component", 
        "20");
    
    if (lowDialog.ShowModal() != wxID_OK) {
        return;
    }
    
    double lowCutoff;
    if (!lowDialog.GetValue().ToDouble(&lowCutoff)) {
        lowCutoff = 20;
    }
    lowCutoff = std::clamp(lowCutoff, 5.0, 100.0);
    
    // Step 3: Get High Pass cutoff for second image
    wxTextEntryDialog highDialog(this,
        "Enter HIGH pass cutoff for second image (20-80):\n"
        "This image will be visible from CLOSE up\n"
        "Higher value = more detail visible",
        "Hybrid Image - High Frequency Component", 
        "40");
    
    if (highDialog.ShowModal() != wxID_OK) {
        return;
    }
    
    double highCutoff;
    if (!highDialog.GetValue().ToDouble(&highCutoff)) {
        highCutoff = 40;
    }
    highCutoff = std::clamp(highCutoff, 10.0, 150.0);
    
    // Step 4: Create the hybrid image
    wxImage currentImage = m_imagePanel->GetCurrentImage();
    
    if (currentImage.IsOk()) {
        wxBusyCursor wait;
        wxBusyInfo info("Creating Hybrid Image...\n"
                       "This may take a moment for large images...", this);
        
        // Generate hybrid image
        wxImage hybrid = FrequencyFilters::CreateHybridImage(
            currentImage,
            secondImage,
            lowCutoff, 
            highCutoff);
        
        // Open new window to display the hybrid result
        ImageFrame* hybridFrame = new ImageFrame("Hybrid Image Result", hybrid);
        hybridFrame->Show(true);
        
        SetStatusText("Hybrid image created in new window.");
        
        wxMessageBox(
            "Hybrid image created!\n\n"
            "View from FAR: You'll see the first image\n"
            "View from CLOSE: You'll see the second image\n\n"
            "Tip: Try squinting or stepping back from screen!",
            "Hybrid Image Created", 
            wxICON_INFORMATION);
    }
}

// ----------------------------------------------------------------------------
// FREQUENCY SPECTRUM VISUALIZATION
// ----------------------------------------------------------------------------
// Displays the magnitude spectrum of the current image
// Useful for analyzing frequency content and understanding filtering effects
// Center of spectrum = low frequencies (smooth areas)
// Edges of spectrum = high frequencies (details, edges)
// ----------------------------------------------------------------------------
void ImageFrame::OnShowSpectrum(wxCommandEvent& event) {
    if (!m_imagePanel) return;

    wxImage currentImage = m_imagePanel->GetCurrentImage();
    
    if (currentImage.IsOk()) {
        wxBusyCursor wait;
        wxBusyInfo info("Computing Frequency Spectrum...\n"
                       "Performing DFT analysis...", this);
        
        // Generate spectrum visualization
        wxImage spectrum = FrequencyFilters::GetFrequencySpectrum(currentImage);
        
        // Open new window to display the spectrum
        ImageFrame* spectrumFrame = new ImageFrame(
            "Frequency Spectrum - Magnitude", spectrum);
        spectrumFrame->Show(true);
        
        SetStatusText("Frequency spectrum displayed in new window.");
        
        wxMessageBox(
            "Frequency Spectrum Analysis:\n\n"
            "- CENTER (bright): Low frequencies (smooth areas)\n"
            "- EDGES (dark): High frequencies (edges, details)\n"
            "- Brighter spots = more energy at that frequency\n\n"
            "This visualization helps understand what the filters do!",
            "Spectrum Information", 
            wxICON_INFORMATION);
    }
}