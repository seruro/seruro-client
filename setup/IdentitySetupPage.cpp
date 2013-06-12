
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

IdentityPage::IdentityPage(SeruroSetup *parent) 
	: SetupPage(parent) //, AddAccountForm(this), login_success(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	//this->next_button = _("&Login");

    wxString msg_text = _(TEXT_INSTALL_IDENTITY);
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const identity_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Install Identity");
    
    //this->AddForm(account_form);
    
	install_identity = new wxCheckBox(this, wxID_ANY, _("Install the identity for this address."));
	identity_form->Add(install_identity, DIALOGS_BOXSIZER_OPTIONS);

    vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}

