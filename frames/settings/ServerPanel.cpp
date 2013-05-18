
#include "SettingsPanels.h"

#include <wx/sizer.h>
#include <wx/button.h>

SettingsPanel_Server::SettingsPanel_Server(SeruroPanelSettings *parent,
    const wxString &settings) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	//wxButton *address_button = new wxButton(this, wxID_ANY, address);
	//wxButton *server_button = new wxButton(this, wxID_ANY, server);
	//vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	//vert_sizer->Add(server_button, 0, wxRIGHT, 5);
    
    
    
	this->SetSizer(vert_sizer);
}
