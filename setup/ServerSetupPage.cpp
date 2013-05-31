
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

ServerPage::ServerPage(SeruroSetup *parent) : SetupPage(parent), AddServerForm(this)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    wxString msg_text = wxT("Please enter the information for your Seruro Server:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const server_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");
    
    this->AddForm(server_form);
    
    vert_sizer->Add(server_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}