
#include "AuthDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

DECLARE_APP(SeruroClient);

AuthDialog::AuthDialog(const wxString &server, const wxString &address, int selected) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Seruro Server Login")),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
    
	/* Show a textual message. */
	Text *msg = new Text(this, wxString(wxT(TEXT_ACCOUNT_LOGIN) + server), false);
	msg->Wrap(300);
    
	sizerTop->Add(msg, DIALOGS_SIZER_OPTIONS);
    
	wxSizer* const sizerInfo = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");
    
	/* Email address selection */
	sizerInfo->Add(new Text(this, "&Email Address:"));
    
	/* Create list of email addresses. */
	wxArrayString address_list;
	if (address.compare(wxEmptyString) == 0) {
		address_list = wxGetApp().config->GetAddressList(server);
		if (address_list.size() == 0) {
			/* There is something very wrong here! */
		}
	} else {
		address_list.Add(address);
	}
    
    
	address_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, address_list);
	address_control->SetSelection(selected);
    
	sizerInfo->Add(address_control, DIALOGS_BOXSIZER_OPTIONS);
    
	/* Password selection. */
	sizerInfo->Add(new Text(this, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
	sizerInfo->Add(password_control, DIALOGS_BOXSIZER_OPTIONS);
    
	/* Default buttons. */
	sizerTop->Add(sizerInfo, DIALOGS_BOXSIZER_SIZER_OPTIONS);
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

