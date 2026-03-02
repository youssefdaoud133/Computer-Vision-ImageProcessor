#ifndef HISTOGRAMFRAME_H
#define HISTOGRAMFRAME_H

#include <wx/wx.h>
#include "HistogramPanel.h"

/**
 * @class HistogramFrame
 * @brief Standalone popup window displaying an RGB histogram with CDF overlay.
 *
 * Wraps a HistogramPanel inside a resizable frame. Open via
 * ImageFrame's "Histogram" toolbar button to inspect any image.
 */
class HistogramFrame : public wxFrame {
public:
    /**
     * @brief Constructs a HistogramFrame and analyses the given image.
     * @param parent Parent window.
     * @param title  Window title shown in the title bar.
     * @param image  Image whose RGB histogram will be displayed.
     */
    HistogramFrame(wxWindow* parent, const wxString& title, const wxImage& image);
};

#endif // HISTOGRAMFRAME_H
