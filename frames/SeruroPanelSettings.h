
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

#include <wx/event.h>
#include <wx/listctrl.h>

class SettingsView;

class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);
    
    /* Other components may refresh the views. */
    void RefreshAccounts();
    void RefreshExtensions();
    
    /* Todo: consider having a log for extensions. */
    
	/* event handlers. */
	void OnSelected(wxListEvent &event);

private:
    void AddMenu(wxSizer *sizer);
    
    /* Components. */
    wxListCtrl *menu;
    SettingsView *general_window;
	SettingsView *accounts_window;
	SettingsView *applications_window;
	SettingsView *extensions_window;

	DECLARE_EVENT_TABLE()
};



#endif