#include "MainFrame.h"
#include "ImageFrame.h"
#include <wx/filedlg.h>
#include <wx/sizer.h>
#include <wx/filename.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_BUTTON(ID_UPLOAD, MainFrame::OnUploadClicked)
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

    m_introText = new wxStaticText(panel, wxID_ANY, wxT("Click below to upload images.\nEach image will open in a new window."));
    wxFont font = m_introText->GetFont();
    font.SetPointSize(14);
    font.MakeBold();
    m_introText->SetFont(font);

    m_uploadBtn = new wxButton(panel, ID_UPLOAD, "Upload Images");
    wxFont btnFont = m_uploadBtn->GetFont();
    btnFont.SetPointSize(12);
    m_uploadBtn->SetFont(btnFont);

    vbox->Add(m_introText, 1, wxALL | wxEXPAND, 50);
    vbox->Add(m_uploadBtn, 0, wxALIGN_CENTER_HORIZONTAL | wxBOTTOM, 50);

    panel->SetSizer(vbox);
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
