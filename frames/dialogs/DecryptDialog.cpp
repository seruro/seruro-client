
#include "DecryptDialog.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"

DECLARE_APP(SeruroClient);

void DecryptForm::AddForms(wxSizer *sizer)
{
	wxFlexGridSizer *const grid_sizer = new wxFlexGridSizer(1, 2, 
		GRID_SIZER_WIDTH, GRID_SIZER_HEIGHT);
	grid_sizer->AddGrowableCol(1, 1);

 	grid_sizer->Add(new Text(parent, "&Decrypt Key:"));
	password_control = new wxTextCtrl(parent, wxID_ANY,
        wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_PASSWORD);
	grid_sizer->Add(password_control, DIALOGS_BOXSIZER_OPTIONS);

	sizer->Add(grid_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
}

DecryptDialog::DecryptDialog(const wxString &method) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Decrypt Certificates")),
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
    
	wxSizer* const sizer_info = new wxStaticBoxSizer(wxVERTICAL, this, 
		"&Certificates Password");
    
	/* Password selection. */
    this->AddForms(sizer_info);
    
	/* Default buttons. */
	sizer_top->Add(sizer_info, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizer_top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), 
		DIALOGS_BUTTONS_OPTIONS);
	SetSizerAndFit(sizer_top);
}

void DecryptForm::DisableForm()
{
#if defined(__WXOSX__) || defined(__WXMAC__)
    password_control->SetWindowStyle(password->GetWindowStyle() | wxTE_READONLY);
#else
    this->password_control->Disable();
#endif
}

void DecryptForm::EnableForm()
{
#if defined(__WXOSX__) || defined(__WXMAC__)
	password_control->SetWindowStyle(wxTE_PASSWORD);
#else
    password_control->Enable(true);
#endif
}

void DecryptForm::FocusForm()
{
	this->password_control->SetFocus();
}

wxString DecryptForm::GetValue()
{
	return password_control->GetValue();
}
