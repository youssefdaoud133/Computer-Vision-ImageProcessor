#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit() {
    wxInitAllImageHandlers(); // Essential to load diverse image formats
    MainFrame* frame = new MainFrame("Image Processor - Intro", wxPoint(50, 50), wxSize(600, 400));
    frame->Show(true);
    return true;
}
