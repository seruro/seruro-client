
#include "AuthDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../SeruroConfig.h"

DECLARE_APP(SeruroClient);

AuthDialog::AuthDialog(const wxString &server_uuid, const wxString &address, int selected) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(_("Seruro Server Login")),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
    
	wxString server_name = theSeruroConfig::Get().GetServerName(server_uuid);

	/* Show a textual message. */
	Text *msg = new Text(this, wxString(_(TEXT_ACCOUNT_LOGIN) + server_name), false);
	msg->Wrap(300);
    
	sizerTop->Add(msg, DIALOGS_SIZER_OPTIONS);
    
	wxSizer* const sizerInfo = new wxStaticBoxSizer(wxVERTICAL, this, "&Seruro Account Information");
    
	/* Email address selection */
	sizerInfo->Add(new Text(this, "&Email Address:"));
    
	/* Create list of email addresses. */
	wxArrayString address_list;
	if (address.compare(wxEmptyString) == 0) {
		address_list = theSeruroConfig::Get().GetAddressList(server_uuid);
		if (address_list.size() == 0) {
			/* There is something very wrong here! The client has no addresses. */
		}
	} else {
		/* The auth event is bound to a single address. */
		address_list.Add(address);
	}
    
	address_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, address_list);
	address_control->SetSelection((selected < 0) ? 0 : selected);
    
	sizerInfo->Add(address_control, DIALOGS_BOXSIZER_OPTIONS);
    
	/* Password selection. */
	sizerInfo->Add(new Text(this, "&Seruro Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
	sizerInfo->Add(password_control, DIALOGS_BOXSIZER_OPTIONS);
    
	/* Default buttons. */
	sizerTop->Add(sizerInfo, DIALOGS_SIZER_OPTIONS.Border(wxBOTTOM | wxLEFT | wxRIGHT, 15));
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizerTop->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	SetSizerAndFit(sizerTop);
}

wxJSONValue AuthDialog::GetValues()
{
	wxJSONValue values;
    
	int selected = this->address_control->GetSelection();
    
	/* Save the selected index for a better user experience. */
	values["selected_address"] = selected;
	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetString(selected);
	values[SERURO_API_AUTH_FIELD_PASSWORD] = this->password_control->GetValue();
    
	return values;
}

