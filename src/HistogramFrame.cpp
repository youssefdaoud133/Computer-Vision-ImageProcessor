#include "HistogramFrame.h"
#include <wx/sizer.h>

HistogramFrame::HistogramFrame(wxWindow* parent, const wxString& title, const wxImage& image)
    : wxFrame(parent, wxID_ANY, title, wxDefaultPosition, wxSize(600, 400)) {
    
    HistogramPanel* panel = new HistogramPanel(this, image);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(panel, 1, wxEXPAND);
    SetSizer(sizer);
    
    Centre();
}
