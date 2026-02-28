#include "HistogramFrame.h"

wxBEGIN_EVENT_TABLE(HistogramFrame, wxFrame)
    EVT_PAINT(HistogramFrame::OnPaint)
    EVT_MOTION(HistogramFrame::OnMouseMove)
wxEND_EVENT_TABLE()

HistogramFrame::HistogramFrame(wxWindow* parent, const std::vector<int>& histogram, const std::vector<double>& curve)
    : wxFrame(parent, wxID_ANY, "Image Histogram", wxDefaultPosition, wxSize(500, 400)),
      m_histogram(histogram), m_curve(curve), m_mousePos(-1, -1) {
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    CreateStatusBar();
    SetStatusText("Move mouse over the graph to see values");
}

void HistogramFrame::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    dc.Clear();

    wxSize size = GetClientSize();
    int marginL = 50; // Left margin for Y axis labels
    int marginR = 20;
    int marginT = 20;
    int marginB = 40; // Bottom margin for X axis labels
    
    int drawWidth = size.GetWidth() - marginL - marginR;
    int drawHeight = size.GetHeight() - marginT - marginB;

    if (m_histogram.empty()) return;

    int maxVal = *std::max_element(m_histogram.begin(), m_histogram.end());
    if (maxVal == 0) maxVal = 1;

    double xStep = (double)drawWidth / 256.0;

    // Draw Histogram Bars (Blue)
    dc.SetPen(wxPen(wxColour(100, 100, 255), 1));
    dc.SetBrush(wxBrush(wxColour(150, 150, 255)));

    for (int i = 0; i < 256; i++) {
        int barHeight = (int)((double)m_histogram[i] / maxVal * drawHeight);
        dc.DrawRectangle(marginL + i * xStep, size.GetHeight() - marginB - barHeight, std::max(1.0, xStep), barHeight);
    }

    // Draw Distribution Curve (Red)
    dc.SetPen(wxPen(*wxRED, 2));
    double maxCurve = *std::max_element(m_curve.begin(), m_curve.end());
    if (maxCurve == 0) maxCurve = 1;

    for (int i = 0; i < 255; i++) {
        int y1 = size.GetHeight() - marginB - (int)(m_curve[i] / maxCurve * drawHeight);
        int y2 = size.GetHeight() - marginB - (int)(m_curve[i+1] / maxCurve * drawHeight);
        dc.DrawLine(marginL + i * xStep, y1, marginL + (i + 1) * xStep, y2);
    }

    // Draw Axes
    dc.SetPen(*wxBLACK_PEN);
    dc.DrawLine(marginL, size.GetHeight() - marginB, size.GetWidth() - marginR, size.GetHeight() - marginB); // X
    dc.DrawLine(marginL, size.GetHeight() - marginB, marginL, marginT); // Y

    // Labels and Numbers
    dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    
    // X Axis Labels
    dc.DrawText("0", marginL, size.GetHeight() - marginB + 5);
    dc.DrawText("128", marginL + 128 * xStep - 10, size.GetHeight() - marginB + 5);
    dc.DrawText("255", marginL + 255 * xStep - 20, size.GetHeight() - marginB + 5);
    dc.DrawText("Pixel Intensity", size.GetWidth() / 2 - 40, size.GetHeight() - 20);

    // Y Axis Label (Vertical-ish)
    dc.DrawRotatedText("Frequency (Repetition)", 15, size.GetHeight() / 2 + 50, 90);
    dc.DrawText(wxString::Format("%d", maxVal), 5, marginT);
    dc.DrawText("0", 35, size.GetHeight() - marginB - 10);

    // Draw Tooltip / Tracking line
    if (m_mousePos.x >= marginL && m_mousePos.x <= marginL + drawWidth) {
        int pixelVal = (int)((m_mousePos.x - marginL) / xStep);
        pixelVal = std::clamp(pixelVal, 0, 255);
        int count = m_histogram[pixelVal];

        // Draw vertical line at mouse pos
        dc.SetPen(wxPen(*wxLIGHT_GREY, 1, wxPENSTYLE_DOT));
        dc.DrawLine(m_mousePos.x, marginT, m_mousePos.x, size.GetHeight() - marginB);

        // Draw small tooltip text near the line
        dc.SetTextForeground(*wxRED);
        wxString toolTip = wxString::Format("Pixel: %d, Count: %d", pixelVal, count);
        dc.DrawText(toolTip, m_mousePos.x + 5, m_mousePos.y - 15);
        
        SetStatusText(wxString::Format("Intensity: %d | Frequency: %d", pixelVal, count));
    }
}

void HistogramFrame::OnMouseMove(wxMouseEvent& event) {
    m_mousePos = event.GetPosition();
    Refresh(); // Trigger repaint
}
