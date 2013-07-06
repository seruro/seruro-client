
#include "SeruroPanelSearch.h"
#include "../SeruroClient.h"

/* Need GetServerChoice */
#include "dialogs/AddServerDialog.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"
#include "../wxJSON/wx/jsonwriter.h"

/* Image data. */
#include "../resources/images/certificate_icon_18_flat.png.h"

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroPanelSearch, wxPanel)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_SEARCH, SeruroPanelSearch::OnSearchResult)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CERTS, SeruroPanelSearch::OnInstallResult)

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
		wxLogMessage(_("SeruroPanelSearch> (OnServerStateChange) removing server (%s)."), event.GetServerName());
	}
    
    /* Focus should update the servers list and filter results. */
    this->DoFocus();
    /* Allow other handlers. */
	event.Skip();
}

void SeruroPanelSearch::DoFocus()
{
	wxArrayString servers = wxGetApp().config->GetServerList();
	wxLogMessage(_("SeruroPanelSearch> (DoFocus) focusing the search."));

	/* If the config has changed, regenerate the list of servers. */
	if (servers.size() != servers_control->GetCount()) {
		this->servers_control->Clear();
		this->servers_control->Append(servers);
		this->servers_control->SetSelection(0);

		/* Filter search results for potentially-removed servers. */
		this->list_control->FilterResultsByServers(servers);
	}

	/* Depending on the number of servers, enable/disable the controls. */
	if (servers.size() == 0) {
		/* This may have been active before, and there may be dangling text. */
		this->search_control->Clear();
		this->DisableSearch();
	} else {
		this->EnableSearch();
		this->search_control->SetFocus();
	}
}

void SeruroPanelSearch::Install(const wxString& address, const wxString& server_name)
{
    wxJSONValue params;
    wxJSONValue server;
    
    wxLogMessage(wxT("SeruroPanelSearch:Install> requesting certificate for (%s) (%s)."), address, server_name);
    
    server = api->GetServer(server_name);
    params["server"] = server;
    params["request_address"] = address;
    
	SeruroRequest *request = api->CreateRequest(SERURO_API_CERTS, params, SERURO_API_CALLBACK_CERTS);
	request->Run();
}

/* After the certificate is installed, update the UI. */
void SeruroPanelSearch::OnInstallResult(SeruroRequestEvent &event)
{
	/* The event data should include the address which was updated. */
    wxJSONValue response = event.GetResponse();
    
    if (! response.HasMember("success") || ! response["success"].AsBool()
        || ! response.HasMember("address")) {
        wxLogMessage(wxT("OnInstallResults> Bad Result."));
        return;
    }
    
    if (! response.HasMember("certs") || response["certs"].Size() == 0) {
        wxLogMessage(wxT("OnInstallResult> No certs found for address (%s)."), response["address"].AsString());
        return;
    }
    
    /* Check the corresponding item(s) in the list control. */
    if (api->InstallCertificate(response)) {
        this->list_control->SetCheck(response["address"].AsString(), true);
    }
}

/* There is no callback for uninstall, this happens locally. */
void SeruroPanelSearch::Uninstall(const wxString& address, const wxString& server_name)
{
    
}

SeruroPanelSearch::SeruroPanelSearch(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Search"))
{
	/* Create an API object for searching. */
	this->api = new SeruroServerAPI(this->GetEventHandler());

	/* The components sizer hold all UI elements. */
	components_sizer = new wxBoxSizer(wxVERTICAL);

	/* The results sizer holds only the list control. */
	wxBoxSizer *results_sizer = new wxBoxSizer(wxHORIZONTAL);

	//wxImageList *list_images = new wxImageList(12, 12, true);
	//list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));

	/* The list control is a report-view displaying search results. */
	list_control = new CheckedListCtrl(this, SERURO_SEARCH_LIST_ID, 
		wxDefaultPosition, wxDefaultSize, (wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SIMPLE));
	//list_control->SetImageList(list_images, wxIMAGE_LIST_SMALL);

	/* Set a column image for the checkbox column. */
	int mail_icon_index = list_control->AddImage(wxGetBitmapFromMemory(certificate_icon_18_flat));
	wxListItem check_box_column;
	check_box_column.SetId(0);
	check_box_column.SetImage(mail_icon_index);
	list_control->SetCheckboxColumn(check_box_column);
	list_control->SetColumnWidth(0, 24);
	
	/* Create all of the column for the search results response. 
	 * This must start at the integer 1, where 0 is the place holder for the checkmark. 
	 */
	list_control->InsertColumn(1, _("Email Address"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(2, _("First Name"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(3, _("Last Name"), wxLIST_FORMAT_LEFT);
	list_control->InsertColumn(4, _("Server"), wxLIST_FORMAT_LEFT);

	/* Add the list-control to the UI. */
	results_sizer->Add(list_control, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(results_sizer, 1, wxALL | wxEXPAND, 5);

	/* Define the search controls. */
	wxStaticBoxSizer* const controls_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Search for Certificates"));
	wxBoxSizer *servers_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	/* Create search list. */
	Text *servers_text = new Text(this, wxT("Select server:"));
	
	this->servers_control = GetServerChoice(this);

	servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
	servers_sizer->Add(this->servers_control, 0, wxRIGHT, 5);
	
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
	this->AddResult(wxString("No Email Address"), wxString("No First Name"), wxString("No Last Name"));
	this->DisableResult(0);
    
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
    wxString server_name = this->GetSelectedServer();
    
    /* When the certificate is requested, it must know what server manages the identity. */
	item_index = this->list_control->InsertItem(0, wxT(" "));
    if (item_index < 0) {
        wxLogMessage(wxT("SeruroPanelSearch:AddResult> could not insert identity (%s)."), address);
        return;
    }
    
    /* Determine if certificate is installed. */
    list_control->SetCheck(item_index, false);
	
    /* Add the textual (UI) information. */
	list_control->SetItem(item_index, 1, address);
	list_control->SetItem(item_index, 2, first_name);
	list_control->SetItem(item_index, 3, last_name);
    list_control->SetItem(item_index, 4, server_name);
}

void SeruroPanelSearch::DoSearch()
{
	/* Todo this should pick up the selected (or ONLY) server. */
    wxString server_name = this->GetSelectedServer();
	wxString query = this->search_control->GetValue();

	wxJSONValue server = this->api->GetServer(server_name);
	/* Sanity check for no servers, but an interactive search input. */
	if (! server.HasMember("host")) {
		wxLogMessage(_("SeruroPanelServer> (DoSearch) Invalid server selected."));
		return;
	}
	
	wxJSONValue params;
	params["query"] = query;
	params["server"] = server;

	/* Disable the search box until the query completes. */
	this->DisableSearch();
	
	SeruroRequest *request = api->CreateRequest(SERURO_API_SEARCH, params, SERURO_API_CALLBACK_SEARCH);
	request->Run();
}

void SeruroPanelSearch::OnSearchResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* Clear the results list. */
	this->list_control->DeleteAllItems();

	/* Set the cursor back to the input field. */
	this->EnableSearch();
	this->search_control->SetFocus();

	if (! response.HasMember("success") || ! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroPanelSearch> (Search) Bad Result."));
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
}

void SeruroPanelSearch::EnableSearch()
{
    this->search_control->Enable(true);
    this->servers_control->Enable(true);
}

