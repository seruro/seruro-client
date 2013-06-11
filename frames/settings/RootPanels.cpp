
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"

#include "../../SeruroClient.h"
#include "../../setup/SeruroSetup.h"
#include "../../api/SeruroServerAPI.h"

#include "../dialogs/AddServerDialog.h"
#include "../dialogs/AddAccountDialog.h"

#include <wx/button.h>
#include <wx/scrolwin.h>

DECLARE_APP(SeruroClient);

enum button_actions
{
	BUTTON_ADD_SERVER,
	BUTTON_ADD_ADDRESS
};

BEGIN_EVENT_TABLE(SettingsPanel_RootAccounts, SettingsPanel)
	EVT_BUTTON(BUTTON_ADD_SERVER, SettingsPanel_RootAccounts::OnAddServer)
	EVT_BUTTON(BUTTON_ADD_ADDRESS, SettingsPanel_RootAccounts::OnAddAddress)

	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_PING, SettingsPanel_RootAccounts::OnAddAddressResult)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CA, SettingsPanel_RootAccounts::OnCAResult)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, SettingsPanel_RootAccounts::OnP12sResult)
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
 * The first parameter (caller) is required for the API event handler.
 */
wxJSONValue AddAddress(wxWindow *caller, 
	wxJSONValue server_info = wxJSONValue(wxJSONTYPE_OBJECT), 
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
	} else {
		delete dialog;
		return address_info;
	}
	delete dialog;

	auth_check_params["address"] = address_info["address"];
	/* Set an explicit password, disabling the built-in auth prompt. */
	auth_check_params["password"] = address_info["password"];

	/* The server info can be set explicitly, or (if not) the user will choose a server name.
	 * Both the authentication parameters and callback metadata need the server info.
	 */
	if (! server_info.HasMember("name")) {
		auth_check_params["meta"] = wxGetApp().config->GetServer(address_info["server_name"].AsString());
		auth_check_params["server"] = auth_check_params["meta"];
	} else {
		auth_check_params["server"] = server_info;
		auth_check_params["meta"] = server_info; 
	}

	/* (Optionally) allow the user to install their identity when the address is added. */
	auth_check_params["meta"]["install_identity"] = address_info["install_identity"];
	
	SeruroServerAPI *api = new SeruroServerAPI(caller);
	SeruroRequest *request = api->Ping(auth_check_params);
	request->Run();
	delete api;

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

/* CA/P12s installers (triggered from adding a new server/address */
void SettingsPanel_RootAccounts::OnCAResult(SeruroRequestEvent &event)
{
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	api->InstallCA(event.GetResponse());
	delete api;
}

void SettingsPanel_RootAccounts::OnP12sResult(SeruroRequestEvent &event)
{
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());

	/* Todo: add erroring checking. */
	if (! api->InstallP12(event.GetResponse())) {
		/* Todo: report that something bad happened. */
	}
	delete api;
}

void SettingsPanel_RootAccounts::OnAddAddressResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();
	wxJSONValue params;
	wxString server_name, address;

	if (! response["success"].AsBool() || ! response.HasMember("address")) {
		wxLogMessage(_("RootAccounts (AddAddressResult) failed to ping server."));
		return;
	}

	/* Every address added will be identified by the server it is added "under".
	 * This server may be a "new" server, meaning addAddress was called as a response
	 * to addServer.
	 *
	 * In both cases, the server information must exists.
	 * Also, try to add the server without intellegence. 
	 */
	if (! response.HasMember("meta") || ! response["meta"].HasMember("name")) {
		wxLogMessage(_("RootAccounts> (AddAddressResult) no server name in meta."));
		return;
	}

	/* The server's CA/CRL might need installing, as well as (optional) a P12. */
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	//SeruroRequest *request;

	/* Save references. */
	server_name = response["meta"]["name"].AsString();
	address = response["address"].AsString();

	/* Save the results to the application's config. */
	bool new_server = wxGetApp().config->AddServer(response["meta"]);
	if (new_server) {
		/* Add the server panel before potentially adding the address panel. */
		this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_SERVER, response["meta"]["name"].AsString());
	}

	/* Regardless of the addServer response, the addAddress will determine a UI update.
	 * (If the server was new, and it fails, the address will fail.)
	 * (If the server was new, and it successes, but the address fails, there is a larger problem.)
	 */
	if (! wxGetApp().config->AddAddress(server_name, address)) {
		goto finished;
	} else {
		/* Check if the user wanted the P12 installed. */

		/* Add an address panel under the server panel. */
		this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_ADDRESS, 
			response["address"].AsString(), response["meta"]["name"].AsString());
	}

	/* Finally, (after the address has been added) and a token saved. */
	params["server"] = response["meta"];
	if (new_server) {
		api->CreateRequest(SERURO_API_CA, params, SERURO_API_CALLBACK_CA)->Run();
	}
	
	/* (Optionally) install the address identity. */
	if (response["meta"].HasMember("install_identity") && response["meta"]["install_identity"].AsBool()) {
		params["address"] = response["address"];
		api->CreateRequest(SERURO_API_P12S, params, SERURO_API_CALLBACK_P12S)->Run();
	}

finished:
	delete api;

	/* Update the UI for this panel. */
	ReRender();
}

void SettingsPanel_RootAccounts::OnAddAddress(wxCommandEvent &event)
{
	AddAddress(this);
}

void SettingsPanel_RootAccounts::OnAddServer(wxCommandEvent &event)
{
    /* Todo: Get all users (emails) for given server. */
    wxJSONValue server_info;
	wxJSONValue address_info;
	
	/* Testing wizard-implementation. */
	SeruroSetup add_server_setup((wxFrame*) (wxGetApp().GetFrame()), true);
	add_server_setup.RunWizard(add_server_setup.GetInitialPage());
	return;

	server_info = AddServer();
	/* Make sure there is a name value, if not then something weird happened. */
	if (! server_info.HasMember("name")) return;

	if (SERURO_MUST_HAVE_ACCOUNT) {
		/* Todo: should loop here until the address receives a valid token. */
		wxString display_server = wxString(_("(New Server) ")) + server_info["name"].AsString();
		AddAddress(this, server_info, display_server);
	} else {
		/* Add server to config, and rerender. */
		wxGetApp().config->AddServer(server_info);
		/* Add a new server panel. */
		this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_SERVER, server_info["name"].AsString());
		ReRender();
	}
}

bool SettingsPanel_RootAccounts::Changed()
{
	return true;
}

void SettingsPanel_RootAccounts::Render()
{
    wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    wxArrayString servers_list = wxGetApp().config->GetServerList();

    Text *msg = new Text(this, wxT("The Seruro Client can be configured to support multiple Seruro Servers."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);
    
	/* The Servers list box will show all the available servers. 
	 * Each server will be a child tree item under the root accounts, and will show additional
	 * details about the server such as the last time the CA/CRL was fetched. 
	 */
	wxString server_string;//, server_name;
	wxArrayString all_address_list;
	wxArrayString address_list;


	if (servers_list.size() != 0) {
		wxSizer *server_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Servers List");

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
	}

	/* Add server button (will always be shown. */
	wxBoxSizer *servers_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *add_server_button = new wxButton(this, BUTTON_ADD_SERVER, wxT("Add Server"));

	/* Add spacer, and button to horz sizer, then horz sizer to vert sizer. */
	//servers_buttons_sizer->AddStretchSpacer();
	servers_buttons_sizer->Add(add_server_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(servers_buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

	/* If there are no servers, we are done. */
	if (servers_list.size() == 0) {
		this->SetSizer(vert_sizer);
		return;
	}

	wxSizer *accounts_list_sizer = new wxStaticBoxSizer(wxVERTICAL, this, "&Address List");

	for (size_t i = 0; i < all_address_list.size(); i++) {
		accounts_list_sizer->Add(new Text(this, all_address_list[i]),
			SETTINGS_PANEL_BOXSIZER_OPTIONS);
	}

	vert_sizer->Add(accounts_list_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

   	/* Add address button */
	wxBoxSizer *addresses_buttons_sizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *add_address_button = new wxButton(this, BUTTON_ADD_ADDRESS, wxT("Add Address"));
    
	addresses_buttons_sizer->Add(add_address_button, SETTINGS_PANEL_BUTTONS_OPTIONS);
	vert_sizer->Add(addresses_buttons_sizer, SETTINGS_PANEL_SIZER_OPTIONS);

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
