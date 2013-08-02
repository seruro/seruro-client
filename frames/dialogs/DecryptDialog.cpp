
#include "DecryptDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

DECLARE_APP(SeruroClient);

void DecryptForm::AddForms(wxSizer *sizer)
{
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(1, 2, 
		GRID_SIZER_WIDTH, GRID_SIZER_HEIGHT);
	grid_sizer->AddGrowableCol(1, 1);

    
    identity_control = new wxTextCtrl(parent, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        /* Should be password by default? */
        wxTE_PASSWORD);
    encryption_control = new wxTextCtrl(parent, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
    
 	grid_sizer->Add(new Text(parent, "&Identity Unlock Code:"));
    grid_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
	grid_sizer->Add(identity_control, DIALOGS_BOXSIZER_OPTIONS);
    
    grid_sizer->Add(new Text(parent, "&Encryption Unlock Code:"));
    grid_sizer->SetItemMinSize((size_t) 2, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    grid_sizer->Add(encryption_control, DIALOGS_BOXSIZER_OPTIONS);

	sizer->Add(grid_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
}

DecryptDialog::DecryptDialog(const wxString &method) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Unlock Identity and Encryption")),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	DecryptForm(this)
{
	wxSizer* const sizer_top = new wxBoxSizer(wxVERTICAL);
    
	/* Todo: This should be switching on an enumeration. */
	wxString method_text;
	if (method.compare("sms") == 0) {
		method_text = wxT(TEXT_DECRYPT_METHOD_SMS);
	} else {
		method_text = wxT(TEXT_DECRYPT_METHOD_EMAIL);
	}
    
	/* Show a textual message. */
    wxString msg_text = method_text + _(" ") + _(TEXT_DECRYPT_EXPLAINATION);
	wxStaticText *msg = new Text(this, msg_text, false);
	msg->Wrap(300);
	sizer_top->Add(msg, DIALOGS_SIZER_OPTIONS);
    
	wxSizer* const sizer_info = new wxStaticBoxSizer(wxVERTICAL, this, "&Unlock Codes");
    
	/* Password selection. */
    this->AddForms(sizer_info);
    
	/* Default buttons. */
	sizer_top->Add(sizer_info, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizer_top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
	SetSizerAndFit(sizer_top);
}

void DecryptForm::DisableForm()
{
//#if defined(__WXOSX__) || defined(__WXMAC__)
//    identity_control->SetWindowStyle(identity_control->GetWindowStyle() | wxTE_READONLY);
//#else
    this->identity_control->Disable();
    this->encryption_control->Disable();
//#endif
}

void DecryptForm::EnableForm()
{
//#if defined(__WXOSX__) || defined(__WXMAC__)
//	identity_control->SetWindowStyle(wxTE_PASSWORD);
//#else
    identity_control->Enable(true);
    encryption_control->Enable(true);
//#endif
}

void DecryptForm::FocusForm()
{
	this->identity_control->SetFocus();
    this->encryption_control->SetFocus();
}

wxJSONValue DecryptForm::GetValues()
{
    wxJSONValue values;
    
    values["authentication"] = this->identity_control->GetValue();
    values["encipherment"] = this->identity_control->GetValue();
    
    return values;
}

wxString DecryptForm::GetValue()
{
	return identity_control->GetValue();
}

