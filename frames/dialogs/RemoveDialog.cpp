
#include "RemoveDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../api/SeruroStateEvents.h"

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
		addresses = wxGetApp().config->GetAddressList(server_name);
	} else {
		addresses.Add(address);
	}

	//remove_identities = (wxCheckBox* []) malloc(sizeof(wxCheckBox*) * addresses.size());
	remove_identities = new wxCheckBox*[addresses.size()];
	//remove_identities = (wxCheckBox[]) malloc(sizeof(wxCheckBox*) * addresses.size());
	for (size_t i = 0; i < addresses.size(); i++) {
		remove_identities[i] = new wxCheckBox(this, wxID_ANY, 
			wxString::Format(_T("Remove identity: '%s'."), addresses[i]));
		address_box->Add(remove_identities[i], DIALOGS_BOXSIZER_OPTIONS);
	}
	vert_sizer->Add(address_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	vert_sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	this->SetSizerAndFit(vert_sizer);
}

void RemoveDialog::DoRemove() 
{
	size_t identities_size = wxGetApp().config->GetAddressList(server_name).size();

	/* Todo: evaluate TOCTOU logic errors. */
	/* Very important, this list must be iterated the same way it is created. */
	for (size_t i = 0; i < identities_size; i++) {
		/* Todo: this next line throws a memory access violation. */
		//if (this->remove_identities[i]->IsChecked()) {

		//}
	}

	/* Finally remove the data from the config (and the app). */
	if (this->remove_server) {
		wxGetApp().config->RemoveServer(this->server_name);

		SeruroStateEvent event(STATE_TYPE_SERVER, STATE_ACTION_REMOVE);
		event.SetServerName(this->server_name);
		//wxGetApp().AddEvent(event);
		this->ProcessWindowEvent(event);
	} else {
		wxGetApp().config->RemoveAddress(this->server_name, this->address);

		SeruroStateEvent event(STATE_TYPE_ACCOUNT, STATE_ACTION_REMOVE);
		event.SetServerName(this->server_name);
		event.SetValue(_("address"), this->address);
		//wxGetApp().AddEvent(event);
		this->ProcessWindowEvent(event);
	}

	
}
