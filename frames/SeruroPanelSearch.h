
#ifndef H_SeruroPanelSearch
#define H_SeruroPanelSearch

#include "SeruroFrame.h"

#include "../api/SeruroServerAPI.h"

#include <wx/listctrl.h>
#include <wx/choice.h>

enum search_actions_t
{
	SEARCH_BUTTON_SEARCH,
	SEARCH_BUTTON_CLEAR
};

/* The size of the components view. */
#define SEARCH_UI_COMPONENTS_HEIGHT 100

// Define a new frame type: this is going to be our main frame
class SeruroPanelSearch : public SeruroPanel
{
public:
    // ctor(s)
    SeruroPanelSearch(wxBookCtrlBase *book);

	void OnSize(wxSizeEvent &event);
	void DoSize();

	void OnSearch(wxCommandEvent &event);
	void OnSearchResult(wxCommandEvent &event);

	//void OnInstallCert(wxCommandEvent &event);
	//void OnUninstallCerl(wxCommandEvent &event);

private:
	/* User inputs. */
	wxListCtrl *list_control;
	wxChoice *servers_control;
	wxTextCtrl *search_control;

	wxBoxSizer *components_sizer;

	SeruroServerAPI *api;

	void OnKey(wxKeyEvent &event);
	DECLARE_EVENT_TABLE()
};

#endif