#include "HistogramPanel.h"
#include <wx/dcbuffer.h>
#include <algorithm>

wxBEGIN_EVENT_TABLE(HistogramPanel, wxPanel)
    EVT_PAINT(HistogramPanel::OnPaint)
wxEND_EVENT_TABLE()

HistogramPanel::HistogramPanel(wxWindow* parent, const wxImage& image)
    : wxPanel(parent, wxID_ANY),
      m_histR(256, 0), m_histG(256, 0), m_histB(256, 0),
      m_cdfR(256, 0.0), m_cdfG(256, 0.0), m_cdfB(256, 0.0),
      m_maxFreq(0) {
    
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CalculateHistogram(image);
}

void HistogramPanel::CalculateHistogram(const wxImage& image) {
    if (!image.IsOk()) return;

    int width = image.GetWidth();
    int height = image.GetHeight();
    long totalPixels = width * height;

    unsigned char* data = image.GetData();
    for (long i = 0; i < totalPixels; ++i) {
        m_histR[data[i * 3]]++;
        m_histG[data[i * 3 + 1]]++;
        m_histB[data[i * 3 + 2]]++;
    }

    // Find max frequency for scaling
    int maxR = *std::max_element(m_histR.begin(), m_histR.end());
    int maxG = *std::max_element(m_histG.begin(), m_histG.end());
    int maxB = *std::max_element(m_histB.begin(), m_histB.end());
    m_maxFreq = std::max({maxR, maxG, maxB});

    // Calculate CDF
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

void HistogramPanel::OnPaint(wxPaintEvent& event) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize size = GetClientSize();
    if (size.x < 50 || size.y < 50) return;

    int padding = 40;
    int drawW = size.x - 2 * padding;
    int drawH = size.y - 2 * padding;

    // Draw axes
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(padding, size.y - padding, size.x - padding, size.y - padding); // X
    dc.DrawLine(padding, padding, padding, size.y - padding);                 // Y

    // Function to map histogram value to Y coordinate
    auto getYHist = [&](int freq) {
        return size.y - padding - (int)((double)freq / m_maxFreq * drawH);
    };

    // Function to map CDF value to Y coordinate
    auto getYCDF = [&](double val) {
        return size.y - padding - (int)(val * drawH);
    };

    // Function to map 0-255 to X coordinate
    auto getX = [&](int i) {
        return padding + (int)((double)i / 255 * drawW);
    };

    // Draw Histogram Bars (Red, Green, Blue)
    auto drawChannelHist = [&](const std::vector<int>& hist, const wxColour& color) {
        dc.SetPen(wxPen(color, 1, wxPENSTYLE_SOLID));
        dc.SetBrush(wxBrush(color, wxBRUSHSTYLE_TRANSPARENT));
        for (int i = 0; i < 255; ++i) {
            dc.DrawLine(getX(i), getYHist(hist[i]), getX(i+1), getYHist(hist[i+1]));
        }
    };

    // Draw CDF Curves
    auto drawChannelCDF = [&](const std::vector<double>& cdf, const wxColour& color) {
        dc.SetPen(wxPen(color, 2, wxPENSTYLE_DOT));
        for (int i = 0; i < 255; ++i) {
            dc.DrawLine(getX(i), getYCDF(cdf[i]), getX(i+1), getYCDF(cdf[i+1]));
        }
    };

    drawChannelHist(m_histR, *wxRED);
    drawChannelHist(m_histG, *wxGREEN);
    drawChannelHist(m_histB, *wxBLUE);

    drawChannelCDF(m_cdfR, wxColour(200, 0, 0));
    drawChannelCDF(m_cdfG, wxColour(0, 200, 0));
    drawChannelCDF(m_cdfB, wxColour(0, 0, 200));

    // Legend
    dc.SetTextForeground(*wxBLACK);
    dc.DrawText("Solid: Histogram", padding + 10, padding / 2);
    dc.DrawText("Dotted: CDF", padding + 150, padding / 2);
}
