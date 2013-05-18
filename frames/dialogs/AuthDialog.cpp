
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
	/* Get available addresses: */
	//this->is_list = false;
    
	/* Create list of email addresses. */
	wxArrayString address_list;
	if (address.compare(wxEmptyString) == 0) {
		address_list = wxGetApp().config->GetAddressList(server);
		//this->is_list = address_list.size() > 1;
		if (address_list.size() == 0) {
			/* There is something very wrong here! */
		} //else if (! this->is_list) {
        //address_control = new wxTextCtrl(this, wxID_ANY);
        //address_list.Add(
		//} else {
        //address_list_control = new wxChoice(this, wxID_ANY,
        //	wxDefaultPosition, wxDefaultSize, address_list);
        //address_list_control->SetSelection(0);
		//}
	} else {
		//address_control = new wxTextCtrl(this, wxID_ANY);
		address_list.Add(address);
	}
    
    
	address_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, address_list);
	address_control->SetSelection(selected);
	//if (this->is_list) {
	//	sizerInfo->Add(address_list_control, wxSizerFlags().Expand().Border(wxBOTTOM));
	//} else {
	sizerInfo->Add(address_control, wxSizerFlags().Expand().Border(wxBOTTOM));
	//}
    
	/* Password selection. */
	sizerInfo->Add(new wxStaticText(this, wxID_ANY, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY,
                                      wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                      wxTE_PASSWORD);
	//password_control->SetDefaultStyle(wxTextAttr(*wxTE_PASSWORD));
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
    
	//if (this->is_list) {
	int selected = this->address_control->GetSelection();
	/* Save the selected index for a better user experience. */
	values["selected"] = selected;
	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetString(selected);
	//} else {
	//	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetValue();
	//}
	values[SERURO_API_AUTH_FIELD_PASSWORD] = this->password_control->GetValue();
    
	return values;
}

