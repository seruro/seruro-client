
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

BEGIN_EVENT_TABLE(ServerPage, wxWizardPageSimple)
	EVT_CHECKBOX(SERURO_ADD_SERVER_PORT_ID, ServerPage::OnCustomPort)
END_EVENT_TABLE()

/* This event handler function will be duplicated (defined) for each implmentor. */
void ServerPage::OnCustomPort(wxCommandEvent &event)
{
    wxLogMessage(wxT("checkbox clicked."));
	this->server_port->Enable(this->checkbox->IsChecked());
	if (! this->checkbox->IsChecked()) {
		/* If the checkbox becomes un-checked, reset the port value. */
		this->server_port->SetValue(SERURO_DEFAULT_PORT);
	}
}

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

