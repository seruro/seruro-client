
#include "SeruroPanelContacts.h"
#include "UIDefs.h"

#include "../SeruroClient.h"

DECLARE_APP(SeruroClient);

SeruroPanelContacts::SeruroPanelContacts(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Contacts"))
{
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *results_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    /* Create the list control. */
    ContactList::Create(this);

    /* Add it to our sizer */
    ContactList::AddContactList(results_sizer);
    sizer->Add(results_sizer, 1, wxALL | wxEXPAND, 5);
    this->SetSizer(sizer);
    
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelContacts::OnContactStateChange, this, STATE_TYPE_CONTACT);
    
    this->Layout();

    ContactList::GenerateContactList();
    if (contact_list->GetItemCount() > 0) {
        //this->AlignList();
    }
}

void SeruroPanelContacts::OnContactStateChange(SeruroStateEvent &event)
{
    ContactList::OnContactStateChange(event);
    this->AlignList();
    
    event.Skip();
}

void SeruroPanelContacts::AlignList()
{
    wxListCtrl *lists[] = {ContactList::contact_list};
    MaximizeAndAlignLists(lists, 1, 1);
}