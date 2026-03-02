#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <vector>
#include <string>

/**
 * @class MainFrame
 * @brief The introductory window used for uploading files.
 *
 * This frame provides users with an interface to select single or
 * multiple images from the file system, opening each selected image
 * in its respective ImageFrame.
 */
class MainFrame : public wxFrame {
public:
    /**
     * @brief Constructs a new MainFrame object.
     * @param title Title of the main window.
     * @param pos Position of the main window.
     * @param size Size of the main window.
     */
    MainFrame(const wxString& title, const wxPoint& pos, const wxSize& size);

private:
    /**
     * @brief Event handler for the upload button click.
     * @param event The wxCommandEvent triggered by the button.
     */
    void OnUploadClicked(wxCommandEvent& event);

    /**
     * @brief Event handler for Hybrid Image creation.
     * Lets the user pick two images, assign low/high-pass roles, and opens the result.
     */
    void OnHybridClicked(wxCommandEvent& event);

    /**
     * @brief Event handler for application exit.
     * @param event The wxCommandEvent triggered by the menu.
     */
    void OnExit(wxCommandEvent& event);

    // Any wxWidgets event handling table needs this declaration
    wxDECLARE_EVENT_TABLE();

    // UI Elements
    wxButton* m_uploadBtn;
    wxButton* m_hybridBtn;
    wxStaticText* m_introText;
};

// Define Event IDs
enum {
    ID_UPLOAD = 1,
    ID_HYBRID
};

#endif // MAINFRAME_H
