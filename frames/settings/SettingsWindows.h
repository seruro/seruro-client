
#ifndef H_SettingsWindows
#define H_SettingsWindows

#include "../../api/SeruroRequest.h"
#include "../../wxJSON/wx/jsonval.h"
#include "../../api/SeruroStateEvents.h"

#include "../../apps/SeruroApps.h"

#include <wx/button.h>
#include <wx/listctrl.h>

enum settings_ids_t {
    SETTINGS_MENU_ID,
	SETTINGS_SERVERS_LIST_ID,
	SETTINGS_ACCOUNTS_LIST_ID,
    SETTINGS_APPS_LIST_ID,
    SETTINGS_APP_ACCOUNTS_LIST_ID,

	BUTTON_ADD_SERVER,
	BUTTON_ADD_ACCOUNT,
	BUTTON_UPDATE,
	BUTTON_REMOVE
};

class SeruroPanelSettings;
class SeruroApps;

class SettingsView : public wxWindow
{
public:
	/* Defined in GeneralWindow. */
    SettingsView(SeruroPanelSettings *window);
	void OnColumnDrag(wxListEvent &event) {
		/* This could be better defined, assumes image=0. */
		if (event.GetColumn() == 0) {
			event.Veto();
		}
	}
    
protected:
    SeruroPanelSettings *parent;
};

/* Window-specific classes. */

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
    void OnDeselect(wxListEvent &event) { DoDeselect(); }
	void DoDeselect();
	void DeselectServers();
	void DeselectAccounts();

	void OnUpdate(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);
	void OnCAResult(SeruroRequestEvent &event);

	void OnAddServer(wxCommandEvent &event);
	void OnAddAccount(wxCommandEvent &event);

	/* What happens when the UI or OS events change settings!? */
	void OnServerStateChange(SeruroStateEvent &event);
	void OnAccountStateChange(SeruroStateEvent &event);
        
    /* Add action. */
    void SetActionLabel(wxString label) {
        update_button->SetLabel(label);
    }

	/* Todo: d-click to view certificate information. */

private:
    /* Add servers/accounts to lists. */
    void GenerateServersList();
    void GenerateAccountsList();
	void AlignLists();
	
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
    
    void GenerateApplicationsList();
    void GenerateAccountsList();
    
private:
    bool account_selected;
    
    /* Information about the selection item. */
    wxString app_name;
    wxString account;
    
    /* Components. */
    wxButton *configure_button;
    wxButton *remove_button;
    wxListCtrl *apps_list;
    wxListCtrl *accounts_list;
    
    /* Helpers. */
    SeruroApps apps_helper;
};

class ExtensionsWindow : public SettingsView
{
public:
    ExtensionsWindow(SeruroPanelSettings *window);
};

#endif 
