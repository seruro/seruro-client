
#include "DecryptDialog.h"

#include "../../SeruroClient.h"

DECLARE_APP(SeruroClient);

DecryptDialog::DecryptDialog(const wxString &method) :
    wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Decrypt Certificates")),
        wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
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
	wxStaticText *msg = new wxStaticText(this, wxID_ANY,
                                         wxString(method_text + wxT(" ") + wxT(TEXT_DECRYPT_EXPLAINATION)));
	msg->Wrap(300);
	sizer_top->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5));
    
	wxSizer* const sizer_info = new wxStaticBoxSizer(wxVERTICAL, this, "&Certificates Password");
	/* Password selection. */
	sizer_info->Add(new wxStaticText(this, wxID_ANY, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY,
                                      wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                      wxTE_PASSWORD);
	sizer_info->Add(password_control, wxSizerFlags().Expand().Border(wxBOTTOM));
    
	/* Default buttons. */
	sizer_top->Add(sizer_info, wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT, 5));
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizer_top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Right().Border());
	SetSizerAndFit(sizer_top);
}

wxString DecryptDialog::GetValue()
{
	return password_control->GetValue();
}
