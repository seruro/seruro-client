
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

#define SERURO_INSTALL_IDENTITY_ID wxID_HIGHEST

BEGIN_EVENT_TABLE(IdentityPage, SetupPage)
	EVT_CHECKBOX(SERURO_INSTALL_IDENTITY_ID, IdentityPage::OnToggleInstall)
END_EVENT_TABLE()

void IdentityPage::OnToggleInstall(wxCommandEvent &event)
{
	//this->wizard->EnableBack(this->install_identity->IsChecked());
	if (this->install_identity->IsChecked()) {
		//this->wizard->EnableBack(false);
		this->wizard->SetButtonText(wxEmptyString, _("&Install"));
	} else {
		//this->wizard->EnableBack(true);
		this->wizard->SetButtonText(wxEmptyString, this->next_button);
	}
}

IdentityPage::IdentityPage(SeruroSetup *parent) 
	: SetupPage(parent) //, AddAccountForm(this), login_success(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	//this->next_button = _("&Login");
	/* The identity page does not allow the user to go backward. */
	//this->wizard->EnableBack(false);
	this->enable_back = false;
	/* Unless the user installs this identity, this is the last page. */
	this->next_button = _("&Finish");

    wxString msg_text = _(TEXT_INSTALL_IDENTITY);
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const identity_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Install Identity");
    
    //this->AddForm(account_form);
    
	install_identity = new wxCheckBox(this, SERURO_INSTALL_IDENTITY_ID, 
		_("Install the identity for this address."));
	identity_form->Add(install_identity, DIALOGS_BOXSIZER_OPTIONS);

    vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}

