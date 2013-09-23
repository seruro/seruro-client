
#include "RemoveDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../api/SeruroStateEvents.h"
#include "../../api/SeruroServerAPI.h"

#include <wx/grid.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

RemoveDialog::RemoveDialog(const wxString &server_uuid, const wxString &address) :
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Remove"))),
	server_uuid(server_uuid), address(address)
{
    wxArrayString addresses;
    wxString server_name;
    wxSizer *address_box;
    
	/* To avoid many-string compares, initially set if we are removing a server. */
    server_name = theSeruroConfig::Get().GetServerName(server_uuid);
	remove_server = (address.compare(wxEmptyString) == 0);
    
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

    wxSizer *message_sizer = new wxBoxSizer(wxHORIZONTAL);
	Text *msg = new Text(this, (remove_server) 
		? wxString::Format(_(TEXT_REMOVE_SERVER), server_name, server_name, server_name) 
		: wxString::Format(_(TEXT_REMOVE_ADDRESS), address, server_name), false);
	msg->Wrap(400);
	message_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    vert_sizer->Add(message_sizer, DIALOGS_SIZER_OPTIONS);

	if (remove_server) {
		wxSizer *const server_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Associated Data");
		/* Add checkboxes for server-wide data. */
		this->remove_ca = new wxCheckBox(this, wxID_ANY, _("Remove the certificate authority."));
		this->remove_certs = new wxCheckBox(this, wxID_ANY, _("Remove downloaded certificates."));
        
		server_box->Add(remove_ca, DIALOGS_BOXSIZER_OPTIONS);
		server_box->Add(remove_certs, DIALOGS_BOXSIZER_OPTIONS);
		vert_sizer->Add(server_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	}
    
    /* Either allow all identities to be removed, or only the explicitly defined. */
	if (remove_server) {
		addresses = theSeruroConfig::Get().GetIdentityList(server_uuid);
	} else {
		addresses.Add(address);
	}

    identity_count = addresses.size();
    if (identity_count > 0) {
        /* Do not show the sizer header if the address list is empty. */
        address_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Associated Identity(s)");

        this->remove_identities = (wxCheckBox**) malloc(sizeof(wxCheckBox*) * identity_count);
        for (size_t i = 0; i < identity_count; i++) {
            this->remove_identities[i] = new wxCheckBox(this, wxID_ANY,
                wxString::Format(_T("Remove identity: '%s'."), addresses[i]));
            address_box->Add(this->remove_identities[i], DIALOGS_BOXSIZER_OPTIONS);
        }
        vert_sizer->Add(address_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    }
        
	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

void RemoveDialog::DoRemove() 
{
	if (this->remove_server) {
        this->RemoveServer();
	} else {
        this->RemoveAddress();
	}
}

void RemoveDialog::RemoveServer()
{
    wxArrayString accounts, certificates;
    wxCheckBox *identity;
    SeruroServerAPI *api;

    accounts = theSeruroConfig::Get().GetIdentityList(this->server_uuid);
    if (accounts.size() != this->identity_count) {
        /* If the number of identities has changed, then at least we KNOW something is wrong. */
        wxLogMessage(_("RemoveDialog> (DoRemove) the number of identities has changed."));
        return;
    }
    
    api = new SeruroServerAPI(this);
    /* Very important, this list must be iterated the same way it is created. */
	for (size_t i = 0; i < this->identity_count; i++) {
        identity = this->remove_identities[i];
        
        if (identity != 0 && identity->IsChecked()) {
            wxLogMessage(_("RemoveDialog> (DoRemove) removing identity for (%s) (%s)."), server_uuid, accounts[i]);
            /* Should this keep the decryption certificates? */
            api->UninstallIdentity(this->server_uuid, accounts[i]);
        }
	}
    
    if (this->remove_ca->IsChecked()) {
        /* Only remove the CA cert if the user requested (this may imply removing the contact certificates?). */
        wxLogMessage(_("RemoveDialog> (DoRemove) removing CA cert (%s)."), server_uuid);
        api->UninstallCA(this->server_uuid);
    }
    
    if (this->remove_certs->IsChecked()) {
        /* Remove contact certificates if the user requested. */
        certificates = theSeruroConfig::Get().GetContactsList(this->server_uuid);
        for (size_t i = 0; i < certificates.size(); i++) {
            wxLogMessage(_("RemoveDialog> (DoRemove) removing cers for (%s) (%s)."), server_uuid, certificates[i]);
            api->UninstallCertificates(this->server_uuid, certificates[i]);
        }
    }
    delete api;
    
    /* Remove from the config is the last action performed (however, the event occurs aftwared). */
    theSeruroConfig::Get().RemoveServer(this->server_uuid);
    
    SeruroStateEvent event(STATE_TYPE_SERVER, STATE_ACTION_REMOVE);
    event.SetServerUUID(this->server_uuid);
    this->ProcessWindowEvent(event);
}

void RemoveDialog::RemoveAddress()
{
    wxCheckBox *identity;
    SeruroServerAPI *api;
    
    api = new SeruroServerAPI(this);
    
    identity = this->remove_identities[0];
    if (identity != 0 && identity->IsChecked()) {
        wxLogMessage(_("RemoveDialog> (DoRemove) removing identity for (%s) (%s)."), server_uuid, this->address);
        /* Should this keep the decryption certificates? */
        api->UninstallIdentity(this->server_uuid, this->address);
    }
    
    delete api;
    
    theSeruroConfig::Get().RemoveAddress(this->server_uuid, this->address);
    
    SeruroStateEvent event(STATE_TYPE_ACCOUNT, STATE_ACTION_REMOVE);
    event.SetServerUUID(this->server_uuid);
    event.SetAccount(this->address);
    this->ProcessWindowEvent(event);
}

