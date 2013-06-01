
#include "SeruroPanelSearch.h"
#include "../SeruroClient.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"

#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/ustring.h>
#include <wx/checkbox.h>

BEGIN_EVENT_TABLE(SeruroPanelSearch, wxPanel)
	//EVT_SIZE(SeruroPanelSearch::OnSize)
	//EVT_CHAR_HOOK(SeruroPanelSearch::OnKey)
	EVT_BUTTON(SEARCH_BUTTON_SEARCH, SeruroPanelSearch::OnSearch)
	EVT_COMMAND(SERURO_API_CALLBACK_SEARCH, SERURO_API_RESULT, SeruroPanelSearch::OnSearchResult)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SearchBox, wxSearchCtrl)
	EVT_TEXT_ENTER(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
END_EVENT_TABLE()

enum checkboxes_t {
	SERURO_CERTIFICATE_CHECKBOX_ID
};

DECLARE_APP(SeruroClient);

/* Todo: consider polling if the user manually removes a certificate for a user which is displauyed. */

/* The action checkbox allows the user to control the certificate for the user (which is a row)
 * The checkbox will use the address of the user to perform the actions.
 */
class CertificateCheckbox : public wxCheckBox
{
public:
	CertificateCheckbox(wxWindow *parent_obj, const wxString &address) : 
		wxCheckBox(parent_obj, wxID_ANY, wxT("Install/Remove this user's certificate.")), 
		address(address)
	{	
		/* Search for the address within the user's certificate store. */
		this->Enable(false);
	}

	/* The user clicks the box, check the state, and whether that state still matches the state
	 * of the certificate store. Then perform the "request" and "appropriate" action.
	 * Request: remove certificate
	 * Appropriate: remove (if certificate exists), nothing (if certificate does not exist).
	 * Finally, update the state (enabled/disabled) of the checkbox depending on the result.
	 */
	void OnAction(wxCommandEvent &event)
	{
		event.Skip();
		if (this->GetValue() == false) {
			/* The user has requested the certificate be installed. */
		} else {
			/* Remove certificate */
		}
	}

private:
	/* Perform the action based on the address given at construction */
	wxString address;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(CertificateCheckbox, wxCheckBox)
	EVT_CHECKBOX(SERURO_CERTIFICATE_CHECKBOX_ID, CertificateCheckbox::OnAction)
END_EVENT_TABLE()


SeruroPanelSearch::SeruroPanelSearch(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Search"))
{
	/* Create an API object for searching. */
	this->api = new SeruroServerAPI(this->GetEventHandler());

	/* The components sizer hold all UI elements. */
	components_sizer = new wxBoxSizer(wxVERTICAL);

	/* The results sizer holds only the list control. */
	wxBoxSizer *results_sizer = new wxBoxSizer(wxHORIZONTAL);

	/* The list control is a report-view displaying search results. */
	list_control = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		(wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME | wxLC_EDIT_LABELS));

	wxListItem list_column;

	/* Create all of the column for the search results response. */
	list_column.SetText(wxUString(L"*")); /* A checkmark. */
	list_control->InsertColumn(0, list_column);
	list_column.SetText(wxT("Email Address"));
	list_control->InsertColumn(1, list_column);
	list_column.SetText(wxT("First Name"));
	list_control->InsertColumn(2, list_column);
	list_column.SetText(wxT("Last Name"));
	list_control->InsertColumn(3, list_column);

	/* Debug for now, show a "nothing message" in the list. */
	//long item_index;
	//item_index = list_control->InsertItem(0, wxT(""));
	//list_control->SetItem(item_index, 1, wxT("No Email Address"));
	//list_control->SetItem(item_index, 2, wxT("No First Name"));
	//list_control->SetItem(item_index, 3, wxT("No Last Name"));
	this->AddResult(wxString("No Email Address"), 
		wxString("No First Name"), wxString("No Last Name"));

	/* Add the list-control to the UI. */
	results_sizer->Add(list_control, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(results_sizer, 1, wxALL | wxEXPAND, 5);

	/* Define the search controls. */
	wxStaticBoxSizer* const controls_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Search for Certificates"));
	wxBoxSizer *servers_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);

	/* Create search list. */
	wxArrayString servers_list;
	servers_list = wxGetApp().config->GetServerList();
	if (servers_list.size() != 0) {
		/* Only create list if there are multiple servers configured. */
		wxStaticText *servers_text = new wxStaticText(this, wxID_ANY, wxT("Select server:"));
		this->servers_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, servers_list);
		this->servers_control->SetSelection(0);

		servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
		servers_sizer->Add(this->servers_control, 0, wxRIGHT, 5);
	}

	/* Create search text-field. */
	//wxStaticText *search_text = new wxStaticText(this, wxID_ANY, wxT("Search:"));
	//this->search_control = new wxTextCtrl(this, SERURO_SEARCH_TEXT_INPUT_ID);
	this->search_control = new SearchBox(this);

	//search_sizer->Add(search_text, 0, wxRIGHT, 5);
	search_sizer->Add(this->search_control, 1);

	/* Create UI buttons. */
	//wxButton *clear_button = new wxButton(this, SEARCH_BUTTON_CLEAR, wxT("Clear")); 
	/* Todo: manually set focus here. */
	//wxButton *search_button = new wxButton(this, SEARCH_BUTTON_SEARCH, wxT("Search"));

	/* For some reason we cannot align right without a spacer. */
	//buttons_sizer->AddStretchSpacer();
	//buttons_sizer->Add(search_button, 0, wxRIGHT, 5);
	/* Todo: add a 'clear' button which removes all results. */
	//buttons_sizer->Add(clear_button, 0, wxRIGHT, 5);

	/* All them all into the components sizer. */
	controls_sizer->Add(servers_sizer, 1, wxALL | wxEXPAND, 5);
	controls_sizer->Add(search_sizer, 1, wxALL | wxEXPAND, 5);
	//controls_sizer->Add(buttons_sizer, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(controls_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	/* Add the components to the main view. */
	this->SetSizer(components_sizer);

	/* Testing: setting even column widths. */
	int list_column_size = SEARCH_PANEL_COLUMN_WIDTH;
	this->list_control->SetColumnWidth(0, 20);
	this->list_control->SetColumnWidth(1, list_column_size);
	this->list_control->SetColumnWidth(2, list_column_size);
	this->list_control->SetColumnWidth(3, list_column_size);

	/* Testing default focus */
	this->search_control->SetFocus();
	this->Layout();
}

void SeruroPanelSearch::AddResult(const wxString &address, 
	const wxString &first_name, const wxString &last_name)
{
	long item_index;
	
	/* place appropriately marked checkbox. */
	CertificateCheckbox *checkbox = new CertificateCheckbox(list_control, address);
	item_index = this->list_control->InsertItem(0, checkbox);
	
	list_control->SetItem(item_index, 1, address);
	list_control->SetItem(item_index, 2, first_name);
	list_control->SetItem(item_index, 3, last_name);
}

void SeruroPanelSearch::OnSearch(wxCommandEvent &event)
{
	this->DoSearch();
}

void SeruroPanelSearch::DoSearch()
{
	/* Todo this should pick up the selected (or ONLY) server. */
    wxString server_name = wxT("Open Seruro"); //m_server_box->GetValue();
	wxString query = this->search_control->GetValue();

	wxJSONValue server = this->api->GetServer(server_name);	
	
	wxJSONValue params;
	params["query"] = query;
	params["server"] = server;

	/* Disable the search box until the query completes. */
	this->search_control->Enable(false);
	
	SeruroRequest *request = api->CreateRequest(SERURO_API_SEARCH, params, SERURO_API_CALLBACK_SEARCH);
	request->Run();
}

void SeruroPanelSearch::OnSearchResult(wxCommandEvent &event)
{
	wxJSONReader reader;
	wxJSONValue response;
	wxString responseString = event.GetString();
	reader.Parse(responseString, &response);

	/* Clear the results list. */
	this->list_control->DeleteAllItems();

	/* Set the cursor back to the input field. */
	this->search_control->Enable(true);
	this->search_control->SetFocus();

	if (! response.HasMember("success") || ! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroPanelSearch> (Search) Bad Result."));
		return;
	}

	wxLogMessage(wxT("SeruroPanelTest> (Search) API valid response."));

	/* From the wxWidgets samples, should speed up adding. */
	//this->list_control->Hide();
	this->list_control->DeleteAllItems();

	/* Add results to UI. */
	for (int i = 0; i < response["results"].Size(); i++) {
		//long item_index;
		//item_index = this->list_control->InsertItem(1, response["results"][i]["email"].AsString());
		//list_control->SetItem(item_index, 2, response["results"][i]["first_name"].AsString());
		//list_control->SetItem(item_index, 3, response["results"][i]["last_name"].AsString());
		this->AddResult(response["results"][i]["email"].AsString(),
			response["results"][i]["first_name"].AsString(), 
			response["results"][i]["last_name"].AsString());
	}

	//this->list_control->Show();

}

#if 0
void SeruroPanelSearch::OnKey(wxKeyEvent &event)
{
	wxWindow *win = FindFocus();
	if (win == NULL) {
		event.Skip();
		return;
	}

	/* Todo: when search control is active, the enter key should 'search'. */
	if (event.GetKeyCode() == WXK_RETURN) {

	}

	event.Skip();
}

/* Override the default search function which allows us to control the list and components 
 * relative or absolite positions / sizes. */
void SeruroPanelSearch::OnSize(wxSizeEvent &event)
{
	DoSize();

	event.Skip();
}

/* Make sure the search components always stay on screen, real-estate is taken from the list. */
void SeruroPanelSearch::DoSize()
{
	if (! list_control)
		return;

	//wxSize size = GetClientSize();
	//wxCoord y = (size.y) - SEARCH_UI_COMPONENTS_HEIGHT; 
	//list_control->SetSize(0, 0, size.x, y);
	//_mainSizer->SetDimension(0, y, size.x, SEARCH_UI_COMPONENTS_HEIGHT);
}
#endif
