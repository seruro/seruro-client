
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

BEGIN_EVENT_TABLE(AccountPage, SetupPage)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_PING, AccountPage::OnPingResult)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void AccountPage::OnPingResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();
	wxJSONValue params;
	wxString server_name, address;

	if (! response["success"].AsBool() || ! response.HasMember("address")) {
		wxLogMessage(_("AccountPage> (OnPingResult) failed to ping server."));
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

	/* Try to add the server to the config, will fail if server existed. */
	bool new_server = wxGetApp().config->AddServer(response["meta"]);
	if (new_server) {
		/* Add the server panel before potentially adding the address panel. */
		//this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_SERVER, response["meta"]["name"].AsString());
	}

	/* The server's CA/CRL might need installing. */
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());

	/* Regardless of the addServer response, the addAddress will determine a UI update.
	 * (If the server was new, and it fails, the address will fail.)
	 * (If the server was new, and it successes, but the address fails, there is a larger problem.)
	 */
	server_name = response["meta"]["name"].AsString();
	address = response["address"].AsString();
	if (! wxGetApp().config->AddAddress(server_name, address)) {
		goto finished;
	} else {
		/* Add an address panel under the server panel. */
		//this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_ADDRESS, 
		//	response["address"].AsString(), response["meta"]["name"].AsString());
	}

	/* After the address has been added, and a token save, check if this is a new server. */
	params["server"] = response["meta"];
	if (new_server) {
		api->CreateRequest(SERURO_API_CA, params, SERURO_API_CALLBACK_CA)->Run();
	}

	/* Only set login_success if the address has been added. */
	this->login_success = true;
	/* Call GoForward, but indicate that this call is from a callback. */
	this->GoForward(true);

finished:
	delete api;
}

AccountPage::AccountPage(SeruroSetup *parent) 
	: SetupPage(parent), AddAccountForm(this), login_success(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	this->next_button = _("&Login");

    wxString msg_text = wxT("Please enter the information for your account:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
    wxSizer *const account_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");
    
    this->AddForm(account_form);
    
    vert_sizer->Add(account_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    this->SetSizer(vert_sizer);
}

/* Use the information in the form to login, update the UI, and allow 
 * the wizard to "move forward". */
bool AccountPage::GoForward(bool from_callback) {
	/* Perform a ping to validate the user's credentials. */
	wxJSONValue server_info, address_info, params;

	/* If a 'previous' login is still valid, allow the user to proceed. */
	if (this->login_success) {
		if (from_callback) {
			this->wizard->ForceForward();
		}
		return true;
	}
	/* If the login failed, do not allow this method to make a subsequent request. */
	if (from_callback) return false;

	if (this->wizard->HasServerInfo()) {
		/* The server information was entered on a previous page. */
		server_info = ((ServerPage *) this->wizard->GetServerPage())->GetValues();
	}

	/* Get values from AddAddressForm. */
	address_info = this->GetValues();
	params["address"] = address_info["address"];
	params["password"] = address_info["password"];

	/* Preserve server info within the request callback using 'meta'. */
	params["meta"] = server_info;
	params["server"] = server_info;

	/* Create a 'ping' request, and within the callback, call 'GoForward' again. */
	SeruroServerAPI *api = new SeruroServerAPI(this);
	//SeruroRequest *request = api->Ping(params);
	//request->Run();
	api->Ping(params)->Run();
	delete api;

	return false;
}

