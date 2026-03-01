#include "HistogramFrame.h"
#include <algorithm>
#include <numeric>
#include <wx/dcbuffer.h>

wxBEGIN_EVENT_TABLE(HistogramFrame, wxFrame)
    EVT_PAINT(HistogramFrame::OnPaint)
    EVT_MOTION(HistogramFrame::OnMouseMove)
wxEND_EVENT_TABLE()

HistogramFrame::HistogramFrame(wxWindow* parent,
                                const std::vector<int>&    histogram,
                                const std::vector<double>& curve)
    : wxFrame(parent, wxID_ANY, "Image Histogram",
              wxDefaultPosition, wxSize(900, 500)),
      m_histogram(histogram), m_curve(curve), m_mousePos(-1, -1)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CreateStatusBar();
    SetStatusText("Move mouse over the graph to see pixel intensity values.");
}

void HistogramFrame::OnPaint(wxPaintEvent& /*event*/) {
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();

    wxSize size = GetClientSize();
    const int marginL = 70;
    const int marginR = 20;
    const int marginT = 30;
    const int marginB = 45;

    int drawWidth  = size.GetWidth()  - marginL - marginR;
    int drawHeight = size.GetHeight() - marginT - marginB;

    if (m_histogram.empty() || drawWidth <= 0 || drawHeight <= 0) return;

    // 99th-percentile cap so a single spike doesn't crush the rest of the chart
    int trueMax = *std::max_element(m_histogram.begin(), m_histogram.end());

    std::vector<int> sorted = m_histogram;
    std::sort(sorted.begin(), sorted.end());
    int scaleMax = sorted[252];
    if (scaleMax <= 0) scaleMax = trueMax;
    if (scaleMax <= 0) scaleMax = 1;

    // Gap-free bar boundary calculation
    auto binLeft  = [&](int i) {
        return marginL + (int)std::round(i       * (double)drawWidth / 256.0);
    };
    auto binRight = [&](int i) {
        return marginL + (int)std::round((i + 1) * (double)drawWidth / 256.0);
    };

    // ── Draw histogram bars (blue) ────────────────────────────────────────────
    dc.SetPen  (wxPen  (wxColour(100, 100, 255), 1));
    dc.SetBrush(wxBrush(wxColour(150, 150, 255)));

    for (int i = 0; i < 256; i++) {
        int barH = std::min(drawHeight,
                            (int)std::round((double)m_histogram[i] / scaleMax * drawHeight));
        int bx = binLeft(i);
        int bw = std::max(1, binRight(i) - bx);
        int by = size.GetHeight() - marginB - barH;
        dc.DrawRectangle(bx, by, bw, barH);
    }

    // ── Draw distribution curve (red) ────────────────────────────────────────
    dc.SetPen(wxPen(*wxRED, 2));

    auto curveY = [&](int i) -> int {
        int y = size.GetHeight() - marginB -
                (int)std::round(m_curve[i] / scaleMax * drawHeight);
        return std::clamp(y, marginT, size.GetHeight() - marginB);
    };

    for (int i = 0; i < 255; i++) {
        dc.DrawLine(binLeft(i),     curveY(i),
                    binLeft(i + 1), curveY(i + 1));
    }

    // ── Axes ─────────────────────────────────────────────────────────────────
    dc.SetPen(wxPen(*wxBLACK, 2));
    dc.DrawLine(marginL, size.GetHeight() - marginB,
                size.GetWidth() - marginR, size.GetHeight() - marginB); // X
    dc.DrawLine(marginL, size.GetHeight() - marginB,
                marginL, marginT);                                        // Y

    // ── Labels ───────────────────────────────────────────────────────────────
    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT,
                      wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    dc.SetTextForeground(*wxBLACK);

    // ── X axis ticks and numbers (0, 32, 64, 96, 128, 160, 192, 224, 255) ───
    const int xTicks[] = {0, 32, 64, 96, 128, 160, 192, 224, 255};
    for (int val : xTicks) {
        int px = binLeft(val);
        // Tick mark
        dc.SetPen(wxPen(*wxBLACK, 1));
        dc.DrawLine(px, size.GetHeight() - marginB,
                    px, size.GetHeight() - marginB + 4);
        // Number — centre it under the tick
        wxString label = wxString::Format("%d", val);
        wxSize   lsz   = dc.GetTextExtent(label);
        dc.DrawText(label, px - lsz.GetWidth() / 2,
                    size.GetHeight() - marginB + 6);
    }

    // X axis title
    wxString xTitle = "Pixel Intensity";
    wxSize   xTSz   = dc.GetTextExtent(xTitle);
    dc.DrawText(xTitle,
                marginL + drawWidth / 2 - xTSz.GetWidth() / 2,
                size.GetHeight() - marginB + 22);

    // ── Y axis ticks and numbers (5 evenly spaced steps) ─────────────────────
    const int ySteps = 5;
    for (int s = 0; s <= ySteps; s++) {
        int val = (int)std::round((double)scaleMax * s / ySteps);
        int py  = size.GetHeight() - marginB -
                  (int)std::round((double)drawHeight * s / ySteps);
        // Tick mark
        dc.SetPen(wxPen(*wxBLACK, 1));
        dc.DrawLine(marginL - 4, py, marginL, py);
        // Number — right-aligned against the Y axis
        wxString label = wxString::Format("%d", val);
        wxSize   lsz   = dc.GetTextExtent(label);
        dc.DrawText(label, marginL - lsz.GetWidth() - 6, py - lsz.GetHeight() / 2);
    }

    // Y axis rotated label
    dc.DrawRotatedText("Frequency (Repetition)", 12, size.GetHeight() / 2 + 65, 90);

    // ── Mouse tracking line + tooltip ────────────────────────────────────────
    if (m_mousePos.x >= marginL && m_mousePos.x < marginL + drawWidth) {
        int pixelVal = std::clamp(
            (int)((m_mousePos.x - marginL) * 256.0 / drawWidth), 0, 255);
        int count = m_histogram[pixelVal];

        // Dotted vertical guide line
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1, wxPENSTYLE_DOT));
        dc.DrawLine(m_mousePos.x, marginT,
                    m_mousePos.x, size.GetHeight() - marginB);

        // Tooltip text
        dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT,
                          wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        wxString tip   = wxString::Format("Pixel: %d   Count: %d", pixelVal, count);
        wxSize   tipSz = dc.GetTextExtent(tip);

        int tx = m_mousePos.x + 10;
        int ty = std::max(marginT + 2, m_mousePos.y - 22);

        if (tx + tipSz.GetWidth() + 6 > size.GetWidth() - marginR)
            tx = m_mousePos.x - tipSz.GetWidth() - 10;

        // Tooltip background box
        dc.SetBrush(wxBrush(wxColour(255, 255, 210)));
        dc.SetPen  (wxPen  (wxColour(160, 160, 0), 1));
        dc.DrawRectangle(tx - 4, ty - 3,
                         tipSz.GetWidth() + 8, tipSz.GetHeight() + 6);

        dc.SetTextForeground(wxColour(180, 0, 0));
        dc.DrawText(tip, tx, ty);

        SetStatusText(wxString::Format(
            "Intensity: %d  |  Frequency: %d", pixelVal, count));
    } else {
        SetStatusText("Move mouse over the graph to see pixel intensity values.");
    }
}

void HistogramFrame::OnMouseMove(wxMouseEvent& event) {
    m_mousePos = event.GetPosition();
    Refresh();
}