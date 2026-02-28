#ifndef HISTOGRAMFRAME_H
#define HISTOGRAMFRAME_H

#include <wx/wx.h>
#include <vector>

class HistogramFrame : public wxFrame {
public:
    HistogramFrame(wxWindow* parent, const std::vector<int>& histogram, const std::vector<double>& curve);

private:
    void OnPaint(wxPaintEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    
    std::vector<int> m_histogram;
    std::vector<double> m_curve;
    wxPoint m_mousePos;

    wxDECLARE_EVENT_TABLE();
};

#endif // HISTOGRAMFRAME_H
