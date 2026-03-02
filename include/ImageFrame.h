#ifndef IMAGEFRAME_H
#define IMAGEFRAME_H

#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>
#include <string>
#include "ImagePanel.h"
#include "HistogramPanel.h"

/**
 * @class ImageFrame
 * @brief Main editing window for a single image.
 *
 * Layout:
 *   ┌─────────────────────────────┬──────────────────┐
 *   │                             │  Live Histogram   │
 *   │       Image Panel           │  (RGB + CDF)      │
 *   │                             ├──────────────────┤
 *   │                             │  History          │
 *   │                             │  (change log)     │
 *   └─────────────────────────────┴──────────────────┘
 *
 * The histogram updates automatically whenever a filter is applied.
 * The history log lets the user click any entry to restore that version.
 */
class ImageFrame : public wxFrame {
public:
    /**
     * @brief Constructs an ImageFrame.
     * @param title     Window title.
     * @param imagePath Absolute path to the image file to open.
     */
    ImageFrame(const wxString& title, const wxString& imagePath);

private:
    // ── UI construction ──────────────────────────────────────
    void CreateFilterToolbar();
    void CreateRightSidebar(wxWindow* parent, wxSizer* mainSizer);

    // ── Helpers ──────────────────────────────────────────────
    /** Returns true if m_imagePanel is valid. Used as a guard. */
    bool PanelReady() const;

    /**
     * @brief Saves the current image as a history snapshot.
     *        Appends an entry to the history list in the sidebar.
     * @param label Short description of the operation (e.g. "Grayscale").
     */
    void PushHistory(const wxString& label);

    /** Redraws the live histogram panel with the current image. */
    void RefreshHistogram();

    /**
     * @brief Shows two popup windows with X/Y gradient images.
     * @param results  {magnitude, gradX, gradY} from an edge detector.
     * @param filterName Name shown in the popup title (e.g. "Sobel").
     */
    void ShowEdgeWindows(const std::vector<wxImage>& results,
                         const wxString& filterName);

    // ── Toolbar filter handlers ──────────────────────────────
    void OnReset           (wxCommandEvent& event);
    void OnFilterGrayscale (wxCommandEvent& event);
    void OnFilterBlur      (wxCommandEvent& event);
    void OnFilterInvert    (wxCommandEvent& event);
    void OnFilterSobel     (wxCommandEvent& event);
    void OnFilterRoberts   (wxCommandEvent& event);
    void OnFilterPrewitt   (wxCommandEvent& event);
    void OnFilterCanny     (wxCommandEvent& event);
    void OnFilterEqualize  (wxCommandEvent& event);
    void OnFilterNormalize (wxCommandEvent& event);
    void OnViewHistogram   (wxCommandEvent& event);

    // ── History sidebar handler ──────────────────────────────
    void OnHistorySelect(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();

    // ── UI elements ──────────────────────────────────────────
    ImagePanel*    m_imagePanel;
    wxToolBar*     m_toolbar;
    HistogramPanel* m_histPanel;   ///< Live RGB histogram in right sidebar
    wxListBox*     m_historyList;  ///< Scrollable change-log list

    // ── State ────────────────────────────────────────────────
    wxString               m_originalPath;
    std::vector<wxImage>   m_historyImages;  ///< Saved image at each step
    std::vector<wxString>  m_historyLabels;  ///< Operation label for each step
};

// ── Toolbar / filter IDs ─────────────────────────────────────
enum {
    ID_RESET             = 100,
    ID_FILTER_GRAYSCALE,
    ID_FILTER_BLUR,
    ID_FILTER_INVERT,
    ID_FILTER_SOBEL,
    ID_FILTER_ROBERTS,
    ID_FILTER_PREWITT,
    ID_FILTER_CANNY,
    ID_FILTER_EQUALIZE,
    ID_FILTER_NORMALIZE,
    ID_VIEW_HISTOGRAM,
    ID_HISTORY_SELECT
};

#endif // IMAGEFRAME_H
