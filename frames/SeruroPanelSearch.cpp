
#include "SeruroPanelSearch.h"
#include "../SeruroClient.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../wxJSON/wx/jsonreader.h"

//#include <wx/stattext.h>
//#include <wx/button.h>
//#include <wx/ustring.h>
//#include <wx/checkbox.h>
#include <wx/renderer.h>

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SeruroPanelSearch, wxPanel)
	//EVT_BUTTON(SEARCH_BUTTON_SEARCH, SeruroPanelSearch::OnSearch)
	/* API call completes. */
	EVT_COMMAND(SERURO_API_CALLBACK_SEARCH, SERURO_API_RESULT, SeruroPanelSearch::OnSearchResult)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SearchBox, wxSearchCtrl)
	/* User wants to search. */
	EVT_SEARCHCTRL_SEARCH_BTN(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
	EVT_TEXT_ENTER(SERURO_SEARCH_TEXT_INPUT_ID, SearchBox::OnSearch)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(wxCheckedListCtrl, wxListCtrl)
	/* User clicks a list row. */
    EVT_LEFT_DCLICK(wxCheckedListCtrl::OnMouseEvent)
	EVT_LEFT_DOWN(wxCheckedListCtrl::OnMouseEvent)
	/* User tries to resize a column. */
	EVT_LIST_COL_BEGIN_DRAG(SERURO_SEARCH_LIST_ID, wxCheckedListCtrl::OnColumnDrag)
END_EVENT_TABLE()

/* Check or uncheck the checkbox, the appropriateness and applicability of this function
 * should be determined by the event handler registrations, eg. LEFT_DCLICK, LEFT_DOWN
 */
void wxCheckedListCtrl::OnMouseEvent(wxMouseEvent &event)
{
    int flags;
    long item = HitTest(event.GetPosition(), flags);
    if (item > -1 && (flags & wxLIST_HITTEST_ONITEMICON)) {
		/* Allow all event to pass into the hitbox check. */
			Check(item, !IsChecked(item));
    } else { 
		event.Skip(); 
	}
}

/* Todo: a custom class for the search panel should inherit from the checked list control. */
void wxCheckedListCtrl::OnColumnDrag(wxListEvent &event)
{
	if (event.GetColumn() == 0) {
		/* Stop resizing of the first column. */
		event.Veto();
	}
}

wxCheckedListCtrl::wxCheckedListCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pt,
    const wxSize& sz, long style): wxListCtrl(parent, id, pt, sz, style),
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
	this->InsertColumn(0, list_column);
}

bool wxCheckedListCtrl::IsChecked(long item) const
{
    wxListItem info;
    info.m_mask = wxLIST_MASK_IMAGE;
    info.m_itemId = item;
    
    if (GetItem(info)) { 
		return (info.m_image == 1); 
	} else { 
		return false; 
	}
}

void wxCheckedListCtrl::Check(long item, bool checked)
{
    SetItemImage(item, (checked ? 1 : 0), -1);
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
	list_control = new wxCheckedListCtrl(this, SERURO_SEARCH_LIST_ID, 
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

	/* Debug for now, show a "nothing message" in the list. */
	this->AddResult(wxString("No Email Address"), 
		wxString("No First Name"), wxString("No Last Name"));

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
	int list_column_size = SEARCH_PANEL_COLUMN_WIDTH;
	this->list_control->SetColumnWidth(0, 25);
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
	item_index = this->list_control->InsertItem(0, wxT(""));
    list_control->Check(item_index, true);
	
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
	this->list_control->DeleteAllItems();

	/* Add results to UI. */
	for (int i = 0; i < response["results"].Size(); i++) {
		this->AddResult(response["results"][i]["email"].AsString(),
			response["results"][i]["first_name"].AsString(), 
			response["results"][i]["last_name"].AsString());
	}

}
