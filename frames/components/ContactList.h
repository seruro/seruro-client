
#ifndef H_ContactList
#define H_ContactList

#include <wx/listctrl.h>
#include <wx/imaglist.h>

#include "../../api/SeruroStateEvents.h"

enum contact_list_ids_t {
    CONTACT_LIST_ID
};

class ContactList
{
public:
    ContactList() {}
    ~ContactList() {}
    
    /* Lazy instance. */
    void Create(wxWindow *parent);
    
    /* Access the internal list, only to add to a sizer. */
    void AddContactList(wxSizer *sizer);
    
    void AddContact(wxString address, wxString server_uuid);
    void RemoveContact(wxString address, wxString server_uuid);
    
    void GenerateContactList();
    
    /* Might not be needed */
    bool SelectContact(long index);

    /* A contact is added or deleted. */
    void OnContactStateChange(SeruroStateEvent &event);
    /* A certificate changes. */
    void OnCertificateStateChange(SeruroStateEvent &event);
    /* An identity is added. */
    void OnIdentityStateChange(SeruroStateEvent &event);
    /* Make sure we property identify/call out identity-accounts. */
    void OnAccountStateChange(SeruroStateEvent &event);
    
protected:
    /* Components. */
    wxListCtrl *contact_list;
	wxImageList *list_images;
    wxWindow *parent;
    
    /* Selectors. */
    wxString selected_server_uuid;
    wxString selected_address;
    
    /* Prevent the first column from moving. */
    void OnContactColumnDrag(wxListEvent &event);
    void OnContactSelected(wxListEvent &event);
};

#endif
