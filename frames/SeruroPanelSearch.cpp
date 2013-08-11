
#include "SeruroPanelSearch.h"
#include "../SeruroClient.h"

/* Need GetServerChoice */
#include "dialogs/AddServerDialog.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"
#include "../wxJSON/wx/jsonwriter.h"
#include <wx/checkbox.h>

/* Image data. */
#include "../resources/images/certificate_icon_18_flat.png.h"

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroPanelSearch, wxPanel)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_SEARCH, SeruroPanelSearch::OnSearchResult)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CERTS, SeruroPanelSearch::OnInstallResult)
	EVT_CHECKBOX(SERURO_SEARCH_ALL_SERVERS_ID, SeruroPanelSearch::OnAllServersCheck)

	/* Defined using a dynamic bind. */
	//EVT_SERURO_STATE(STATE_TYPE_SERVER, SeruroPanelSearch::OnServerStateChange)

	EVT_SET_FOCUS(SeruroPanelSearch::OnFocus)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SearchBox, wxSearchCtrl)
	/* User wants to search. */
	EVT_SEARCHCTRL_SEARCH_BTN(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
	EVT_TEXT_ENTER(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
END_EVENT_TABLE()

void SeruroPanelSearch::OnServerStateChange(SeruroStateEvent &event)
{
    wxLogMessage(_("SeruroPanelServer> (OnServerStateChange)"));
	if (event.GetAction() == STATE_ACTION_REMOVE) {
		wxLogMessage(_("SeruroPanelSearch> (OnServerStateChange) removing server (%s)."), event.GetServerUUID());
	}
    
    /* Focus should update the servers list and filter results. */
    this->DoFocus();
    /* Allow other handlers. */
	event.Skip();
}

void SeruroPanelSearch::DoFocus()
{
	wxArrayString server_names;
	wxLogDebug(_("SeruroPanelSearch> (DoFocus) focusing the search."));
    
    server_names = wxGetApp().config->GetServerNames();
	/* If the config has changed, regenerate the list of servers. */
	if (server_names.size() != servers_control->GetCount()) {
		this->servers_control->Clear();
		this->servers_control->Append(server_names);
		this->servers_control->SetSelection(0);

		/* Filter search results for potentially-removed servers. */
		this->list_control->FilterResultsByServers(server_names);
	}

	/* Depending on the number of servers, enable/disable the controls. */
	if (server_names.size() == 0) {
		/* This may have been active before, and there may be dangling text. */
		this->search_control->Clear();
		this->DisableSearch();
	} else {
		this->EnableSearch();
		this->search_control->SetFocus();
	}
}

void SeruroPanelSearch::Install(const wxString& server_name, const wxString& address)
{
    wxJSONValue params;
    wxJSONValue server_info;
    
    wxLogMessage(wxT("SeruroPanelSearch:Install> requesting certificate for (name= %s) (%s)."), server_name, address);
    
    server_info = api->GetServer(wxGetApp().config->GetServerUUID(server_name));
    params["server"] = server_info;
    params["request_address"] = address;
    
	SeruroRequest *request = api->CreateRequest(SERURO_API_CERTS, params, SERURO_API_CALLBACK_CERTS);
	request->Run();
}

/* After the certificate is installed, update the UI. */
void SeruroPanelSearch::OnInstallResult(SeruroRequestEvent &event)
{
	/* The event data should include the address which was updated. */
    wxJSONValue response;
    wxString server_name;
    
    response = event.GetResponse();
    if (! response.HasMember("success") || ! response["success"].AsBool() || ! response.HasMember("address")) {
        wxLogMessage(wxT("OnInstallResults> Bad Result."));
        return;
    }
    
    if (! response.HasMember("certs") || response["certs"].Size() == 0) {
        wxLogMessage(wxT("OnInstallResult> No certs found for address (%s)."), response["address"].AsString());
        return;
    }
    
    /* Check the corresponding item(s) in the list control. */
    server_name = wxGetApp().config->GetServerName(response["server_uuid"].AsString());
    if (api->InstallCertificate(response)) {
        list_control->SetCheck(server_name, response["address"].AsString(), true);
    }
}

/* There is no callback for uninstall, this happens locally. */
void SeruroPanelSearch::Uninstall(const wxString& server_name, const wxString& address)
{
    wxString server_uuid;
    
    server_uuid = wxGetApp().config->GetServerUUID(server_name);
    wxLogMessage(_("SeruroPanelSearch> (Uninstall) trying to uninstall (name= %s) (%s)."), server_name, address);
    if (api->UninstallCertificates(server_uuid, address)) {
        list_control->SetCheck(server_name, address, false);
    }
}

void SeruroPanelSearch::OnAllServersCheck(wxCommandEvent &event)
{
	/* Simply update the servers_control to reflect the status of 'all_servers'. */
	if (this->all_servers_control->IsChecked()) {
		this->servers_control->Disable();
	} else {
		this->servers_control->Enable();
	}
}

SeruroPanelSearch::SeruroPanelSearch(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Search"))
{
	/* Create an API object for searching. */
	this->api = new SeruroServerAPI(this);

	/* The components sizer hold all UI elements. */
	components_sizer = new wxBoxSizer(wxVERTICAL);

	/* The results sizer holds only the list control. */
	wxBoxSizer *results_sizer = new wxBoxSizer(wxHORIZONTAL);

	/* The list control is a report-view displaying search results. */
	list_control = new CheckedListCtrl(this, SERURO_SEARCH_LIST_ID, 
		wxDefaultPosition, wxDefaultSize, (wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SIMPLE));
	should_clear_list = false;

	/* Set a column image for the checkbox column. */
	int mail_icon_index = list_control->AddImage(wxGetBitmapFromMemory(certificate_icon_18_flat));
	wxListItem check_box_column;
	check_box_column.SetId(0);
	check_box_column.SetImage(mail_icon_index);
	list_control->SetCheckboxColumn(check_box_column);
	list_control->SetColumnWidth(0, 28);
	
	/* Create all of the column for the search results response. 
	 * This must start at the integer 1, where 0 is the place holder for the checkmark. 
	 */
	list_control->InsertColumn(1, _("Email Address"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(2, _("First Name"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(3, _("Last Name"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(4, _("Server Name"), wxLIST_FORMAT_LEFT);

	/* Add the list-control to the UI. */
	results_sizer->Add(list_control, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(results_sizer, 1, wxALL | wxEXPAND, 5);

	/* Define the search controls. */
	wxStaticBoxSizer* const controls_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Search for Certificates"));
	wxBoxSizer *servers_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	/* Create search list. */
	Text *servers_text = new Text(this, wxT("Select server: "));
	
	this->servers_control = GetServerChoice(this);
	/* When checked/unchecked the servers_control is disabled/enabled. */
	this->all_servers_control = new wxCheckBox(this, SERURO_SEARCH_ALL_SERVERS_ID, _("Search all servers"));

	servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
	servers_sizer->Add(this->servers_control, 0, wxRIGHT, 5);
	servers_sizer->Add(this->all_servers_control, 0, wxRight, 5);
	
	/* Create search text-field. */
	this->search_control = new SearchBox(this);

	search_sizer->Add(this->search_control, 1);

	/* All them all into the components sizer. */
	controls_sizer->Add(servers_sizer, 1, wxALL | wxEXPAND, 5);
	controls_sizer->Add(search_sizer, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(controls_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	/* Add the components to the main view. */
	this->SetSizer(components_sizer);

	/* Testing: setting even column widths. */
	this->list_control->SetColumnWidth(0, 25);
	this->list_control->SetColumnWidth(1, SEARCH_PANEL_COLUMN_WIDTH/4);
	this->list_control->SetColumnWidth(2, SEARCH_PANEL_COLUMN_WIDTH/4);
	this->list_control->SetColumnWidth(3, SEARCH_PANEL_COLUMN_WIDTH/4);
    this->list_control->SetColumnWidth(4, SEARCH_PANEL_COLUMN_WIDTH/4);

    /* Debug for now, show a "nothing message" in the list. */
	//this->AddResult(wxString("No Email Address"), wxString("No First Name"), wxString("No Last Name"));
	//this->DisableResult(0);
    
	/* Testing default focus */
	DoFocus();

	/* Create dynamic event binders. */
	wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelSearch::OnServerStateChange, this, STATE_TYPE_SERVER);

	this->Layout();
}

void SeruroPanelSearch::AddResult(const wxString &address,
	const wxString &first_name, const wxString &last_name)
{
	long item_index;
	
	/* place appropriately marked checkbox. */
    wxString server_name;
    wxString server_uuid;
    wxString display_address = address;
    
    /* Get server info from selected server. */
    server_name = this->GetSelectedServer();
    server_uuid = wxGetApp().config->GetServerUUID(server_name);
    
    /* When the certificate is requested, it must know what server manages the identity. */
	item_index = this->list_control->InsertItem(0, wxT(" "));
    if (item_index < 0) {
        wxLogMessage(wxT("SeruroPanelSearch:AddResult> could not insert result (%s)."), address);
        return;
    }
    
    /* Determine if certificate is installed. */
    bool have_certificate = wxGetApp().config->HaveCertificates(server_uuid, address);
    bool have_identity = wxGetApp().config->HaveIdentity(server_uuid, address);
    list_control->SetCheck(item_index, have_certificate);
	if (have_identity) {
        list_control->DisableRow(item_index);
        display_address = display_address + _(" (you)");
    }
    
    /* Add the textual (UI) information. */
	list_control->SetItem(item_index, 1, display_address);
	list_control->SetItem(item_index, 2, first_name);
	list_control->SetItem(item_index, 3, last_name);
    list_control->SetItem(item_index, 4, server_name);
}

void SeruroPanelSearch::DoSearch()
{
	/* Todo this should pick up the selected (or ONLY) server. */
    wxString server_name;
	wxString query;
    wxJSONValue server_info;

    /* Do not search an empty string. */
    query = this->search_control->GetValue();
    if (query.compare(wxEmptyString) == 0) return;
    if (query.Length() < SERURO_MIN_SEARCH_LENGTH) return;
    /* Do not duplicate searches. */
    //if (server_name.compare(searched_server_name) == 0 && query.compare(searched_query) == 0) return;
    
	wxJSONValue params;
	params["query"] = query;

	/* Disable the search box until the query completes. */
	this->DisableSearch();

	/* Cache results to prevent duplicate searches. */
    this->searched_query = query;
	/* Allow the callback to clear the list once. */
	this->should_clear_list = true;

	if (! this->all_servers_control->IsChecked()) {
		server_name = this->GetSelectedServer();
		server_info = this->api->GetServer(wxGetApp().config->GetServerUUID(server_name));
		/* Sanity check for no servers, but an interactive search input. */
		//if (! server.HasMember("host")) {
		//	wxLogMessage(_("SeruroPanelServer> (DoSearch) Invalid server selected."));
		//	return;
		//}

		this->searched_server_name = server_name;
		params["server"] = server_info;

		api->CreateRequest(SERURO_API_SEARCH, params, SERURO_API_CALLBACK_SEARCH)->Run();
	} else {
		/* Send the request to all servers. */
		wxArrayString servers_list = wxGetApp().config->GetServerList();
		this->searched_server_name = wxEmptyString;
		for (size_t i = 0; i < servers_list.size(); i++) {
			server_info = this->api->GetServer(servers_list[i]);
			params["server"] = server_info;

			api->CreateRequest(SERURO_API_SEARCH, params, SERURO_API_CALLBACK_SEARCH)->Run();
		}
	}
}

void SeruroPanelSearch::OnSearchResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* Clear the results list. */
	if (this->should_clear_list) {
		this->should_clear_list = false;
		this->list_control->DeleteAllItems();
	}

	/* Set the cursor back to the input field. */
	this->EnableSearch();
	this->search_control->SetFocus();

	if (! response.HasMember("success") || ! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroPanelSearch> (Search) Bad Result."));
        
        /* Clear "cached" results, allow the user to re-submit search. */
        searched_server_name = _(wxEmptyString);
        searched_query = _(wxEmptyString);
        
		return;
	}

	wxLogMessage(wxT("SeruroPanelTest> (Search) API valid response."));

	/* From the wxWidgets samples, should speed up adding. */
	this->list_control->DeleteAllItems();

	/* Add results to UI. */
	for (int i = 0; i < response["results"].Size(); i++) {
		this->AddResult(response["results"][i]["email"].AsString(),
			response["results"][i]["first_name"].AsString(), 
			response["results"][i]["last_name"].AsString());
	}

}

/* UI helpers during search requests, and results processing. */
void SeruroPanelSearch::DisableSearch()
{
    this->search_control->Enable(false);
    this->servers_control->Enable(false);
    this->all_servers_control->Enable(false);
}

void SeruroPanelSearch::EnableSearch()
{
    this->search_control->Enable(true);
    this->servers_control->Enable(true);
    this->all_servers_control->Enable(true);
}

void SeruroPanelSearch::AlignList()
{
	wxListCtrl *lists[] = {list_control};
	MaximizeAndAlignLists(lists, 1, 1);
}