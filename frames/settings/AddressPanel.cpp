
#include "SettingsPanels.h"
#include "../../api/SeruroServerAPI.h"
//#include "../../api/SeruroRequest.h"

#include "../../wxJSON/wx/jsonval.h"

#include <wx/sizer.h>
#include <wx/button.h>

enum button_actions {
    BUTTON_UPDATE,
    BUTTON_REMOVE
};

BEGIN_EVENT_TABLE(SettingsPanel_Address, SettingsPanel)
    EVT_BUTTON(BUTTON_UPDATE, SettingsPanel_Address::OnUpdate)
    EVT_BUTTON(BUTTON_REMOVE, SettingsPanel_Address::OnDelete)

	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, SettingsPanel_Address::OnUpdateResponse)
END_EVENT_TABLE()

bool SettingsPanel_Address::Changed() { return false; }

void SettingsPanel_Address::Render() {}

SettingsPanel_Address::SettingsPanel_Address(SeruroPanelSettings *parent,
	const wxString &address, const wxString &server) :
    SettingsPanelView(parent), address(address), server_name(server)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	//wxButton *address_button = new wxButton(this, wxID_ANY, address);
	//wxButton *server_button = new wxButton(this, wxID_ANY, server);
	//vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	//vert_sizer->Add(server_button, 0, wxRIGHT, 5);
    
    Text *msg = new Text(this, wxString(wxT("View the details and status for your address: ") + address));
    vert_sizer->Add(msg, wxSizerFlags().Expand().Border(wxALL, 5));
    
    wxSizer *info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Address Information");
    
    /* Status information about address. */
    Text *server_name_info = new Text(this, wxString(wxT("Server Name: ") + server_name));
    Text *address_info = new Text(this, wxString(wxT("Address: ") + address));
    Text *p12_info = new Text(this, wxString(wxT("Identity last updated: Today")));
    Text *token_info = new Text(this, wxString(wxT("Token last updated: Today")));
    
    info_box->Add(server_name_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(address_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(p12_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    info_box->Add(token_info, wxSizerFlags().Expand().Border(wxBOTTOM));
    
    vert_sizer->Add(info_box, wxSizerFlags().Expand().Border(wxALL, 5));
    
    Text *update_warning = new Text(this, wxT("Note: updating the address identity will send a new decryption password."));
    vert_sizer->Add(update_warning, wxSizerFlags().Expand().Border(wxALL, 5));
    
    /* Control buttons. */
    wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *update_button = new wxButton(this, BUTTON_UPDATE, wxT("Update"));
    wxButton *remove_button = new wxButton(this, BUTTON_REMOVE, wxT("Remove"));
    
    buttons_sizer->Add(update_button, wxSizerFlags().Right().Expand());
    buttons_sizer->Add(remove_button, wxSizerFlags().Right().Expand());
    
    vert_sizer->Add(buttons_sizer, wxSizerFlags().Expand().Border(wxALL, 5));

	this->SetSizer(vert_sizer);
}

void SettingsPanel_Address::OnUpdateResponse(SeruroRequestEvent &event)
{
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());

	/* Todo: add erroring checking. */
	if (! api->InstallP12(event.GetResponse())) {
		/* Todo: report that something bad happened. */
	}
}

/* Todo: consider having the API call use a global callback function defined in API perhaps.
 * This prevents code duplication and allows maintainence of API handleing. 
 * This would also prevent custom call-back handling (such as alerts).
 */
void SettingsPanel_Address::OnUpdate(wxCommandEvent &event)
{
    wxJSONValue params; /* no params */
    
    SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
    
	params["server"] = api->GetServer(this->server_name);
	params["address"] = this->address;
    
	SeruroRequest *request = api->CreateRequest(SERURO_API_P12S, params, SERURO_API_CALLBACK_P12S);
	request->Run();
	/* Todo: Cannot delete the request because the thread still exists, who cleans up this memory? */
}

void SettingsPanel_Address::OnDelete(wxCommandEvent &event)
{
    
}


