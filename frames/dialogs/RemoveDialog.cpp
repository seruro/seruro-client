
#include "RemoveDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../api/SeruroStateEvents.h"
#include "../../api/SeruroServerAPI.h"

#include <wx/grid.h>
#include <wx/log.h>

DECLARE_APP(SeruroClient);

RemoveDialog::RemoveDialog(const wxString &server_name, const wxString &address) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Remove"))),
	server_name(server_name), address(address)
{
	/* To avoid many-string compares, initially set if we are removing a server. */
	remove_server = (address.compare(wxEmptyString) == 0);
    
	wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);

	Text *msg = new Text(this, (remove_server) 
		? wxString::Format(_(TEXT_REMOVE_SERVER), server_name, server_name, server_name) 
		: wxString::Format(_(TEXT_REMOVE_ADDRESS), address, server_name));
	msg->Wrap(300);
	vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);

	if (remove_server) {
		wxSizer *const server_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Associated Data");
		/* Add checkboxes for server-wide data. */
		this->remove_ca = new wxCheckBox(this, wxID_ANY, _("Remove the certificate authority."));
		this->remove_certs = new wxCheckBox(this, wxID_ANY, _("Remove downloaded certificates."));
        
		server_box->Add(remove_ca, DIALOGS_BOXSIZER_OPTIONS);
		server_box->Add(remove_certs, DIALOGS_BOXSIZER_OPTIONS);
		vert_sizer->Add(server_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	}

	wxSizer *const address_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Associated Identity(s)");

	/* Either allow all identities to be removed, or only the explicitly defined. */
	wxArrayString addresses;
	if (remove_server) {
		addresses = wxGetApp().config->GetIdentityList(server_name);
	} else {
		addresses.Add(address);
	}

	//remove_identities = (wxCheckBox* []) malloc(sizeof(wxCheckBox*) * addresses.size());
	//remove_identities = new wxCheckBox*[addresses.size()];
	this->remove_identities = (wxCheckBox**) malloc(sizeof(wxCheckBox*) * addresses.size());
    identity_count = addresses.size();
	for (size_t i = 0; i < identity_count; i++) {
		this->remove_identities[i] = new wxCheckBox(this, wxID_ANY,
			wxString::Format(_T("Remove identity: '%s'."), addresses[i]));
		address_box->Add(this->remove_identities[i], DIALOGS_BOXSIZER_OPTIONS);
	}
	vert_sizer->Add(address_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

void RemoveDialog::DoRemove() 
{
    wxArrayString accounts, certificates;
    wxCheckBox *identity;
    SeruroServerAPI *api;
    
    accounts = wxGetApp().config->GetIdentityList(this->server_name);
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
            wxLogMessage(_("RemoveDialog> (DoRemove) removing identity for (%s) (%s)."), server_name, accounts[i]);
            /* Should this keep the decryption certificates? */
            api->UninstallIdentity(this->server_name, accounts[i]);
        }
	}
    delete api;

	/* Finally remove the data from the config (and the app). */
	if (this->remove_server) {
        this->RemoveServer();
	} else {
        this->RemoveAddress();
	}
}

void RemoveDialog::RemoveServer()
{
    wxArrayString certificates;
    SeruroServerAPI *api;
    
    wxGetApp().config->RemoveServer(this->server_name);
    
    SeruroStateEvent event(STATE_TYPE_SERVER, STATE_ACTION_REMOVE);
    event.SetServerUUID(this->server_name);
    this->ProcessWindowEvent(event);
    
    api = new SeruroServerAPI(this);
    if (this->remove_ca->IsChecked()) {
        /* Only remove the CA cert if the user requested (this may imply removing the contact certificates?). */
        wxLogMessage(_("RemoveDialog> (DoRemove) removing CA cert (%s)."), server_name);
        api->UninstallCA(this->server_name);
    }
    
    if (this->remove_certs->IsChecked()) {
        /* Remove contact certificates if the user requested. */
        certificates = wxGetApp().config->GetCertificatesList(this->server_name);
        for (size_t i = 0; i < certificates.size(); i++) {
            wxLogMessage(_("RemoveDialog> (DoRemove) removing cers for (%s) (%s)."), server_name, certificates[i]);
            api->UninstallCertificates(this->server_name, certificates[i]);
        }
    }
    delete api;
}

void RemoveDialog::RemoveAddress()
{
    wxGetApp().config->RemoveAddress(this->server_name, this->address);
    
    SeruroStateEvent event(STATE_TYPE_ACCOUNT, STATE_ACTION_REMOVE);
    event.SetServerUUID(this->server_name);
    event.SetAccount(this->address);
    this->ProcessWindowEvent(event);
}

