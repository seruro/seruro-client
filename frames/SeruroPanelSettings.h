
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/listctrl.h>
#include <wx/splitter.h>

class MenuWindow;
class SettingsView;

class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);
    //~SeruroPanelSettings() {
    //    menu_window->Destory();
    //    general_window->Destory();
    //    splitter->Destory();
    //}
    
    /* Other components may refresh the views. */
    void RefreshAccounts();
    void RefreshExtensions();
    
    /* Todo: consider having a log for extensions. */
    
private:
    
    //void AddMenu(wxSizer *sizer);
    
    //void AddGeneral(wxSizer *sizer);
    //void AddAccounts(wxSizer *sizer);
    //void AddApplications(wxSizer *sizer);
    //void AddExtensions(wxSizer *sizer);
    
    /* Components. */
    //wxListCtrl *menu;
    MenuWindow *menu_window;
    SettingsView *general_window;
    
    wxSplitterWindow *splitter;
};

class MenuWindow : public wxScrolledWindow
{
public:
    MenuWindow(SeruroPanelSettings *window);
    
private:
    wxListCtrl *menu;
    SeruroPanelSettings *parent;
};

//class SettingsView : public wxScrolledWindow
class SettingsView : public wxWindow
{
public:
    SettingsView(SeruroPanelSettings *window) : parent(window),
    wxWindow(window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_THEME) {
        SetBackgroundColour(_("white"));
    }
    
protected:
    SeruroPanelSettings *parent;
};

class GeneralWindow : public SettingsView
{
public:
    GeneralWindow(SeruroPanelSettings *window);
};

class AccountsWindow : public SettingsView
{
public:
    AccountsWindow(SeruroPanelSettings *window);
};

#endif