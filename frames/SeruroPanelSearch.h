
#ifndef H_SeruroPanelSearch
#define H_SeruroPanelSearch

#include "SeruroFrame.h"

#include "../api/SeruroServerAPI.h"

#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>

enum search_actions_t
{
	SEARCH_BUTTON_SEARCH,
	SEARCH_BUTTON_CLEAR
};

enum search_ids_t
{
	SERURO_SEARCH_TEXT_INPUT_ID = 7991
};

/* The size of the components view. */
#define SEARCH_UI_COMPONENTS_HEIGHT 100

class SearchBox;

// Define a new frame type: this is going to be our main frame
class SeruroPanelSearch : public SeruroPanel
{
public:
    // ctor(s)
    SeruroPanelSearch(wxBookCtrlBase *book);

	//void OnSize(wxSizeEvent &event);
	//void DoSize();

	void OnSearch(wxCommandEvent &event);
	void DoSearch();
	void OnSearchResult(wxCommandEvent &event);
	/* Searches if the address exists, and adds the result line to the
	 * view list, with the appropriate checkbox depending on whether this
	 * OS has the cert installed, and if the cert is updated.
	 */
	void AddResult(const wxString &address, 
	const wxString &first_name, const wxString &last_name);

	//void OnInstallCert(wxCommandEvent &event);
	//void OnUninstallCerl(wxCommandEvent &event);

private:
	/* User inputs. */
	wxListCtrl *list_control;
	wxChoice *servers_control;
	SearchBox *search_control;

	wxBoxSizer *components_sizer;

	SeruroServerAPI *api;

	//void OnKey(wxKeyEvent &event);
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