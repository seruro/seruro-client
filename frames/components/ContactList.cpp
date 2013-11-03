
#include "ContactList.h"

#include "../../SeruroClient.h"
#include "../../SeruroConfig.h"
#include "../../crypto/SeruroCrypto.h"
#include "../UIDefs.h"
#include "../ImageDefs.h"

/* Image data. */
//#include "../../resources/images/blank.png.h"
//#include "../../resources/images/certificate_icon_12_flat.png.h"

/* Status (for accounts/identities) */
//#include "../../resources/images/check_icon_12_flat.png.h"
//#include "../../resources/images/cross_icon_12_flat.png.h"

#define ITEM_IMAGE_EXISTS 2
#define ITEM_IMAGE_NOT_EXISTS 0

DECLARE_APP(SeruroClient);

void ContactList::OnContactStateChange(SeruroStateEvent &event)
{
    if (event.GetAction() == STATE_ACTION_ADD) {
        this->AddContact(event.GetAccount(), event.GetServerUUID());
    } else if (event.GetAction() == STATE_ACTION_REMOVE) {
        this->RemoveContact(event.GetAccount(), event.GetServerUUID());
    } else {
        /* No implementation for update as of yet. */
        this->GenerateContactList();
    }

    event.Skip();
}

/* A certificate changes. */
void ContactList::OnCertificateStateChange(SeruroStateEvent &event)
{
    /* Brute force regenerate. */
    this->GenerateContactList();
}

/* An identity is added. */
void ContactList::OnIdentityStateChange(SeruroStateEvent &event)
{
    /* Brute force regenerate. */
    this->GenerateContactList();
}

/* Make sure we property identify/call out identity-accounts. */
void ContactList::OnAccountStateChange(SeruroStateEvent &event)
{
    
}

void ContactList::Create(wxWindow *parent)
{
    this->parent = parent;
    this->contact_list = new wxListCtrl(parent, CONTACT_LIST_ID, wxDefaultPosition, wxDefaultSize,
        (wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SIMPLE));
    
    list_images = new wxImageList(SERURO_IMAGE_LIST_WIDTH, SERURO_IMAGE_LIST_HEIGHT, true);
    list_images->Add(wxGetBitmapFromMemory(blank_icon));
    list_images->Add(wxGetBitmapFromMemory(certificate_icon_flat));
    list_images->Add(wxGetBitmapFromMemory(check_icon_flat));
    
    this->contact_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);
    
    /* Create image column. */
    wxListItem image_column;
    image_column.SetId(0);
    image_column.SetImage(1);
    
    /* Add to the list control. */
    this->contact_list->InsertColumn(0, image_column);
    this->contact_list->SetColumnWidth(0, 25);
    
    this->contact_list->InsertColumn(1, _("Email Address"), wxLIST_FORMAT_LEFT);
    this->contact_list->InsertColumn(2, _("First Name"), wxLIST_FORMAT_LEFT);
    this->contact_list->InsertColumn(3, _("Last Name"), wxLIST_FORMAT_LEFT);
    this->contact_list->InsertColumn(4, _("Server Name"), wxLIST_FORMAT_LEFT);
    
	this->contact_list->SetColumnWidth(1, SEARCH_PANEL_COLUMN_WIDTH/3);
	this->contact_list->SetColumnWidth(2, SEARCH_PANEL_COLUMN_WIDTH/4.55);
	this->contact_list->SetColumnWidth(3, SEARCH_PANEL_COLUMN_WIDTH/4.55);
    this->contact_list->SetColumnWidth(4, SEARCH_PANEL_COLUMN_WIDTH/4.55);

    /* Use the app object to bind. */
    wxGetApp().Bind(wxEVT_LIST_COL_BEGIN_DRAG, &ContactList::OnContactColumnDrag, this, CONTACT_LIST_ID);
    wxGetApp().Bind(wxEVT_LIST_ITEM_SELECTED, &ContactList::OnContactSelected, this, CONTACT_LIST_ID);
}

void ContactList::AddContactList(wxSizer *sizer)
{
    sizer->Add(this->contact_list, 1, wxALL | wxEXPAND, 5);
}

void ContactList::GenerateContactList()
{
    wxArrayString servers;
    wxArrayString contacts;
    
    /* Reset the list. */
    this->contact_list->DeleteAllItems();

    servers = theSeruroConfig::Get().GetServerList();
    for (size_t i = 0; i < servers.size(); ++i) {
        contacts = theSeruroConfig::Get().GetContactsList(servers[i]);
        for (size_t j = 0; j < contacts.size(); ++j) {
            this->AddContact(contacts[j], servers[i]);
        }
    }
    
    /* Nothing left. */
}

void ContactList::AddContact(wxString address, wxString server_uuid)
{
    long item_index;
    wxString server_name;
    wxJSONValue contact;
    SeruroCrypto crypto;
    
    bool contact_complete;

    server_name = theSeruroConfig::Get().GetServerName(server_uuid);
    contact = theSeruroConfig::Get().GetContact(server_uuid, address);
    
    /* Create a new row (complete means certificates exists). */
    contact_complete = (crypto.HaveCertificates(server_uuid, address) || crypto.HaveIdentity(server_uuid, address));
    item_index = this->contact_list->InsertItem(0, wxEmptyString,
        (contact_complete) ? ITEM_IMAGE_EXISTS : ITEM_IMAGE_NOT_EXISTS);

    /* Update the column/row if the address is an identity. */
    if (theSeruroConfig::Get().AddressExists(address)) {
        this->contact_list->SetItem(item_index, 1, wxString::Format(_("%s"), address));
        //this->contact_list->SetItem(item_index, 1, wxString::Format(_("%s (you)"), address));
        this->contact_list->SetItemTextColour(item_index, wxColour(DISABLED_TEXT_COLOR));
    } else {
        /* Otherwise a normal contact. */
        this->contact_list->SetItem(item_index, 1, address);
    }
    
    this->contact_list->SetItem(item_index, 2, contact["name"][0].AsString());
    this->contact_list->SetItem(item_index, 3, contact["name"][1].AsString());
    this->contact_list->SetItem(item_index, 4, server_name);
}

void ContactList::RemoveContact(wxString address, wxString server_uuid)
{
    //long item_index;
    wxListItem server_item, address_item;
    wxString server_name;
    
    address_item.SetMask(wxLIST_MASK_TEXT);
    address_item.SetColumn(1);

    server_item.SetMask(wxLIST_MASK_TEXT);
    server_item.SetColumn(4);
    
    server_name = theSeruroConfig::Get().GetServerName(server_uuid);
    for (int i = 0; i < this->contact_list->GetItemCount(); i++) {
        address_item.SetId(i);
        server_item.SetId(i);
        
        if (! contact_list->GetItem(address_item) || ! contact_list->GetItem(server_item)) {
            continue;
        }
        
        /* If both match, remove. */
        if (address_item.GetText() == address && server_item.GetText() == server_name) {
            contact_list->DeleteItem(i);
        }
    }
}

bool ContactList::SelectContact(long index)
{
    wxListItem address_item;
    
    address_item.SetMask(wxLIST_MASK_TEXT);
    address_item.SetColumn(1);
    address_item.SetId(index);
    
    if (! contact_list->GetItem(address_item)) {
        return false;
    }
    
    /* The address exists as an account, do not select the item. */
    if (theSeruroConfig::Get().AddressExists(address_item.GetText())) {
        contact_list->SetItemState(index, 0, wxLIST_STATE_SELECTED);
        return false;
    }
    
    return true;
}

void ContactList::OnContactSelected(wxListEvent &event)
{
    long index = event.GetIndex();
    
    if (! this->SelectContact(index)) {
        event.Veto();
        return;
    }
}

void ContactList::OnContactColumnDrag(wxListEvent &event)
{
    /* This could be better defined, assumes image=0. */
    if (event.GetColumn() == 0) {
        event.Veto();
    }
}
