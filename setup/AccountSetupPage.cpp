
#include "SeruroSetup.h"
//#include "../frames/dialogs/AddServerDialog.h"
//#include "../frames/dialogs/AddAccountDialog.h"

#include "../crypto/SeruroCrypto.h"
#include "../api/SeruroStateEvents.h"

#include <wx/event.h>

BEGIN_EVENT_TABLE(AccountPage, SetupPage)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_PING, AccountPage::OnPingResult)
	EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CA, AccountPage::OnCAResult)

	EVT_CHOICE(wxID_ANY, AccountPage::OnSelectServer)

    EVT_TEXT_PASTE(wxID_ANY, AccountPage::OnPastePassword)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void AccountPage::OnPastePassword(wxClipboardTextEvent& event)
{
	if (! this->password->HasFocus()) {
		/* This is a good opprotunity to sanitize the data. */
		event.Skip();
		return;
	}

	/* The password field must have it's data dumped manually. */
	PasteIntoControl(this->password);
}

/* CA installer (triggered from adding a new server) */
void AccountPage::OnCAResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* There are no more callback actions. */
	this->EnablePage();

	SeruroServerAPI *api = new SeruroServerAPI(this);
	/* This is a boolean, which indicates a successful add, but the user may deny. */
	api->InstallCA(response);
	delete api;

	/* Check that the (now-known) CA hash exists within the trusted Root store. */
	SeruroCrypto crypto_helper;
	this->has_ca = crypto_helper.HaveCA(response["server_uuid"].AsString());

	/* If the user canceled the install, stop. */
	if (! has_ca) {
		/* Remove the account and server which was saved to the config. */
		wxJSONValue account_info = AddAccountForm::GetValues();

		wxGetApp().config->RemoveServer(response["server_uuid"].AsString());
		SetAccountStatus(_("Unable to install certificate authority."));

        /* Process remove server event. */
        SeruroStateEvent state_event(STATE_TYPE_SERVER, STATE_ACTION_REMOVE);
		state_event.SetServerUUID(response["server_uuid"].AsString());
		this->ProcessWindowEvent(state_event);
        
		/* Allow the login to "try-again". */
		this->FocusForm();
		this->login_success = false;
		return;
	}
    
    /* Process update server event (now with certificate). */
    SeruroStateEvent state_event(STATE_TYPE_SERVER, STATE_ACTION_UPDATE);
    state_event.SetServerUUID(response["server_uuid"].AsString());
    this->ProcessWindowEvent(state_event);
	
	/* This result handler may be able to proceed the setup. */
	if (response.HasMember("meta") && response["meta"].HasMember("go_forward")) {
		this->GoNext(true);
	}
}

void AccountPage::OnPingResult(SeruroRequestEvent &event)
{
	wxJSONValue response;
	wxJSONValue params, server_info;
	wxString address;
    bool new_server;

	/* The server's CA/CRL might need installing. */
	SeruroServerAPI *api = new SeruroServerAPI(this);
    response = event.GetResponse();

	if (! response["success"].AsBool() || ! response.HasMember("address")) {
		wxLogMessage(_("AccountPage> (OnPingResult) failed to ping server."));
		if (response["error"].AsString().compare(_(SERURO_API_ERROR_CONNECTION)) == 0) {
			SetAccountStatus(response["error"].AsString());
		} else {
			SetAccountStatus(_("Invalid account information."));
		}

		goto enable_form;
	}

	/* Every address added will be identified by the server it is added "under".
	 * This server may be a "new" server, meaning addAddress was called as a response
	 * to addServer.
	 *
	 * In both cases, the server information must exists.
	 * Also, try to add the server without intellegence. 
	 */
	if (! response.HasMember("name")) {
		wxLogMessage(_("RootAccounts> (AddAddressResult) no server name in response."));
		SetAccountStatus(_("Invalid server response."));
		goto enable_form;
	}

    /* Set the instance variable for additiona pages to access. */
    this->server_uuid = response["uuid"].AsString();
    
    /* Try to add the server to the config, will fail if server existed. */
    server_info["uuid"] = this->server_uuid;
    server_info["name"] = response["name"];
    server_info["host"] = response["meta"]["host"];
    server_info["port"] = response["meta"]["port"];
	new_server = wxGetApp().config->AddServer(server_info);
    
	if (new_server) {
        /* Create new server event. */
        SeruroStateEvent event(STATE_TYPE_SERVER, STATE_ACTION_ADD);
		event.SetServerUUID(this->server_uuid);
		this->ProcessWindowEvent(event);
	}

	/* Regardless of the addServer response, the addAddress will determine a UI update.
	 * (If the server was new, and it fails, the address will fail.)
	 * (If the server was new, and it successes, but the address fails, there is a larger problem.)
	 */
	address = response["address"].AsString();
	if (! wxGetApp().config->AddAddress(this->server_uuid, address)) {
		SetAccountStatus(_("Account already exists."));
		goto enable_form;
	} else {
        /* Create new server event. */
        SeruroStateEvent event(STATE_TYPE_ACCOUNT, STATE_ACTION_ADD);
		event.SetServerUUID(this->server_uuid);
        event.SetAccount(address);
		this->ProcessWindowEvent(event);
	}

	/* Only set login_success if the address has been added. */
	this->login_success = true;
	SetAccountStatus(_("Success."));

	/* After the address has been added, and a token save, check if this is a new server. */
	params["server"] = server_info;
	if (new_server) {
		params["meta"]["go_forward"] = true;
        params["no_prompt"] = true;
		api->CreateRequest(SERURO_API_CA, params, SERURO_API_CALLBACK_CA)->Run();
		goto finished;
	} else {
		/* If the server existed before, then a successful add of the account can proceed the
		 * setup. Otherwise the result handler of InstallCA will proceed the setup. 
         */
        this->has_ca = true;
	
		/* There are no more callback-actions. */
		this->EnablePage();

		/* Call GoForward, but indicate that this call is from a callback. */
		this->GoNext(true);
		goto finished;
	}

enable_form:
	/* There are no more callback-actions (if GoForward is called, it must be called after this). */
	this->FocusForm();
	this->EnablePage();

finished:
	delete api;
}

void AccountPage::EnablePage()
{
	this->EnableForm();
	wizard->EnablePrev(true);
	wizard->EnableNext(true);

	this->FocusForm();
}

void AccountPage::DisablePage()
{
	this->DisableForm();
	wizard->EnablePrev(false);
	wizard->EnableNext(false);
}

AccountPage::AccountPage(SeruroSetup *parent) 
	: SetupPage(parent), AddAccountForm(this), AddServerForm(this),
	  login_success(false), has_ca(false)
{
    wxSizer *const vert_sizer = new wxBoxSizer(wxVERTICAL);
    
	this->next_button = _("&Login");
	this->require_auth = true;
    this->enable_next = true;
    this->enable_prev = false;

    wxString msg_text = wxT("Please enter the information for your account:");
    Text *msg = new Text(this, msg_text);
    vert_sizer->Add(msg, DIALOGS_SIZER_OPTIONS);
    
	/* Server information (when this page is generated), allow a lookup. */
	if (! wizard->IsNewServer()) {
		this->server_menu = GetServerChoice(this);

		wxSizer *const servers_box = new wxStaticBoxSizer(wxVERTICAL, this, "&Available Servers");
		wxBoxSizer *const servers_sizer = new wxBoxSizer(wxHORIZONTAL);
		Text *servers_text = new Text(this, wxT("Select server:"));
		servers_sizer->Add(servers_text, 0, wxRIGHT, 5);
		servers_sizer->Add(this->server_menu, 0, wxRIGHT, 5);
		servers_box->Add(servers_sizer, DIALOGS_BOXSIZER_OPTIONS);
		vert_sizer->Add(servers_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	} else {
		/* No pre-existing server, the user is adding a server. */
		wxSizer *const server_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Server Information");
    
		AddServerForm::AddForm(server_form);
        Bind(wxEVT_COMMAND_CHECKBOX_CLICKED, &AccountPage::OnCustomPort, this, SERURO_ADD_SERVER_PORT_ID);
        
		vert_sizer->Add(server_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	}

    wxSizer *const account_form = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");

	/* Add the form, which is itself, a grid sizer. */
    AddAccountForm::AddForm(account_form);

    /* Add a status message (display a response if the 'add' was not successful). */
    wxSizer *const status_sizer = new wxBoxSizer(wxHORIZONTAL);
	status_sizer->Add(new Text(this, _("Login status: ")), DIALOGS_SIZER_OPTIONS);
    status_sizer->SetItemMinSize((size_t) 0, SERURO_SETTINGS_FLEX_LABEL_WIDTH, -1);
    
	this->account_status = new Text(this, _("Not logged in."));
	status_sizer->Add(this->account_status, wxSizerFlags().Expand().Border(wxTOP | wxBOTTOM, 5));
	account_form->Add(status_sizer, DIALOGS_BOXSIZER_OPTIONS);
    
	vert_sizer->Add(account_form, DIALOGS_BOXSIZER_SIZER_OPTIONS);

	/* Show textual status messages for the account (login success) and server
	 * (connectivity/CA installation success).
	 */
    this->SetSizer(vert_sizer);
}

void AccountPage::OnCustomPort(wxCommandEvent &event)
{
    AddServerForm::OnCustomPort();
}

void AccountPage::OnSelectServer(wxCommandEvent &event)
{
	wxString new_server_name;
    SeruroCrypto crypto_helper;
    
    new_server_name = server_menu->GetString(server_menu->GetSelection());
	wxLogMessage(_("AccountPage> (OnSelectServer) server (%s) was selected."), new_server_name);
    
	this->server_uuid = wxGetApp().config->GetServerUUID(new_server_name);
	this->has_ca = crypto_helper.HaveCA(this->server_uuid);
}

void AccountPage::DoFocus()
{
	//wxArrayString servers = wxGetApp().config->GetServerList();
	wxLogDebug(_("AccountSetupPage> (DoFocus) focusing the account page."));

	/* If there is no server page, then the initially-generated list is OK. */
	if (! wizard->IsNewServer()) return;

	/* Otherwise the server name may have changed. */
	/* This (should) cause the CA lookup (but it is not needed, only the UI update). */
}

/* Use the information in the form to login, update the UI, and allow 
 * the wizard to "move forward". */
bool AccountPage::GoNext(bool from_callback) {
	/* Perform a ping to validate the user's credentials. */
	wxJSONValue server_info, address_info, params;

    wxLogDebug(_("AccountSetupPage> (GoNext) trying to proceed the setup."));
	/* If a 'previous' login is still valid, allow the user to proceed. */
	if (this->login_success && this->has_ca) {
		if (from_callback) {
			this->wizard->ForceNext();
		}
		return true;
	}
	/* If the login failed, do not allow this method to make a subsequent request. */
	if (from_callback) return false;

	/* About to perform some callback-action. (Must disable the form and next). */
	this->DisablePage();

	if (this->wizard->IsNewServer()) {
		/* The server information was entered on a previous page. */
		//server_info = ((ServerPage *) this->wizard->GetServerPage())->GetValues();
		server_info = AddServerForm::GetValues();
	} else {
        server_info = wxGetApp().config->GetServer(
            wxGetApp().config->GetServerUUID(server_menu->GetString(server_menu->GetSelection()))
        );
    }

	/* Get values from AddAddressForm. */
	address_info = AddAccountForm::GetValues();
	params["address"] = address_info["address"];
    /* Prevent the "bad-auth" UI component, return errors directly to this handler. */
    params["require_password"] = true;
	if (address_info["password"].AsString().compare(wxEmptyString) == 0) {
		/* If the user did not enter a password, fill un null. */
		params["password"] = "null";
	} else {
		params["password"] = address_info["password"];
	}

	/* Preserve server info within the request callback using 'meta'. */
	params["meta"] = server_info;
	params["server"] = server_info;

    /* Create a 'ping' request, and within the callback, call 'GoForward' again. */
	SeruroServerAPI *api = new SeruroServerAPI(this);
	SetAccountStatus(_("Connecting..."));
	api->Ping(params)->Run();
	delete api;

	return false;
}

