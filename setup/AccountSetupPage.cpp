
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

AccountPage::AccountPage(SeruroSetup *parent) : 
  SetupPage(parent), AddAccountForm(this)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	this->next_button = _("&Login");

    wxString msg_text = wxT("Please enter the information for your account:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const account_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");
    
    this->AddForm(account_form);
    
    vert_sizer->Add(account_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}

/* Use the information in the form to login, update the UI, and allow 
 * the wizard to "move forward". */
bool AccountPage::GoForward() {
	return false;
}

