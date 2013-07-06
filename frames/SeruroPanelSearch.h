
#ifndef H_SeruroPanelSearch
#define H_SeruroPanelSearch

#include "SeruroFrame.h"
#include "components/CheckedListCtrl.h"

#include "../api/SeruroServerAPI.h"
#include "../wxJSON/wx/jsonval.h"
#include "../api/SeruroStateEvents.h"

//#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>

/* The size of the components view. */
#define SEARCH_UI_COMPONENTS_HEIGHT 100

enum search_actions_t
{
	SEARCH_BUTTON_SEARCH,
	SEARCH_BUTTON_CLEAR
};

enum search_ids_t
{
	SERURO_SEARCH_TEXT_INPUT_ID
};

typedef wxJSONValue* IdentityItemPtr;

class SearchBox;

// Define a new frame type: this is going to be our main frame
class SeruroPanelSearch : public SeruroPanel
{
public:
    SeruroPanelSearch(wxBookCtrlBase *book);
    
    /* Find the current server, useful during a disable/enable search. */
    wxString GetSelectedServer() {
        if (! this->servers_control) { return wxEmptyString; }
        int index = this->servers_control->GetSelection();
        return this->servers_control->GetString(index);
    }

	void OnSearch(wxCommandEvent &event);
	void DoSearch();
	void OnSearchResult(SeruroRequestEvent &event);
    
    /* UI helpers during search requests, and results processing. */
    void DisableSearch();
    void EnableSearch();

    void Install(const wxString& address, const wxString& server_name);
    void Uninstall(const wxString& address, 
		const wxString& server_name = wxEmptyString);
	void OnInstallResult(SeruroRequestEvent &event);

	/* Searches if the address exists, and adds the result line to the
	 * view list, with the appropriate checkbox depending on whether this
	 * OS has the cert installed, and if the cert is updated.
	 */
	void AddResult(const wxString &address, 
	const wxString &first_name, const wxString &last_name);

	//void OnInstallCert(wxCommandEvent &event);
	//void OnUninstallCerl(wxCommandEvent &event);

	/* Since other parts of the app may update server/address(s)
	 * Catch view changing events to update the server list
	 * or potentially remove entries from cached search results.
	 */
	void OnFocus(wxFocusEvent &event) { 
		this->DoFocus();
	}
	/* Apply the focuing logic */
	void DoFocus();

	void OnServerStateChange(SeruroStateEvent &event);

private:
    CheckedListCtrl *list_control;
    
	/* User inputs. */
	wxChoice *servers_control;
	SearchBox *search_control;
	wxBoxSizer *components_sizer;

	SeruroServerAPI *api;

	DECLARE_EVENT_TABLE()
};

/* The searchbox is where the user interactes. */
class SearchBox : public wxSearchCtrl
{
public:
	SearchBox(SeruroPanelSearch *parent_obj) :
    wxSearchCtrl(parent_obj, SERURO_SEARCH_TEXT_INPUT_ID,
		wxEmptyString, wxDefaultPosition, wxDefaultSize,
        /* Make sure to handle ENTER events normally. */
        wxTE_PROCESS_ENTER),
    parent(parent_obj) {}
    
	void OnSearch(wxCommandEvent &event) {
		this->parent->DoSearch();
	}
private:
	SeruroPanelSearch *parent;
	DECLARE_EVENT_TABLE();
};

#endif