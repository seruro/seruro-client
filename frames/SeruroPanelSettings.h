
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

	/* Event handlers. */
	void OnServerSelected(wxListEvent &event);
	void OnAccountSelected(wxListEvent &event);
	void DeselectServers();
	void DeselectAccounts();

	void OnUpdate(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);

	void OnAddServer(wxCommandEvent &event);
	void OnAddAccount(wxCommandEvent &event);

	/* Todo: d-click to view certificate information. */

private:
	/* Determine if an account is selected. */
	bool account_selected;

	/* Information about the selected item. */
	wxString server_name;
	wxString address;

	/* Button components, enable/disable, change label. */
	wxButton *update_button;
	wxButton *remove_button;

	/* List components (fire events and deselect). */
	wxListCtrl *servers_list;
	wxListCtrl *accounts_list;

	DECLARE_EVENT_TABLE()
};

class ApplicationsWindow : public SettingsView
{
public:
    ApplicationsWindow(SeruroPanelSettings *window);
};

class ExtensionsWindow : public SettingsView
{
public:
    ExtensionsWindow(SeruroPanelSettings *window);
};

#endif