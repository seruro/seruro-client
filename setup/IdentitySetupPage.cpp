
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"
#include <wx/button.h>

enum {
    BUTTON_DOWNLOAD_IDENTITY
};

BEGIN_EVENT_TABLE(IdentityPage, SetupPage)
	//EVT_CHECKBOX(SERURO_INSTALL_IDENTITY_ID, IdentityPage::OnToggleInstall)
    EVT_BUTTON(BUTTON_DOWNLOAD_IDENTITY, IdentityPage::OnDownloadIdentity)
    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, IdentityPage::OnP12sResponse)
END_EVENT_TABLE()

void IdentityPage::OnP12sResponse(SeruroRequestEvent &event)
{
    
}

void IdentityPage::OnDownloadIdentity(wxCommandEvent &event)
{
    
}

IdentityPage::IdentityPage(SeruroSetup *parent, bool force_install)
	: SetupPage(parent) //, AddAccountForm(this), login_success(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	/* The identity page does not allow the user to go backward. */
	this->enable_prev = false;
    this->enable_next = false;
	/* The user may not proceeded unless the P12 is download (may happen automatically). */
	this->next_button = _("&Install");

    /* Textual notice about the use of an identity. */
    wxString msg_text = _(TEXT_INSTALL_IDENTITY);
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    /* Download form if the P12 is not retreived automatically. */
    wxSizer *const identity_form = new wxStaticBoxSizer(wxHORIZONTAL, this, "&Download Identity");
    identity_form->Add(new Text(this, _("I trust, and send email from, this machine.")), DIALOGS_BOXSIZER_OPTIONS);
    identity_form->AddStretchSpacer();
    identity_button = new wxButton(this, BUTTON_DOWNLOAD_IDENTITY, _("Download"));
    identity_form->Add(identity_button, DIALOGS_BOXSIZER_OPTIONS);
    


    vert_sizer->Add(identity_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}

void IdentityPage::EnablePage()
{
    if (! this->identity_downloaded) {
        identity_button->Enable();
        identity_button->SetLabelText("&Download");
    }
    
}

