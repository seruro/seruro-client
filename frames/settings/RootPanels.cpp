
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"
#include "../../SeruroClient.h"
#include "../../api/SeruroServerAPI.h"

#include "../dialogs/AddServerDialog.h"
#include "../dialogs/AddAccountDialog.h"

#include <wx/button.h>
#include <wx/scrolwin.h>

DECLARE_APP(SeruroClient);

enum button_actions
{
	BUTTON_ADD_SERVER,
	BUTTON_ADD_ACCOUNT
};

BEGIN_EVENT_TABLE(SettingsPanel_RootAccounts, SettingsPanel)
	EVT_BUTTON(BUTTON_ADD_SERVER, SettingsPanel_RootAccounts::OnAddServer)
END_EVENT_TABLE()

/* Create the 'AddServer' dialog, and if the user does not cancle, return the values
 * from the dialog's form.
 */
wxJSONValue AddServer()
{
	wxJSONValue server_info;

	AddServerDialog *dialog = new AddServerDialog();
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("RootPanels::AddServer> OK"));
		server_info = dialog->GetValues();
	}
	delete dialog;

	return server_info;
}

/* Try to add an address, prompt for a username and password, display a server or a server menu.
 * Also try to authenticate the user using a PING API call, which will trigger a request.
 */
wxJSONValue AddAddress(wxJSONValue server_info, 
	const wxString &display_server = wxEmptyString)
{
	wxJSONValue address_info;
	/* Params used for a SeruroRequest, which checks auth. */
	wxJSONValue auth_check_params;

	//wxString empty_address = wxEmptyString;
	AddAccountDialog *dialog = new AddAccountDialog(wxEmptyString, display_server);
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("RootPanels::AddAddress> OK"));
		address_info = dialog->GetValues();
	}
	delete dialog;

	auth_check_params["server"] = server_info;
	auth_check_params["address"] = address_info["address"];
	/* Set an explicit password, disabling the built-in auth prompt. */
	auth_check_params["password"] = address_info["password"];

	/* Todo: add server info as meta-data.*/
	/* Todo: add callback which calls addAddress again on an auth failure. */

	return address_info;
}

SettingsPanel::SettingsPanel(SeruroPanelSettings *instance_panel) : 
	wxScrolledWindow(instance_panel->GetViewer(), wxID_ANY), 
	main_panel(instance_panel) {}

SettingsPanelView::SettingsPanelView(SeruroPanelSettings *instance_panel) :
	SettingsPanel(instance_panel)
{
	this->SetWindowStyle(wxBORDER_SIMPLE);

	/* Initialize the child-view using an "unchecked" call to render. 
	 * The constructor should have set the data used by render (which will be checked 
	 * and possibly refreshed by 'Changed'.
	 */
	//this->Render();
}

void SettingsPanel_RootAccounts::OnAddAccount(wxCommandEvent &event)
{

}

void SettingsPanel_RootAccounts::OnAddServer(wxCommandEvent &event)
{
    /* Todo: Get all users (emails) for given server. */
    wxJSONValue server_info;
	wxJSONValue address_info;
	//wxJSONValue auth_server_info;
	
	server_info = AddServer();
	/* Make sure there is a name value, if not then something weird happened. */
	if (! server_info.HasMember("name")) return;

	/* Todo: should loop here until the address receives a valid token. */
	wxString display_server = wxString(_("(New Server) ")) + server_info["name"].AsString();
	address_info = AddAddress(server_info, display_server);

	//auth_check_params["server"] = auth_server_info;
	//auth_check_params["server"]["


	/* Perform a request for nothing. */
}

bool SettingsPanel_RootAccounts::Changed()
{
	return true;
}

void SettingsPanel_RootAccounts::Render()
{
    wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    Text *msg = new Text(this, wxT("The Seruro Client can be configured to support multiple Seruro Servers."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);
    
	/* The Servers list box will show all the available servers. 
	 * Each server will be a child tree item under the root accounts, and will show additional
	 * details about the server such as the last time the CA/CRL was fetched. 
	 */
	wxSizer *server_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Servers List");

	wxString server_string;//, server_name;
	wxArrayString servers_list = wxGetApp().config->GetServerList();
	wxArrayString all_address_list;
	wxArrayString address_list;

	/* Iterate through the list of servers, adding each to list display. */
	for (size_t i = 0; i < servers_list.size(); i++) {
		server_string =  wxGetApp().config->GetServerString(servers_list[i]);
		server_list_sizer->Add(new Text(this, server_string), 
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
		address_list = wxGetApp().config->GetAddressList(servers_list[i]);
		for (size_t j = 0; j < address_list.size(); j++) {
			all_address_list.Add(address_list[j] + wxT(" (") + servers_list[i] + wxT(")"));
		}
	}

	vert_sizer->Add(server_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);
	
	/* Add server button */
	wxBoxSizer *servers_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *add_server_button = new wxButton(this, BUTTON_ADD_SERVER, wxT("Add Server"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//servers_buttons_sizer->AddStretchSpacer();
	servers_buttons_sizer->Add(add_server_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(servers_buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	wxSizer *accounts_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Address List");

	for (size_t i = 0; i < all_address_list.size(); i++) {
		accounts_list_sizer->Add(new Text(this, all_address_list[i]),
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
	}

	vert_sizer->Add(accounts_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

   	/* Add address button */
	//wxBoxSizer *servers_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	//wxButton *add_server_button = new wxButton(this, BUTTON_ADD_SERVER, wxT("Add Account"));
    
    this->SetSizer(vert_sizer);
}

void SettingsPanel_RootGeneral::Render()
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	//wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	//vert_sizer->Add(test_button, 0, wxRIGHT, 5);
    
    Text *msg = new Text(this, 
		wxT("The Seruro Client has many optional features which can be controlled below.")
        wxT("Use the navigation tree on the left to add additional servers and accounts, ")
		wxT("check the status of servers and accounts, configuration application settings, ")
		wxT("and control Seruro extensions."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);

	this->SetSizer(vert_sizer);
}
