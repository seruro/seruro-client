
#ifndef H_SeruroPanelSearch
#define H_SeruroPanelSearch

#include "SeruroFrame.h"

#include "../api/SeruroServerAPI.h"
#include "../wxJSON/wx/jsonval.h"

#include <wx/listctrl.h>
#include <wx/choice.h>
#include <wx/srchctrl.h>
#include <wx/imaglist.h>

enum search_actions_t
{
	SEARCH_BUTTON_SEARCH,
	SEARCH_BUTTON_CLEAR
};

enum search_ids_t
{
	SERURO_SEARCH_TEXT_INPUT_ID = 7991,
	SERURO_SEARCH_LIST_ID
};

/* The size of the components view. */
#define SEARCH_UI_COMPONENTS_HEIGHT 100
/* The side of a square checkbox. */
#define CHECKBOX_SIZE 18

class SearchBox;
class CheckedListCtrl;

// Define a new frame type: this is going to be our main frame
class SeruroPanelSearch : public SeruroPanel
{
public:
    // ctor(s)
    SeruroPanelSearch(wxBookCtrlBase *book);

	//void OnSize(wxSizeEvent &event);
	//void DoSize();
    
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

    void Install(const wxString& address, const wxString& server);
    void Uninstall(const wxString& address, const wxString& server = wxEmptyString);
	void OnInstallResult(SeruroRequestEvent &event);

	/* Searches if the address exists, and adds the result line to the
	 * view list, with the appropriate checkbox depending on whether this
	 * OS has the cert installed, and if the cert is updated.
	 */
	void AddResult(const wxString &address, 
	const wxString &first_name, const wxString &last_name);

	//void OnInstallCert(wxCommandEvent &event);
	//void OnUninstallCerl(wxCommandEvent &event);

private:
    CheckedListCtrl *list_control;
    
	/* User inputs. */
	wxChoice *servers_control;
	SearchBox *search_control;
    
	wxBoxSizer *components_sizer;

	SeruroServerAPI *api;

	DECLARE_EVENT_TABLE()
};

#if 0
/* A custom type of list item which contains a JSON member holding both the address of 
 * this identity and the server which manages it.
 */
class IdentityItem : public wxListItem
{
public:
    //ItentityItem() : wxListItem() {}

    void SetIdentity(const wxString& address, const wxString& server);
    wxJSONValue GetIdentity();// { return identity; }
    
private:
    //wxJSONValue identity;
};
#endif

/* The checked list control enhances the normal control to include a 
 * checkbox for each item within the report. 
 * http://wiki.wxwidgets.org/WxListCtrl#Implement_wxListCtrl_with_Checkboxes
 */
class CheckedListCtrl : public wxListCtrl
{
public:
	CheckedListCtrl(SeruroPanelSearch* parent, wxWindowID id, const wxPoint& pt,
		const wxSize& sz, long style);
    
	/* Replace the mouse event to capture clicks within the checkbox area. */
	void OnMouseEvent(wxMouseEvent &event);
	/* Do not allow the user to resize the first column. */
	void OnColumnDrag(wxListEvent &event);

	/* Provide helper methods which toggle the checkbox. */
	bool IsChecked(long item) const;
    void DoCheck(long item, bool checked);
    void SetCheck(long item, bool checked);
    
private:
	/* Save an imagelist of rendered checkbox states. */
    SeruroPanelSearch* parent;
    wxImageList m_imageList;
    
    DECLARE_EVENT_TABLE();
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