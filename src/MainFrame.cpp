#include "MainFrame.h"
#include "ImageFrame.h"
#include "Filtering.h"
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/filename.h>
#include <wx/choice.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_UPLOAD, MainFrame::OnUploadClicked)
    EVT_BUTTON(ID_HYBRID, MainFrame::OnHybridClicked)
    EVT_MENU(wxID_EXIT, MainFrame::OnExit)
wxEND_EVENT_TABLE()

MainFrame::MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(NULL, wxID_ANY, title, pos, size) {
    
    // Set up standard menubar with Exit option
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT, "E&xit");
    
    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    CreateStatusBar();
    SetStatusText("Welcome to Image Processor!");

    // Layout the intro panel
    wxPanel* panel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

    m_introText = new wxStaticText(panel, wxID_ANY, wxT("Click below to upload images.\nEach image will open in a new window."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    wxFont font = m_introText->GetFont();
    font.SetPointSize(14);
    font.MakeBold();
    m_introText->SetFont(font);

    m_uploadBtn = new wxButton(panel, ID_UPLOAD, "Upload Images", wxDefaultPosition, wxSize(200, 50));
    wxFont btnFont = m_uploadBtn->GetFont();
    btnFont.SetPointSize(12);
    m_uploadBtn->SetFont(btnFont);

    m_hybridBtn = new wxButton(panel, ID_HYBRID, "Hybrid Image", wxDefaultPosition, wxSize(200, 50));
    m_hybridBtn->SetFont(btnFont);

    // Use spacers to center elements vertically
    vbox->AddStretchSpacer(1);
    vbox->Add(m_introText, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 20);
    vbox->Add(m_uploadBtn, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 20);
    vbox->Add(m_hybridBtn, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 10);
    vbox->AddStretchSpacer(1);

    panel->SetSizer(vbox);

    // Set a sizer for the frame to manage the panel correctly
    wxBoxSizer* frameSizer = new wxBoxSizer(wxVERTICAL);
    frameSizer->Add(panel, 1, wxEXPAND);
    SetSizer(frameSizer);

    Centre();
}

void MainFrame::OnUploadClicked(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, _("Open Image Files"), "", "",
                                "Image files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp", 
                                wxFD_OPEN | wxFD_FILE_MUST_EXIST | wxFD_MULTIPLE);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;     // the user changed idea...

    // Get the selection of paths
    wxArrayString paths;
    openFileDialog.GetPaths(paths);

    // Open each path in a new frame
    for (size_t i = 0; i < paths.GetCount(); i++) {
        wxFileName fn(paths[i]);
        ImageFrame* imgFrame = new ImageFrame("Image View - " + fn.GetFullName(), paths[i]);
        imgFrame->Show(true);
    }
}

void MainFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MainFrame::OnHybridClicked(wxCommandEvent&) {
    // --- Pick Image A ---
    wxFileDialog dlgA(this, "Select Image A", "", "",
        "Image files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlgA.ShowModal() == wxID_CANCEL) return;
    wxString pathA = dlgA.GetPath();

    // --- Pick Image B ---
    wxFileDialog dlgB(this, "Select Image B", "", "",
        "Image files (*.png;*.jpg;*.jpeg;*.bmp)|*.png;*.jpg;*.jpeg;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlgB.ShowModal() == wxID_CANCEL) return;
    wxString pathB = dlgB.GetPath();

    // --- Parameters dialog: choose roles + kernel/sigma ---
    wxDialog paramDlg(this, wxID_ANY, "Hybrid Image Parameters",
                      wxDefaultPosition, wxSize(420, 320));
    wxBoxSizer* vs = new wxBoxSizer(wxVERTICAL);

    // File labels
    wxFileName fnA(pathA), fnB(pathB);
    vs->Add(new wxStaticText(&paramDlg, wxID_ANY,
        wxString::Format("A: %s\nB: %s", fnA.GetFullName(), fnB.GetFullName())),
        0, wxALL, 10);

    wxFlexGridSizer* gs = new wxFlexGridSizer(4, 2, 6, 10);
    gs->AddGrowableCol(1);

    // Role selector
    wxArrayString roles;
    roles.Add("A = Low-pass,  B = High-pass");
    roles.Add("A = High-pass, B = Low-pass");
    wxChoice* roleChoice = new wxChoice(&paramDlg, wxID_ANY, wxDefaultPosition,
                                        wxDefaultSize, roles);
    roleChoice->SetSelection(0);
    gs->Add(new wxStaticText(&paramDlg, wxID_ANY, "Role:"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(roleChoice, 1, wxEXPAND);

    // Filter type
    wxArrayString filterTypes;
    filterTypes.Add("Ideal (hard circle)");
    filterTypes.Add("Gaussian (smooth)");
    wxChoice* filterChoice = new wxChoice(&paramDlg, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, filterTypes);
    filterChoice->SetSelection(1); // default Gaussian
    gs->Add(new wxStaticText(&paramDlg, wxID_ANY, "Filter type:"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(filterChoice, 1, wxEXPAND);

    // Low cutoff frequency
    wxTextCtrl* tcLowCutoff = new wxTextCtrl(&paramDlg, wxID_ANY, "30");
    gs->Add(new wxStaticText(&paramDlg, wxID_ANY, "Low cutoff (px):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcLowCutoff, 1, wxEXPAND);

    // High cutoff frequency
    wxTextCtrl* tcHighCutoff = new wxTextCtrl(&paramDlg, wxID_ANY, "60");
    gs->Add(new wxStaticText(&paramDlg, wxID_ANY, "High cutoff (px):"), 0, wxALIGN_CENTER_VERTICAL);
    gs->Add(tcHighCutoff, 1, wxEXPAND);

    vs->Add(gs, 0, wxEXPAND | wxALL, 12);
    vs->Add(paramDlg.CreateButtonSizer(wxOK | wxCANCEL), 0, wxEXPAND | wxALL, 8);
    paramDlg.SetSizer(vs);
    paramDlg.Centre();

    if (paramDlg.ShowModal() != wxID_OK) return;

    double lowCutoff = 30.0;
    double highCutoff = 60.0;
    int filterType = filterChoice->GetSelection(); // 0=Ideal, 1=Gaussian
    tcLowCutoff->GetValue().ToDouble(&lowCutoff);
    tcHighCutoff->GetValue().ToDouble(&highCutoff);
    if (lowCutoff < 1.0)  lowCutoff  = 1.0;
    if (highCutoff < 1.0) highCutoff = 1.0;

    // Load images
    wxImage imgA(pathA, wxBITMAP_TYPE_ANY);
    wxImage imgB(pathB, wxBITMAP_TYPE_ANY);
    if (!imgA.IsOk() || !imgB.IsOk()) {
        wxMessageBox("Failed to load one or both images.", "Error", wxICON_ERROR);
        return;
    }

    // Determine low-pass and high-pass sources based on role selection
    wxImage lowSrc, highSrc;
    if (roleChoice->GetSelection() == 0) {
        // A = low, B = high
        lowSrc  = imgA;
        highSrc = imgB;
    } else {
        // A = high, B = low
        lowSrc  = imgB;
        highSrc = imgA;
    }

    SetStatusText("Computing Hybrid Image -- please wait...");
    Update();

    wxImage hybrid = Filtering::HybridImage(lowSrc, highSrc, lowCutoff, highCutoff, filterType);
    if (!hybrid.IsOk()) {
        wxMessageBox("Hybrid image generation failed.", "Error", wxICON_ERROR);
        SetStatusText("Hybrid image failed.");
        return;
    }

    // Save the result to a temp file and open it in an ImageFrame
    wxString tmpPath = wxFileName::CreateTempFileName("hybrid");
    tmpPath += ".png";
    hybrid.SaveFile(tmpPath, wxBITMAP_TYPE_PNG);

    ImageFrame* frame = new ImageFrame("Hybrid Image Result", tmpPath);
    frame->Show(true);
    SetStatusText("Hybrid image opened in new window.");
}
