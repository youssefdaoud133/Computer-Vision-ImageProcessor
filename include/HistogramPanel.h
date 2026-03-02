#ifndef HISTOGRAMPANEL_H
#define HISTOGRAMPANEL_H

#include <wx/wx.h>
#include <vector>

/**
 * @class HistogramPanel
 * @brief Custom wxPanel that draws RGB histograms and their CDF curves.
 *
 * - Solid lines  : per-channel (R/G/B) frequency histogram.
 * - Dotted lines : per-channel cumulative distribution function (CDF).
 *
 * Call Update() whenever the source image changes to redraw automatically.
 */
class HistogramPanel : public wxPanel {
public:
    /**
     * @brief Constructs a HistogramPanel and computes histograms from image.
     * @param parent Parent window.
     * @param image  Source image to analyse (can be invalid — panel stays blank).
     */
    HistogramPanel(wxWindow* parent, const wxImage& image);

    /**
     * @brief Recomputes histograms/CDFs for a new image and repaints.
     * @param image Updated image to analyse.
     */
    void Update(const wxImage& image);

private:
    /** Resets all histogram and CDF vectors, then fills them from image. */
    void CalculateHistogram(const wxImage& image);

    /** wxPaintEvent handler — draws axes, bars and CDF overlay. */
    void OnPaint(wxPaintEvent& event);

    // Per-channel histogram counts (index = intensity 0-255)
    std::vector<int> m_histR, m_histG, m_histB;

    // Per-channel cumulative distribution (normalised 0-1)
    std::vector<double> m_cdfR, m_cdfG, m_cdfB;

    // Maximum frequency across all channels (used for Y-axis scaling)
    int m_maxFreq;

    wxDECLARE_EVENT_TABLE();
};

#endif // HISTOGRAMPANEL_H
