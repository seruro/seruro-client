
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"
#include "../../api/SeruroServerAPI.h"
#include "../../SeruroClient.h"
//#include "../../api/SeruroRequest.h"
#include "../dialogs/RemoveDialog.h"

#include "../../wxJSON/wx/jsonval.h"

#include <wx/sizer.h>
#include <wx/button.h>

enum button_actions {
    BUTTON_UPDATE,
    BUTTON_REMOVE
};

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SettingsPanel_Address, SettingsPanel)
    EVT_BUTTON(BUTTON_UPDATE, SettingsPanel_Address::OnUpdate)
    EVT_BUTTON(BUTTON_REMOVE, SettingsPanel_Address::OnRemove)

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
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);
    
    wxSizer *info_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Address Information");
    
    /* Status information about address. */
    Text *server_name_info = new Text(this, wxString(wxT("Server Name: ") + server_name));
    Text *address_info = new Text(this, wxString(wxT("Address: ") + address));
    Text *p12_info = new Text(this, wxString(wxT("Identity last updated: Today")));
    Text *token_info = new Text(this, wxString(wxT("Token last updated: Today")));
    
    info_box->Add(server_name_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
    info_box->Add(address_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
    info_box->Add(p12_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
    info_box->Add(token_info, SETTINGS_PANEL_BOXSIZER_OPTIONS);
    
    vert_sizer->Add(info_box, SETTINGS_PANEL_SIZER_OPTIONS);
    
    Text *update_warning = new Text(this, 
		wxT("Note: updating the address identity will send a new decryption password."));
    vert_sizer->Add(update_warning, SETTINGS_PANEL_SIZER_OPTIONS);
    
    /* Control buttons. */
    wxBoxSizer *buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
    
	/* Set the button's verb based on the status of the identity. */
	wxString identity_status;
	identity_status = (wxGetApp().config->HaveIdentity(server_name, address)) 
		? _("Update") : _("Install");

    wxButton *update_button = new wxButton(this, BUTTON_UPDATE, identity_status);
    wxButton *remove_button = new wxButton(this, BUTTON_REMOVE, wxT("Remove"));

	/* Don't allow the remove button, if this is the only account. */
	if (SERURO_MUST_HAVE_ACCOUNT) {
		remove_button->Enable(false);
	}
    
    buttons_sizer->Add(update_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
    buttons_sizer->Add(remove_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
    
    vert_sizer->Add(buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	this->SetSizer(vert_sizer);
}

void SettingsPanel_Address::OnUpdateResponse(SeruroRequestEvent &event)
{
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());

	/* Todo: add erroring checking. */
	if (! api->InstallP12(event.GetResponse())) {
		/* Todo: report that something bad happened. */
	}
	delete api;
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

void SettingsPanel_Address::OnRemove(wxCommandEvent &event)
{
	if (SERURO_MUST_HAVE_ACCOUNT && wxGetApp().config->GetAddressList(server_name).size() == 1) {
		/* Don't allow this account to be removed. */
		wxLogMessage(_("AddressPanel> (OnRemove) Cannot remove last account for server (%s)."),
			server_name);
		/* Todo: display warnning message? */
		return;
	}

    RemoveDialog *dialog = new RemoveDialog(this->server_name, this->address);
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("AddressPanel> (OnRemove) OK"));
		//server_info = dialog->GetValues();
		dialog->DoRemove();

		/* Place the user back on the select servers/accounts panel view.*/
		this->MainPanel()->ShowPanel(SETTINGS_VIEW_TYPE_SERVER);

		/* Remove this server, and all subsequent account views. */
		this->MainPanel()->RemoveTreeItem(SETTINGS_VIEW_TYPE_ADDRESS, this->address, this->server_name);
	}
	delete dialog;
}


