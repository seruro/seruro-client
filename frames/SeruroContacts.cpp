
#include "SeruroContacts.h"
#include "UIDefs.h"

#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../api/ServerMonitor.h"

enum {
    RECHECK_BUTTON_ID
};

BEGIN_EVENT_TABLE(SeruroPanelContacts, SeruroPanel)
    EVT_BUTTON(RECHECK_BUTTON_ID,  SeruroPanelContacts::OnRecheckContacts)
END_EVENT_TABLE()

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
    
    /* A button to manual check for next contacts if the check is happening automaticaly. */
    this->recheck_button = new wxButton(this, RECHECK_BUTTON_ID, _("Check for new Contacts"));
    if (theSeruroConfig::Get().GetOption("auto_download") != "true") {
        this->recheck_button->Hide();
    }
    
    wxBoxSizer *button_sizer = new wxBoxSizer(wxHORIZONTAL);
    //button_sizer->AddStretchSpacer();
    button_sizer->Add(recheck_button, 1, wxBOTTOM|wxLEFT|wxEXPAND, 10);
    sizer->Add(button_sizer);
    
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelContacts::OnContactStateChange, this, STATE_TYPE_CONTACT);
	wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelContacts::OnServerStateChange, this, STATE_TYPE_SERVER);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelContacts::OnIdentityStateChange, this, STATE_TYPE_IDENTITY);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelContacts::OnOptionChange, this, STATE_TYPE_OPTION);
    
    this->SetSizer(sizer);
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

void SeruroPanelContacts::OnServerStateChange(SeruroStateEvent &event)
{
    /* This is a little brutish. */
	if (event.GetAction() == STATE_ACTION_REMOVE) {
		ContactList::GenerateContactList();
		this->AlignList();
	}
    
    event.Skip();
}

void SeruroPanelContacts::OnIdentityStateChange(SeruroStateEvent &event)
{
    event.Skip();
    
    ContactList::OnIdentityStateChange(event);
    this->AlignList();
}

void SeruroPanelContacts::OnOptionChange(SeruroStateEvent &event)
{
    event.Skip();
    
    if (event.GetValue("option_name") != "auto_download") {
        return;
    }
    
    /* Perform the appropriate UI action. */
    if (event.GetValue("option_value") == "true") {
        this->recheck_button->Show();
    } else {
        this->recheck_button->Hide();
    }
}

void SeruroPanelContacts::OnRecheckContacts(wxCommandEvent &event)
{
    /* Restart the monitor (server) using a transient call to the server monitor. */
    ServerMonitor *transient_monitor = new ServerMonitor(true);
    transient_monitor->Monitor();
    delete transient_monitor;
}

void SeruroPanelContacts::AlignList()
{
    wxListCtrl *lists[] = {ContactList::contact_list};
    MaximizeAndAlignLists(lists, 1, 1);
}
