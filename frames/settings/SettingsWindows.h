
#ifndef H_SettingsWindows
#define H_SettingsWindows

#include "../../api/SeruroRequest.h"
#include "../../wxJSON/wx/jsonval.h"
#include "../../api/SeruroStateEvents.h"
#include "../../logging/SeruroLogger.h"

/* Half of ApplicationsView is a separate controller. */
#include "../components/AppAccountList.h"

#include <wx/button.h>
#include <wx/listctrl.h>
#include <wx/imaglist.h>

enum settings_ids_t {
    SETTINGS_MENU_ID,
	SETTINGS_SERVERS_LIST_ID,
	SETTINGS_ACCOUNTS_LIST_ID,
    SETTINGS_APPS_LIST_ID,
    SETTINGS_APP_ACCOUNTS_LIST_ID,

	BUTTON_ADD_SERVER,
	BUTTON_ADD_ACCOUNT,
	BUTTON_UPDATE,
	BUTTON_REMOVE,
    BUTTON_ASSIGN,
    BUTTON_UNASSIGN,
    BUTTON_REFRESH
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

	void OnAutoDownload(wxCommandEvent &event);
	void OnDefaultServer(wxCommandEvent &event);

	void OnSaveEncipherment(wxCommandEvent &event);
	void OnPollRevocations(wxCommandEvent &event);
	void OnPollCertstore(wxCommandEvent &event);

	/* State events. */
	void OnServerStateEvent(SeruroStateEvent &event);

private:
	bool GetBoolean(wxString key);
	void SetBoolean(wxString key, bool value);
	void SetString(wxString key, wxString value);

	void GenerateServersList();

	/* Option controls */
	wxCheckBox *option_auto_download;
	wxChoice   *option_default_server;

	wxCheckBox *option_save_encipherment;
	wxCheckBox *option_poll_revocations;
	wxCheckBox *option_poll_certstore;

	DECLARE_EVENT_TABLE()
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
	wxString server_uuid;
	wxString address;

	/* Button components, enable/disable, change label. */
	wxButton *update_button;
	wxButton *remove_button;
	wxButton *add_account_button;

	/* List components (fire events and deselect). */
	wxListCtrl *servers_list;
	wxListCtrl *accounts_list;
	wxImageList *list_images;

	DECLARE_EVENT_TABLE()
};

class ApplicationsWindow : public SettingsView, public AppAccountList
{
public:
    ApplicationsWindow(SeruroPanelSettings *window);
    ~ApplicationsWindow();
    
    void GenerateApplicationsList();
    //void AddAccount(wxString app, wxString account);
    //void GenerateAccountsList();
    
    /* Button event handlers. */
    void OnAssign(wxCommandEvent &event);
    void OnUnassign(wxCommandEvent &event);
    void OnRefresh(wxCommandEvent &event);
    
    /* Selection handlers. */
    void OnAppSelected(wxListEvent &event);
	void OnAccountSelected(wxListEvent &event);
    void OnDeselect(wxListEvent &event) { DoDeselect(); }
	void DoDeselect();
	void DeselectApps();
	//void DeselectAccounts();
    
    /* The client changes something about an account. */
    void OnAccountStateChange(SeruroStateEvent &event);
    void OnApplicationStateChange(SeruroStateEvent &event);
    void OnIdentityStateChange(SeruroStateEvent &event);
    
private:
    void AlignLists();
    
    bool account_selected;
    
    /* Information about the selection item. */
    //wxString app_name;
    //wxString account;
    
    /* Components. */
    wxButton *assign_button;
    wxButton *unassign_button;
    wxListCtrl *apps_list;
    //wxListCtrl *accounts_list;
	wxImageList *apps_list_images;
    
    /* Helpers. */
    //SeruroApps *apps_helper;
    
    DECLARE_EVENT_TABLE()
};

class ExtensionsWindow : public SettingsView
{
public:
    ExtensionsWindow(SeruroPanelSettings *window);
};

class LogWindow : public SettingsView, public SeruroLoggerTarget
{
public:
    LogWindow(SeruroPanelSettings *window);
    
    void OnSendReport(wxCommandEvent &event);
    
protected:
    void ProxyLog(wxLogLevel level, const wxString &msg);
    
private:
    wxTextCtrl *log_box;
    wxButton *send_button;
    
    DECLARE_EVENT_TABLE();
};

#endif
