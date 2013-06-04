
#include "SeruroPanelSearch.h"
#include "../SeruroClient.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"
#include "../wxJSON/wx/jsonwriter.h"

//#include <wx/stattext.h>
//#include <wx/button.h>
//#include <wx/ustring.h>
//#include <wx/checkbox.h>
#include <wx/renderer.h>

#define SEARCH_LIST_IMAGE_COLUMN 0
#define SEARCH_LIST_ADDRESS_COLUMN 1
#define SEARCH_LIST_SERVER_COLUMN 4

#define SEARCH_LIST_ITEM_UNCHECKED 0
#define SEARCH_LIST_ITEM_CHECKED 1

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroPanelSearch, wxPanel)
	//EVT_BUTTON(SEARCH_BUTTON_SEARCH, SeruroPanelSearch::OnSearch)
	/* API call completes. */
	//EVT_COMMAND(SERURO_API_CALLBACK_SEARCH, SERURO_API_RESULT, SeruroPanelSearch::OnSearchResult)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_SEARCH, SeruroPanelSearch::OnSearchResult)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_GET_CERT, SeruroPanelSearch::OnInstallResult)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SearchBox, wxSearchCtrl)
	/* User wants to search. */
	EVT_SEARCHCTRL_SEARCH_BTN(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
	EVT_TEXT_ENTER(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CheckedListCtrl, wxListCtrl)
	/* User clicks a list row. */
    EVT_LEFT_DCLICK(CheckedListCtrl::OnMouseEvent)
	EVT_LEFT_DOWN(CheckedListCtrl::OnMouseEvent)
	/* User tries to resize a column. */
	EVT_LIST_COL_BEGIN_DRAG(SERURO_SEARCH_LIST_ID, CheckedListCtrl::OnColumnDrag)
END_EVENT_TABLE()


/* Check or uncheck the checkbox, the appropriateness and applicability of this function
 * should be determined by the event handler registrations, eg. LEFT_DCLICK, LEFT_DOWN
 */
void CheckedListCtrl::OnMouseEvent(wxMouseEvent &event)
{
    int flags;
    long item = HitTest(event.GetPosition(), flags);
    if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON)) {
		/* Allow all event to pass into the hitbox check. */
        DoCheck(item, !IsChecked(item));
    } else { 
		event.Skip(); 
	}
}

/* Todo: a custom class for the search panel should inherit from the checked list control. */
void CheckedListCtrl::OnColumnDrag(wxListEvent &event)
{
	if (event.GetColumn() == SEARCH_LIST_IMAGE_COLUMN) {
		/* Stop resizing of the first column. */
		event.Veto();
	}
}

CheckedListCtrl::CheckedListCtrl(SeruroPanelSearch* parent, wxWindowID id,
    const wxPoint& pt, const wxSize& sz, long style)
    : wxListCtrl(parent, id, pt, sz, style), parent(parent),
    m_imageList(CHECKBOX_SIZE, CHECKBOX_SIZE, true)
{
    SetImageList(&m_imageList, wxIMAGE_LIST_SMALL);
    
    /* Todo: Using the native size would be better. */
    wxBitmap unchecked_bmp(CHECKBOX_SIZE, CHECKBOX_SIZE),
		checked_bmp(CHECKBOX_SIZE, CHECKBOX_SIZE),
		unchecked_disabled_bmp(CHECKBOX_SIZE, CHECKBOX_SIZE),
		checked_disabled_bmp(CHECKBOX_SIZE, CHECKBOX_SIZE);
    
    /* Bitmaps must not be selected by a DC for addition to the image list 
     * but I don't see a way of diselecting them in wxMemoryDC so let's just 
     * use a code block to end the scope.
     */
    {
        wxMemoryDC renderer_dc;
        
        /* Unchecked. */
        renderer_dc.SelectObject(unchecked_bmp);
        renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc,
            wxRect(0, 0, CHECKBOX_SIZE, CHECKBOX_SIZE), 0);
        
        /* Checked. */
        renderer_dc.SelectObject(checked_bmp);
        renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc,
            wxRect(0, 0, CHECKBOX_SIZE, CHECKBOX_SIZE), wxCONTROL_CHECKED);
        
        /* Unchecked and Disabled. */
        renderer_dc.SelectObject(unchecked_disabled_bmp);
        renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc,
            wxRect(0, 0, CHECKBOX_SIZE, CHECKBOX_SIZE), 0 | wxCONTROL_DISABLED);
        
        /* Checked and Disabled. */
        renderer_dc.SelectObject(checked_disabled_bmp);
        renderer_dc.SetBackground(*wxTheBrushList->FindOrCreateBrush(GetBackgroundColour(), wxSOLID));
        renderer_dc.Clear();
        wxRendererNative::Get().DrawCheckBox(this, renderer_dc,
            wxRect(0, 0, CHECKBOX_SIZE, CHECKBOX_SIZE), wxCONTROL_CHECKED | wxCONTROL_DISABLED);
    }
    
    /* the add order must respect the wxCLC_XXX_IMGIDX defines in the headers ! */
    m_imageList.Add(unchecked_bmp);
    m_imageList.Add(checked_bmp);
    m_imageList.Add(unchecked_disabled_bmp);
    m_imageList.Add(checked_disabled_bmp);

	/* Add a 0th column, which cannot be resized, to hold the checkmark. */
	wxListItem list_column;
	list_column.SetText(wxT("*")); /* A checkmark. */
	this->InsertColumn(SEARCH_LIST_IMAGE_COLUMN, list_column);
}

bool CheckedListCtrl::IsChecked(long item) const
{
    wxListItem info;
    info.SetMask(wxLIST_MASK_IMAGE);
    info.SetId(item);
    
    if (GetItem(info)) { 
		return (info.GetImage() == SEARCH_LIST_ITEM_CHECKED);
	} else { 
		return false; 
	}
}

/* The user has checked/unchecked an item. */
void CheckedListCtrl::DoCheck(long item, bool checked)
{
    //IdentityItem info;
    //wxJSONValue identity;
    wxListItem address, server;
    
    /* Set the mask as the type of information requested using GetItem. */
    address.SetMask(wxLIST_MASK_TEXT);
    address.SetId(item);
    address.SetColumn(SEARCH_LIST_ADDRESS_COLUMN);
    
    server.SetMask(wxLIST_MASK_TEXT);
    server.SetId(item);
    server.SetColumn(SEARCH_LIST_SERVER_COLUMN);
    
    /* Request the text for the item (item). */
    if (! this->GetItem(address)) {
        wxLogMessage(wxT("CheckedListCtrl:Check> Cannot find an item at index (%d)."), item);
        return;
    }
    this->GetItem(server);
    //identity = info.GetIdentity();
    
	if (this->IsChecked(item)) {
		/* No uninstalling as of now. */
        wxLogMessage("debug: cannot uninstall certificate.");
		return;
        
        //this->parent->Uninstall(identity);
	} else {
        this->parent->Install(address.GetText(), server.GetText());
    }
}

void CheckedListCtrl::SetCheck(long item, bool checked)
{
    SetItemImage(item, (checked ? 1 : 0), -1);
}

void CheckedListCtrl::SetCheck(const wxString &address, bool checked)
{
    wxListItem item_address;

    /* Set constant mask and column. */
    item_address.SetMask(wxLIST_MASK_TEXT);
    item_address.SetColumn(SEARCH_LIST_ADDRESS_COLUMN);
    
    for (long i = this->GetItemCount()-1; i >= 0; i--) {
        item_address.SetId(i);
        if (!this->GetItem(item_address)) {
            wxLogMessage(wxT("SetCheck> could not get item (%d)."), i);
            continue;
        }
        
        wxLogMessage(wxT("SetCheck> trying (%d) with address of (%s)."), i, item_address.GetText());
        
        /* If this is a valid item, compare the address to the installed address. */
        if (item_address.GetText().compare(address) == 0) {
            wxLogMessage(wxT("SetCheck> checking item (%d) with address (%s)."), i, address);
            this->SetCheck(i, checked);
        }
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
    
	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CERT, params, SERURO_API_CALLBACK_GET_CERT);
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
    if (api->InstallCert(response)) {
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

	/* The list control is a report-view displaying search results. */
	list_control = new CheckedListCtrl(this, SERURO_SEARCH_LIST_ID, 
		wxDefaultPosition, wxDefaultSize, (wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME));

	wxListItem list_column;

	/* Create all of the column for the search results response. 
	 * This must start at the integer 1, where 0 is the place holder for the checkmark. 
	 */
	list_column.SetText(wxT("Email Address"));
	list_control->InsertColumn(1, list_column);
	list_column.SetText(wxT("First Name"));
	list_control->InsertColumn(2, list_column);
	list_column.SetText(wxT("Last Name"));
	list_control->InsertColumn(3, list_column);
    list_column.SetText(wxT("Server"));
    list_control->InsertColumn(4, list_column);

	/* Add the list-control to the UI. */
	results_sizer->Add(list_control, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(results_sizer, 1, wxALL | wxEXPAND, 5);

	/* Define the search controls. */
	wxStaticBoxSizer* const controls_sizer = new wxStaticBoxSizer(wxVERTICAL, this, wxT("Search for Certificates"));
	wxBoxSizer *servers_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *search_sizer = new wxBoxSizer(wxHORIZONTAL);
	//wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);

	/* Create search list. */
	wxArrayString servers_list;
	servers_list = wxGetApp().config->GetServerList();
	if (servers_list.size() != 0) {
		/* Only create list if there are multiple servers configured. */
		Text *servers_text = new Text(this, wxT("Select server:"));
		this->servers_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, servers_list);
		this->servers_control->SetSelection(0);

		servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
		servers_sizer->Add(this->servers_control, 0, wxRIGHT, 5);
	}

	/* Create search text-field. */
	this->search_control = new SearchBox(this);

	//search_sizer->Add(search_text, 0, wxRIGHT, 5);
	search_sizer->Add(this->search_control, 1);

	/* All them all into the components sizer. */
	controls_sizer->Add(servers_sizer, 1, wxALL | wxEXPAND, 5);
	controls_sizer->Add(search_sizer, 1, wxALL | wxEXPAND, 5);
	//controls_sizer->Add(buttons_sizer, 1, wxALL | wxEXPAND, 5);
	components_sizer->Add(controls_sizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

	/* Add the components to the main view. */
	this->SetSizer(components_sizer);

	/* Testing: setting even column widths. */
	//int list_column_size = SEARCH_PANEL_COLUMN_WIDTH;
	this->list_control->SetColumnWidth(0, 25);
	this->list_control->SetColumnWidth(1, SEARCH_PANEL_COLUMN_WIDTH/4);
	this->list_control->SetColumnWidth(2, SEARCH_PANEL_COLUMN_WIDTH/4);
	this->list_control->SetColumnWidth(3, SEARCH_PANEL_COLUMN_WIDTH/4);
    this->list_control->SetColumnWidth(4, SEARCH_PANEL_COLUMN_WIDTH/4);

    /* Debug for now, show a "nothing message" in the list. */
	this->AddResult(wxString("No Email Address"),
        wxString("No First Name"), wxString("No Last Name"));
    
	/* Testing default focus */
	this->search_control->SetFocus();
	this->Layout();
}

void SeruroPanelSearch::AddResult(const wxString &address,
	const wxString &first_name, const wxString &last_name)
{
	long item_index;
	
	/* place appropriately marked checkbox. */
    //IdentityItem item;
    wxString server_name = this->GetSelectedServer();
    
    /* When the certificate is requested, it must know what server manages the identity. */
    //item.SetIdentity(address, server_name);
    //wxListItem item;
	item_index = this->list_control->InsertItem(0, wxT(" "));
    if (item_index < 0) {
        wxLogMessage(wxT("SeruroPanelSearch:AddResult> could not insert identity (%s)."), address);
        return;
    }
    //this->list_control->SetItemText(item_index, address);
    
    /* Determine if certificate is installed. */
    list_control->SetCheck(item_index, false);
	
    /* Add the textual (UI) information. */
	list_control->SetItem(item_index, 1, address);
	list_control->SetItem(item_index, 2, first_name);
	list_control->SetItem(item_index, 3, last_name);
    list_control->SetItem(item_index, 4, server_name);
}

void SeruroPanelSearch::OnSearch(wxCommandEvent &event)
{
	this->DoSearch();
}

void SeruroPanelSearch::DoSearch()
{
	/* Todo this should pick up the selected (or ONLY) server. */
    wxString server_name = this->GetSelectedServer();
	wxString query = this->search_control->GetValue();

	wxJSONValue server = this->api->GetServer(server_name);	
	
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
	//wxJSONReader reader;
	wxJSONValue response = event.GetResponse();
	//wxString responseString = event.GetString();
	//reader.Parse(responseString, &response);

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

