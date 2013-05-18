
#include "SettingsPanels.h"
#include "../../SeruroClient.h"
//#include "../../SeruroConfig.h"

#include "../../wxJSON/wx/jsonval.h"

#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>

enum button_actions
{
	BUTTON_EDIT_INFO,
	BUTTON_UPDATE,
	BUTTON_DELETE
};

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SettingsPanel_Server, SettingsPanel)
	EVT_BUTTON(BUTTON_UPDATE, SettingsPanel_Server::OnUpdate)
	EVT_BUTTON(BUTTON_EDIT_INFO, SettingsPanel_Server::OnEdit)
	EVT_BUTTON(BUTTON_DELETE, SettingsPanel_Server::OnDelete)
END_EVENT_TABLE()

SettingsPanel_Server::SettingsPanel_Server(SeruroPanelSettings *parent,
    const wxString &server) : SettingsPanelView(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	wxJSONValue server_info = wxGetApp().config->GetServer(server);
    
	/* Show a small bit of text with a description and instruction. */
    wxStaticText *msg = new Text(this,
        wxString(wxT("View and change the configuration settings for: ")) + server);
    
    vert_sizer->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5)); 

	/* Create an info box to display and edit server settings. */
	wxSizer* const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");

	/* The server information may change with an edit, so store text controls (??). */
	Text *server_name_info = new Text(this, wxString(wxT("Name: ") + server));
	Text *server_host_info = new Text(this, wxString(wxT("Host: ") + server_info["host"].AsString()));
	Text *server_port_info = new Text(this, wxString(wxT("Port: ") + server_info["port"].AsString()));

	info_box->Add(server_name_info, wxSizerFlags().Expand().Border(wxBOTTOM));
	info_box->Add(server_host_info, wxSizerFlags().Expand().Border(wxBOTTOM));
	info_box->Add(server_port_info, wxSizerFlags().Expand().Border(wxBOTTOM));

	vert_sizer->Add(info_box, wxSizerFlags().Expand().Border(wxALL, 5));

	/* Edit server information button (use Horz sizer with spacer) */
	wxBoxSizer *edit_button_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *edit_info_button = new wxButton(this, BUTTON_EDIT_INFO, wxT("Edit Info"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//edit_button_sizer->AddStretchSpacer();
	edit_button_sizer->Add(edit_info_button, wxSizerFlags().Right().Expand());
	vert_sizer->Add(edit_button_sizer, wxSizerFlags().Expand().Border(wxALL, 5));

	/* Next: add status box. */
	wxSizer * const status_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Status");

	/* Todo: make updated/check reflect actual times. */
	Text *server_updated_status = new Text(this, wxString(wxT("Last updated: Today")));
	Text *server_checked_status = new Text(this, wxString(wxT("Last checked: Today")));
	
	status_box->Add(server_updated_status, wxSizerFlags().Expand().Border(wxBOTTOM));
	status_box->Add(server_checked_status, wxSizerFlags().Expand().Border(wxBOTTOM));

	vert_sizer->Add(status_box, wxSizerFlags().Expand().Border(wxALL, 5));

	/* Update/Delete server buttons with sizer and spacer. */
	wxBoxSizer *status_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *update_button = new wxButton(this, BUTTON_UPDATE, wxT("Update"));
	wxButton *delete_button = new wxButton(this, BUTTON_DELETE, wxT("Delete"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//status_buttons_sizer->AddStretchSpacer();
	status_buttons_sizer->Add(update_button, wxSizerFlags().Right().Expand());
	status_buttons_sizer->Add(delete_button, wxSizerFlags().Right().Expand());
	vert_sizer->Add(status_buttons_sizer, wxSizerFlags().Expand().Border(wxALL, 5));

	this->SetSizer(vert_sizer);
}

void SettingsPanel_Server::OnUpdate(wxCommandEvent &event)
{

}

void SettingsPanel_Server::OnEdit(wxCommandEvent &event)
{

}

void SettingsPanel_Server::OnDelete(wxCommandEvent &event)
{

}

