
#include "../frames/dialogs/AddAccountDialog.h"

#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

BEGIN_EVENT_TABLE(ServerPage, wxWizardPageSimple)
	EVT_CHECKBOX(SERURO_ADD_SERVER_PORT_ID, ServerPage::OnForm_OnCustomPort)
END_EVENT_TABLE()

/* This event handler function will be duplicated (defined) for each implmentor. */
void ServerPage::OnForm_OnCustomPort(wxCommandEvent &event)
{
    this->OnCustomPort();
}

ServerPage::ServerPage(SeruroSetup *parent) : SetupPage(parent), AddServerForm(this)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    /* In case this page is started from a button. */
    this->enable_next = true;
    
    wxString msg_text = wxT("Please enter the hostname of your Seruro Server:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const server_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");
    
    this->AddForm(server_form);
    
    vert_sizer->Add(server_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}
