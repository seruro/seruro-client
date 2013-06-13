
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"
#include "../frames/dialogs/AddServerDialog.h"

#include "../crypto/SeruroCrypto.h"

BEGIN_EVENT_TABLE(AccountPage, SetupPage)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_PING, AccountPage::OnPingResult)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CA, AccountPage::OnCAResult)

	EVT_CHOICE(wxID_ANY, AccountPage::OnSelectServer)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

/* CA installer (triggered from adding a new server) */
void AccountPage::OnCAResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* There are no more callback actions. */
	this->EnablePage();

	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	/* This is a boolean, which indicates a successful add, but the user may deny. */
	api->InstallCA(response);
	delete api;

	/* Check that the (now-known) CA hash exists within the trusted Root store. */
	this->has_ca = HasServerCertificate(response["server_name"].AsString());
	
	/* If the user canceled the install, stop. */
	if (! has_ca) {
		/* Remove the account and server which was saved to the config. */
		wxJSONValue account_info = this->GetValues();
		//wxGetApp().config->RemoveAddress(response["server_name"], account_info["address"]);
		wxGetApp().config->RemoveServer(response["server_name"].AsString());

		/* Allow the login to "try-again". */
		this->login_success = false;
		return;
	}
	
	/* This result handler may be able to proceed the setup. */
	if (response.HasMember("meta") && response["meta"].HasMember("go_forward")) {
		this->GoForward(true);
	}
}

bool AccountPage::HasServerCertificate(wxString server_name)
{
	SeruroCrypto crypto_helper;

	bool has_ca = crypto_helper.HaveCA(server_name);
	wxLogMessage(_("AccountPage> (HasServerCertificate) certificate (%s) in store: (%s)."), 
		server_name, (has_ca) ? "true" : "false");
	return has_ca;
}

void AccountPage::OnPingResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();
	wxJSONValue params;
	wxString server_name, address;

	/* The server's CA/CRL might need installing. */
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());

	if (! response["success"].AsBool() || ! response.HasMember("address")) {
		wxLogMessage(_("AccountPage> (OnPingResult) failed to ping server."));
		goto enable_form;
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
		goto enable_form;
	}

	/* Try to add the server to the config, will fail if server existed. */
	bool new_server = wxGetApp().config->AddServer(response["meta"]);
	if (new_server) {
		/* Add the server panel before potentially adding the address panel. */
		//this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_SERVER, response["meta"]["name"].AsString());
	}

	/* Regardless of the addServer response, the addAddress will determine a UI update.
	 * (If the server was new, and it fails, the address will fail.)
	 * (If the server was new, and it successes, but the address fails, there is a larger problem.)
	 */
	server_name = response["meta"]["name"].AsString();
	address = response["address"].AsString();
	if (! wxGetApp().config->AddAddress(server_name, address)) {
		goto enable_form;
	} else {
		/* Add an address panel under the server panel. */
		//this->MainPanel()->AddTreeItem(SETTINGS_VIEW_TYPE_ADDRESS, 
		//	response["address"].AsString(), response["meta"]["name"].AsString());
	}

	/* Only set login_success if the address has been added. */
	this->login_success = true;

	/* After the address has been added, and a token save, check if this is a new server. */
	params["server"] = response["meta"];
	if (new_server) {
		params["meta"]["go_forward"] = true;
		api->CreateRequest(SERURO_API_CA, params, SERURO_API_CALLBACK_CA)->Run();
		goto finished;
	} else {
		/* If the server existed before, then a successful add of the account can proceed the
		 * setup. Otherwise the result handler of InstallCA will proceed the setup. */
	
		/* There are no more callback-actions. */
		this->EnablePage();

		/* Call GoForward, but indicate that this call is from a callback. */
		this->GoForward(true);
		goto finished;
	}

enable_form:
	/* There are no more callback-actions (if GoForward is called, it must be called after this). */
	this->EnablePage();

finished:
	delete api;
}

void AccountPage::EnablePage()
{
	this->EnableForm();
	wizard->EnableBack(true);
	wizard->EnableForward(true);
}

void AccountPage::DisablePage()
{
	this->DisableForm();
	wizard->EnableBack(false);
	wizard->EnableForward(false);
}

AccountPage::AccountPage(SeruroSetup *parent) 
	: SetupPage(parent), AddAccountForm(this), login_success(false), has_ca(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	this->next_button = _("&Login");
	//this->wizard->RequireAuth(true);
	this->require_auth = true;

    wxString msg_text = wxT("Please enter the information for your account:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
	/* Server information (when this page is generated), allow a lookup. */
	if (! wizard->HasServerInfo()) {
		this->server_menu = GetServerChoice(this);

		wxSizer *const servers_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Available Servers");
		wxBoxSizer *const servers_sizer = new wxBoxSizer(wxHORIZONTAL);
		Text *servers_text = new Text(this, wxT("Select server:"));
		servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
		servers_sizer->Add(this->server_menu, 0, wxRIGHT, 5);
		servers_box->Add(servers_sizer, DIALOGS_BOXSIZER_OPTIONS);
		vert_sizer->Add(servers_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	}

    wxSizer *const account_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");
    this->AddForm(account_form);
	vert_sizer->Add(account_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	/* Todo: if this came from a new server, check for CA. Otherwise check from the displayed
	 * list of CAs. */

    this->SetSizer(vert_sizer);
}

void AccountPage::OnSelectServer(wxCommandEvent &event)
{
	wxString new_server = server_menu->GetString(server_menu->GetSelection());

	wxLogMessage(_("AccountPage> (OnSelectServer) server (%s) was selected."), new_server);
	this->server_name = new_server;
	this->has_ca = HasServerCertificate(new_server);
}

void AccountPage::DoFocus()
{
	//wxArrayString servers = wxGetApp().config->GetServerList();
	wxLogMessage(_("AccountSetupPage> (DoFocus) focusing the account page."));

	/* If there is no server page, then the initially-generated list is OK. */
	if (! wizard->HasServerInfo()) return;

	/* Otherwise the server name may have changed. */
	//server_name = ((ServerPage*) wizard->GetServerPage())->GetValues()["name"].AsString();

	//this->server_menu->Clear();
	//this->server_menu->Disable();
	//this->server_menu->Append(this->server_name);

	/* This (should) cause the CA lookup (but it is not needed, only the UI update). */
	//this->server_menu->SetSelection(0);
}

/* Use the information in the form to login, update the UI, and allow 
 * the wizard to "move forward". */
bool AccountPage::GoForward(bool from_callback) {
	/* Perform a ping to validate the user's credentials. */
	wxJSONValue server_info, address_info, params;

	/* If a 'previous' login is still valid, allow the user to proceed. */
	if (this->login_success && this->has_ca) {
		if (from_callback) {
			this->wizard->ForceForward();
		}
		return true;
	}
	/* If the login failed, do not allow this method to make a subsequent request. */
	if (from_callback) return false;

	/* About to perform some callback-action. (Must disable the form and next). */
	this->DisablePage();

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

