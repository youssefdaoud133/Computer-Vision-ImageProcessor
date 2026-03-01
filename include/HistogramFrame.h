#ifndef HISTOGRAMFRAME_H
#define HISTOGRAMFRAME_H

#include <wx/wx.h>
#include "HistogramPanel.h"

/**
 * @class HistogramFrame
 * @brief Standalone window for displaying image histograms.
 */
class HistogramFrame : public wxFrame {
public:
    /**
     * @brief Constructs a HistogramFrame.
     * @param parent Parent window.
     * @param title Window title.
     * @param image The image to analyze.
     */
    HistogramFrame(wxWindow* parent, const wxString& title, const wxImage& image);
};

#endif // HISTOGRAMFRAME_H
