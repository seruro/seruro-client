
#include "CheckedListCtrl.h"
#include "../SeruroPanelSearch.h"

#include <wx/log.h>
#include <wx/renderer.h>
#include <wx/dcmemory.h>

BEGIN_EVENT_TABLE(CheckedListCtrl, wxListCtrl)
	/* Make sure they cannot select a disabled row. */
	EVT_LIST_ITEM_SELECTED(SERURO_SEARCH_LIST_ID, CheckedListCtrl::OnItemSelected)

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

/* Add an image to the automatically-generated list of checkboxes. */
int CheckedListCtrl::AddImage(const wxBitmap &bitmap)
{
	this->m_imageList.Add(bitmap);
	return this->m_imageList.GetImageCount() - 1;
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
	//list_column.SetText(wxT("*")); /* A checkmark. */
	this->InsertColumn(SEARCH_LIST_IMAGE_COLUMN, list_column);
}

void CheckedListCtrl::SetCheckboxColumn(wxListItem column)
{
	this->SetColumn(SEARCH_LIST_IMAGE_COLUMN, column);
}

void CheckedListCtrl::DisableRow(long item)
{
	/* Do not worry about the checkbox for a disabled row. */

	//for (int i = 0; i < this->GetColumnCount(); i++) {}
	this->SetItemImage(item, (IsChecked(item)) ? DISABLED_CHECKED : DISABLED_UNCHECKED);
	this->SetItemTextColour(item, wxColour(DISABLED_TEXT_COLOR));
	this->SetItemBackgroundColour(item, wxColour(DISABLED_BACKGROUND_COLOR));
}

void CheckedListCtrl::OnItemSelected(wxListEvent &event)
{
	/* Do not allow a disabled row to be selected. */
	wxLogMessage(_("CheckedListCtrl> (OnItemSelected) item (%d) trying to select."), event.GetId());
	if (IsDisabled(event.GetId())) {
		wxLogMessage(_("CheckedListCtrl> (OnItemSelected) item is disabled."));
		SetItemState(event.GetId(), 0, wxLIST_STATE_SELECTED);
		event.Veto();
	}
}

bool CheckedListCtrl::IsDisabled(long item) const
{
	wxListItem info;
	info.SetId(item);
	info.SetMask(wxLIST_MASK_IMAGE);
	if (GetItem(info)) {
		return (info.GetImage() == DISABLED_UNCHECKED || info.GetImage() == DISABLED_CHECKED);
	}
	return false;
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
    wxListItem address, server;
    
	/* Do now allow actions on a disabled item. */
	if (IsDisabled(item)) return;

    /* Set the mask as the type of information requested using GetItem. */
    address.SetMask(wxLIST_MASK_TEXT);
    address.SetId(item);
    address.SetColumn(SEARCH_LIST_ADDRESS_COLUMN);
    
    server.SetMask(wxLIST_MASK_TEXT);
    server.SetId(item);
    server.SetColumn(SEARCH_LIST_SERVER_COLUMN);
    
    /* Request the text for the item (item). */
    if (! this->GetItem(address) || ! this->GetItem(server)) {
        wxLogMessage(wxT("CheckedListCtrl:Check> Cannot find an item at index (%d)."), item);
        return;
    }
    
	if (this->IsChecked(item)) {
        ((SeruroPanelSearch *) this->parent)->Uninstall(server.GetText(), address.GetText());
	} else {
        ((SeruroPanelSearch *) this->parent)->Install(server.GetText(), address.GetText());
    }
}

void CheckedListCtrl::SetCheck(long item, bool checked)
{
    SetItemImage(item, (checked ? 1 : 0), -1);
}

void CheckedListCtrl::SetCheck(const wxString &server_name, const wxString &address, bool checked)
{
    wxListItem item_address, item_server;

    /* Set constant mask and column. */
    item_address.SetMask(wxLIST_MASK_TEXT);
    item_address.SetColumn(SEARCH_LIST_ADDRESS_COLUMN);
    item_server.SetMask(wxLIST_MASK_TEXT);
    item_server.SetColumn(SEARCH_LIST_SERVER_COLUMN);
    
    for (long i = this->GetItemCount()-1; i >= 0; i--) {
        item_address.SetId(i);
        item_server.SetId(i);
        if (!this->GetItem(item_address) || !this->GetItem(item_server)) {
            wxLogMessage(wxT("SetCheck> could not get item (%d)."), i);
            continue;
        }
        
        wxLogMessage(wxT("SetCheck> trying (%d) with address of (%s) (%s)."), i,
            item_server.GetText(), item_address.GetText());
        
        /* If this is a valid item, compare the address to the installed address. */
        if (item_address.GetText().compare(address) == 0 && item_server.GetText().compare(server_name) == 0) {
            wxLogMessage(wxT("SetCheck> checking item (%d) with address (%s) (%s)."), i, server_name, address);
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
