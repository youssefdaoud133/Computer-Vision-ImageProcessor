#ifndef APP_H
#define APP_H

#include <wx/wx.h>

/**
 * @class App
 * @brief The main application class.
 *
 * Inherits from wxApp and serves as the entry point of the wxWidgets program.
 */
class App : public wxApp {
public:
    /**
     * @brief Called upon application initialization.
     * @return true if initialization was successful, false otherwise.
     */
    virtual bool OnInit() override;
};

#endif // APP_H
