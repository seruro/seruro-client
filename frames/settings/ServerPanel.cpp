
#include "SettingsPanels.h"
#include "../../SeruroClient.h"
//#include "../../SeruroConfig.h"

#include "../../wxJSON/wx/jsonval.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>

DECLARE_APP(SeruroClient);

SettingsPanel_Server::SettingsPanel_Server(SeruroPanelSettings *parent,
    const wxString &server) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	//wxButton *address_button = new wxButton(this, wxID_ANY, address);
	//wxButton *server_button = new wxButton(this, wxID_ANY, server);
	//vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	//vert_sizer->Add(server_button, 0, wxRIGHT, 5);

	wxJSONValue server_info = wxGetApp().config->GetServer(server);
    
	/* Show a small bit of text with a description and instruction. */
    wxStaticText *msg = new Text(this,
        wxString(wxT("View and change the configuration settings for: ")) + server);
	//msg->Wrap(300);

	/* Testing wrapping. */
	//wxBoxSizer *horz_sizer = new wxBoxSizer(wxHORIZONTAL);
	//horz_sizer->Add(msg, 0, wxEXPAND);
    
    vert_sizer->Add(msg, 0, wxEXPAND); //wxSizerFlags().Expand().Border(wxALL, 5)
    
	/* Create an info box to display and edit server settings. */
	wxSizer* const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");
	/* The server information may change with an edit, so store text controls. */
	//wxBoxSizer *server_name_sizer = new wxBoxSizer(wxHORIZONTAL);
	//server_name_sizer 
	wxStaticText *server_name_info = new wxStaticText(this, wxID_ANY,
		wxString(wxT("Name: ") + server));
	wxStaticText *server_host_info = new wxStaticText(this, wxID_ANY,
		wxString(wxT("Host: ") + server_info["host"].AsString()));
	wxStaticText *server_port_info = new wxStaticText(this, wxID_ANY,
		wxString(wxT("Port: ") + server_info["port"].AsString()));

	info_box->Add(server_name_info, wxSizerFlags().Expand().Border(wxBOTTOM));
	info_box->Add(server_host_info, wxSizerFlags().Expand().Border(wxBOTTOM));
	info_box->Add(server_port_info, wxSizerFlags().Expand().Border(wxBOTTOM));

	vert_sizer->Add(info_box, wxSizerFlags().Expand().Border(wxALL, 5));

	wxSizer * const status_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Status");



	this->SetSizer(vert_sizer);
}
