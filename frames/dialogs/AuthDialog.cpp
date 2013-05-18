
#include "AuthDialog.h"

#include "../../SeruroClient.h"

DECLARE_APP(SeruroClient);

AuthDialog::AuthDialog(const wxString &server, const wxString &address, int selected) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Seruro Server Login")),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);
    
	/* Show a textual message. */
	wxStaticText *msg = new wxStaticText(this, wxID_ANY,
        wxString(wxT(TEXT_ACCOUNT_LOGIN) + server));
	msg->Wrap(300);
    
	sizerTop->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5));
    
	wxSizer* const sizerInfo = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");
    
	/* Email address selection */
	sizerInfo->Add(new wxStaticText(this, wxID_ANY, "&Email Address:"));
    
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
    
	sizerInfo->Add(address_control, wxSizerFlags().Expand().Border(wxBOTTOM));
    
	/* Password selection. */
	sizerInfo->Add(new wxStaticText(this, wxID_ANY, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
	sizerInfo->Add(password_control, wxSizerFlags().Expand().Border(wxBOTTOM));
    
	/* Default buttons. */
	sizerTop->Add(sizerInfo, wxSizerFlags().Expand().Border(wxTOP | wxRIGHT | wxLEFT, 5));
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizerTop->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Right().Border());
	SetSizerAndFit(sizerTop);
}

wxJSONValue AuthDialog::GetValues()
{
	wxJSONValue values;
    
	int selected = this->address_control->GetSelection();
    
	/* Save the selected index for a better user experience. */
	values["selected"] = selected;
	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetString(selected);
	values[SERURO_API_AUTH_FIELD_PASSWORD] = this->password_control->GetValue();
    
	return values;
}

