
#include "SettingsPanels.h"
#include "../../SeruroClient.h"
#include "../../api/SeruroServerAPI.h"
//#include "../../SeruroConfig.h"

#include "../../wxJSON/wx/jsonval.h"
#include "../../wxJSON/wx/jsonreader.h"

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

	EVT_COMMAND(SERURO_API_CALLBACK_GET_CA, SERURO_API_RESULT, SettingsPanel_Server::OnUpdateResult)
END_EVENT_TABLE()

bool SettingsPanel_Server::Changed() { return false; }

void SettingsPanel_Server::Render() {}

SettingsPanel_Server::SettingsPanel_Server(SeruroPanelSettings *parent,
    const wxString &server) : SettingsPanelView(parent), server_name(server)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	wxJSONValue server_info = wxGetApp().config->GetServer(server);
    
	/* Show a small bit of text with a description and instruction. */
    wxStaticText *msg = new Text(this,
        wxString(wxT("View and change the configuration settings for: ")) + server);
    
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS); 

	/* Create an info box to display and edit server settings. */
	wxSizer* const info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");

	/* The server information may change with an edit, so store text controls (??). */
	Text *server_name_info = new Text(this, wxString(wxT("Name: ") + server));
	Text *server_host_info = new Text(this, wxString(wxT("Host: ") + server_info["host"].AsString()));
	Text *server_port_info = new Text(this, wxString(wxT("Port: ") + server_info["port"].AsString()));

	info_box->Add(server_name_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
	info_box->Add(server_host_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
	info_box->Add(server_port_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);

	vert_sizer->Add(info_box, SETTINGS_PANEL_SIZER_OPTIONS);

	/* Edit server information button (use Horz sizer with spacer) */
	wxBoxSizer *edit_button_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *edit_info_button = new wxButton(this, BUTTON_EDIT_INFO, wxT("Edit Info"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//edit_button_sizer->AddStretchSpacer();
	edit_button_sizer->Add(edit_info_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(edit_button_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	/* Next: add status box. */
	wxSizer * const status_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Status");

	/* Todo: make updated/check reflect actual times. */
	Text *server_updated_status = new Text(this, wxString(wxT("Last updated: Today")));
	Text *server_checked_status = new Text(this, wxString(wxT("Last checked: Today")));
	
	status_box->Add(server_updated_status, SETTINGS_PANEL_BOXSIZER_OPTIONS);
	status_box->Add(server_checked_status, SETTINGS_PANEL_BOXSIZER_OPTIONS);

	vert_sizer->Add(status_box, SETTINGS_PANEL_SIZER_OPTIONS);

	/* Update/Delete server buttons with sizer and spacer. */
	wxBoxSizer *status_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *update_button = new wxButton(this, BUTTON_UPDATE, wxT("Update"));
	wxButton *delete_button = new wxButton(this, BUTTON_DELETE, wxT("Delete"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//status_buttons_sizer->AddStretchSpacer();
	status_buttons_sizer->Add(update_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	status_buttons_sizer->Add(delete_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(status_buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	this->SetSizer(vert_sizer);
}

void SettingsPanel_Server::OnUpdate(wxCommandEvent &event)
{
	wxJSONValue params; /* no params */
	//wxString server_name = wxT("Open Seruro");
    
    SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
    
    params["server"] = api->GetServer(this->server_name);
    
	SeruroRequest *request = api->CreateRequest(SERURO_API_GET_CA, params, SERURO_API_CALLBACK_GET_CA);
	request->Run();
}

void SettingsPanel_Server::OnUpdateResult(wxCommandEvent &event)
{
	wxJSONReader reader;
	wxJSONValue response;
	wxString responseString = event.GetString();
	
	reader.Parse(responseString, &response);

	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	api->InstallCA(response);
}

void SettingsPanel_Server::OnEdit(wxCommandEvent &event)
{

}

void SettingsPanel_Server::OnDelete(wxCommandEvent &event)
{

}

