
#include "CheckedListCtrl.h"
#include "../SeruroPanelSearch.h"

#include <wx/renderer.h>

#define SEARCH_LIST_IMAGE_COLUMN 0
#define SEARCH_LIST_ADDRESS_COLUMN 1
#define SEARCH_LIST_SERVER_COLUMN 4

#define SEARCH_LIST_ITEM_UNCHECKED 0
#define SEARCH_LIST_ITEM_CHECKED 1

BEGIN_EVENT_TABLE(CheckedListCtrl, wxListCtrl)
	/* User clicks a list row. */
    EVT_LEFT_DCLICK(CheckedListCtrl::OnMouseEvent)
	EVT_LEFT_DOWN(CheckedListCtrl::OnMouseEvent)
	/* User tries to resize a column. */
	EVT_LIST_COL_BEGIN_DRAG(SERURO_SEARCH_LIST_ID, CheckedListCtrl::OnColumnDrag)
END_EVENT_TABLE()

int wxCALLBACK CompareAddresses(wxIntPtr item1, wxIntPtr item2, wxIntPtr WXUNUSED(sort))
{
    // inverse the order
    if (item1 < item2)
        return 1;
    if (item1 > item2)
        return -1;
    return 0;
}

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

CheckedListCtrl::CheckedListCtrl(wxWindow* parent, wxWindowID id,
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
        wxLogMessage(_("debug: cannot uninstall certificate."));
		return;
        
        //this->parent->Uninstall(identity);
	} else {
        ((SeruroPanelSearch *) this->parent)->Install(address.GetText(), server.GetText());
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

void CheckedListCtrl::FilterResultsByServers(wxArrayString servers)
{
	wxListItem item;
	bool server_exists;

	/* Search each row's server. */
	item.SetMask(wxLIST_MASK_TEXT);
	item.SetColumn(SEARCH_LIST_SERVER_COLUMN);

	for (long i = this->GetItemCount()-1; i >= 0; i--) {
		item.SetId(i);
		if (! this->GetItem(item)) { continue; }

		/* The row's server must be in the list provided. */
		server_exists = false;
		for (size_t j = 0; j < servers.size(); j++) {
			if (item.GetText().compare(servers[j]) == 0) server_exists = true;
		}

		if (! server_exists) {
			this->DeleteItem(i);
		}
	}
}
