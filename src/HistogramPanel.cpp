#include "HistogramPanel.h"
#include <wx/dcbuffer.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(HistogramPanel, wxPanel)
    EVT_PAINT(HistogramPanel::OnPaint)
wxEND_EVENT_TABLE()

// ─────────────────────────────────────────────────────────────
// Construction
// ─────────────────────────────────────────────────────────────
HistogramPanel::HistogramPanel(wxWindow* parent, const wxImage& image)
    : wxPanel(parent, wxID_ANY),
      m_histR(256, 0), m_histG(256, 0), m_histB(256, 0),
      m_cdfR(256, 0.0), m_cdfG(256, 0.0), m_cdfB(256, 0.0),
      m_maxFreq(0)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CalculateHistogram(image);
}

// ─────────────────────────────────────────────────────────────
// Public: recalculate and repaint
// ─────────────────────────────────────────────────────────────
void HistogramPanel::Update(const wxImage& image) {
    CalculateHistogram(image);
    Refresh();
}

// ─────────────────────────────────────────────────────────────
// Private: build per-channel histogram + CDF
// ─────────────────────────────────────────────────────────────
void HistogramPanel::CalculateHistogram(const wxImage& image) {
    // Reset to zero
    std::fill(m_histR.begin(), m_histR.end(), 0);
    std::fill(m_histG.begin(), m_histG.end(), 0);
    std::fill(m_histB.begin(), m_histB.end(), 0);
    m_maxFreq = 0;

    if (!image.IsOk()) return;

    long totalPixels = static_cast<long>(image.GetWidth()) * image.GetHeight();
    unsigned char* data = image.GetData();

    // Count per-channel frequencies
    for (long i = 0; i < totalPixels; ++i) {
        m_histR[data[i * 3    ]]++;
        m_histG[data[i * 3 + 1]]++;
        m_histB[data[i * 3 + 2]]++;
    }

    // Max frequency for Y-axis scaling
    int maxR = *std::max_element(m_histR.begin(), m_histR.end());
    int maxG = *std::max_element(m_histG.begin(), m_histG.end());
    int maxB = *std::max_element(m_histB.begin(), m_histB.end());
    m_maxFreq = std::max({ maxR, maxG, maxB });

    // Cumulative Distribution Functions (normalised 0-1)
    double sumR = 0, sumG = 0, sumB = 0;
    for (int i = 0; i < 256; ++i) {
        sumR += m_histR[i];
        sumG += m_histG[i];
        sumB += m_histB[i];
        m_cdfR[i] = sumR / totalPixels;
        m_cdfG[i] = sumG / totalPixels;
        m_cdfB[i] = sumB / totalPixels;
    }
}

// ─────────────────────────────────────────────────────────────
// Private: paint handler
// ─────────────────────────────────────────────────────────────
void HistogramPanel::OnPaint(wxPaintEvent&) {
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    wxSize size = GetClientSize();
    const int pad  = 30;
    const int drawW = size.x - 2 * pad;
    const int drawH = size.y - 2 * pad;

    if (drawW < 10 || drawH < 10) return;

    // ── Axes ──────────────────────────────────────────────────
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(pad, size.y - pad, size.x - pad, size.y - pad); // X
    dc.DrawLine(pad, pad,          pad,           size.y - pad); // Y

    if (m_maxFreq == 0) return; // blank image — nothing to draw

    // ── Coordinate helpers ───────────────────────────────────
    auto xOf = [&](int i)     { return pad + (int)((double)i / 255 * drawW); };
    auto yHist = [&](int f)   { return size.y - pad - (int)((double)f / m_maxFreq * drawH); };
    auto yCDF  = [&](double v){ return size.y - pad - (int)(v * drawH); };

    // ── Draw histogram polyline per channel ──────────────────
    auto drawHist = [&](const std::vector<int>& hist, const wxColour& col) {
        dc.SetPen(wxPen(col, 1));
        for (int i = 0; i < 255; ++i)
            dc.DrawLine(xOf(i), yHist(hist[i]), xOf(i+1), yHist(hist[i+1]));
    };

    // ── Draw CDF polyline per channel ────────────────────────
    auto drawCDF = [&](const std::vector<double>& cdf, const wxColour& col) {
        dc.SetPen(wxPen(col, 2, wxPENSTYLE_DOT));
        for (int i = 0; i < 255; ++i)
            dc.DrawLine(xOf(i), yCDF(cdf[i]), xOf(i+1), yCDF(cdf[i+1]));
    };

    drawHist(m_histR, *wxRED);
    drawHist(m_histG, wxColour(0, 180, 0));
    drawHist(m_histB, *wxBLUE);

    drawCDF(m_cdfR, wxColour(180, 0,   0  ));
    drawCDF(m_cdfG, wxColour(0,   140, 0  ));
    drawCDF(m_cdfB, wxColour(0,   0,   180));

    // ── Legend ───────────────────────────────────────────────
    dc.SetFont(wxFont(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(wxColour(80, 80, 80));
    dc.DrawText("Solid=Histogram  Dotted=CDF", pad, 4);
}
