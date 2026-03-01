#ifndef HISTOGRAMPANEL_H
#define HISTOGRAMPANEL_H

#include <wx/wx.h>
#include <vector>

/**
 * @class HistogramPanel
 * @brief Custom wxPanel for drawing RGB histograms and CDF curves.
 */
class HistogramPanel : public wxPanel {
public:
    /**
     * @brief Constructs a HistogramPanel.
     * @param parent Parent window.
     * @param image The image to calculate histogram from.
     */
    HistogramPanel(wxWindow* parent, const wxImage& image);

private:
    /**
     * @brief Calculates R, G, B histograms and CDFs.
     * @param image Input image.
     */
    void CalculateHistogram(const wxImage& image);

    /**
     * @brief Event handler for painting.
     * @param event Paint event.
     */
    void OnPaint(wxPaintEvent& event);

    // Data
    std::vector<int> m_histR, m_histG, m_histB;
    std::vector<double> m_cdfR, m_cdfG, m_cdfB;
    int m_maxFreq;

    wxDECLARE_EVENT_TABLE();
};

#endif // HISTOGRAMPANEL_H
