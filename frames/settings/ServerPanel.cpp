
#include "SettingsPanels.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>

SettingsPanel_Server::SettingsPanel_Server(SeruroPanelSettings *parent,
    const wxString &server) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	//wxButton *address_button = new wxButton(this, wxID_ANY, address);
	//wxButton *server_button = new wxButton(this, wxID_ANY, server);
	//vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	//vert_sizer->Add(server_button, 0, wxRIGHT, 5);
    
    wxStaticText *msg = new wxStaticText(this, wxID_ANY,
        wxString(wxT("View and change the configuration settings for: ") + server));
	msg->Wrap(300);
    
    vert_sizer->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5));
    
	this->SetSizer(vert_sizer);
}
