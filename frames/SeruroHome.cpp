
#include "SeruroHome.h"

#include "../SeruroClient.h"
#include "../apps/SeruroApps.h"

DECLARE_APP(SeruroClient);

#define HOME_SUCCESS_WELCOME_TEXT "You are ready to send and receive secure email!"
#define HOME_FAILURE_WELCOME_TEXT "You are almost ready, there are some steps that need your attention."

#define HOME_SERVERS_TEXT "You are using %d Seruro server(s), and the default server is %s."
#define HOME_ACCOUNTS_TEXT "The following Seruro accounts are installed: %s."

#define HOME_CONTACTS_TEXT "There are %d Seruro contact(s) installed. \
Contacts are added %s, you may view them using the \n'Contacts' tab above. "
#define HOME_CONTACTS_MANUAL_TEXT "To add additional contacts use the 'Search' tab above."

#define HOME_APPLICATIONS_TEXT "The following email applications: %s, \
are configured to send and receive secured email."
#define HOME_HELP_TEXT "For help and tutorials on Seruro and secure email, \
please use the 'help' icons or 'Help' tab above."

void SeruroPanelHome::OnAccountStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    event.Skip();
}

void SeruroPanelHome::OnServerStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    event.Skip();
}

void SeruroPanelHome::OnContactStateChange(SeruroStateEvent &event)
{
    this->GenerateServerBox();
    event.Skip();
}

void SeruroPanelHome::OnApplicationStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateApplicationBox();
    event.Skip();
}

void SeruroPanelHome::OnIdentityStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    this->GenerateApplicationBox();
    event.Skip();
}

void SeruroPanelHome::OnOptionStateChange(SeruroStateEvent &event)
{
    if (event.GetValue("option_name") == "auto_download") {
        this->GenerateServerBox();
    }
    event.Skip();
}

void SeruroPanelHome::GenerateWelcomeBox()
{
    if (this->IsReady()) {
        text_welcome->SetLabel(HOME_SUCCESS_WELCOME_TEXT);
    } else {
        text_welcome->SetLabel(HOME_FAILURE_WELCOME_TEXT);
    }
}

void SeruroPanelHome::GenerateServerBox()
{
    /* Server messages / icon. */
    size_t servers_count, contacts_count;
    wxString default_server_name, accounts_string;
    wxArrayString servers, accounts;
    bool automatic_contacts;
    
    servers = theSeruroConfig::Get().GetServerList();
    servers_count = servers.size();
    
    if (theSeruroConfig::Get().GetOption("default_server") != wxEmptyString) {
        default_server_name = theSeruroConfig::Get().GetServerName(theSeruroConfig::Get().GetOption("default_server"));
    } else if (servers_count > 0) {
        default_server_name = theSeruroConfig::Get().GetServerNames()[0];
    } else {
        default_server_name = _("(None)");
    }
    
    servers_welcome->SetLabel(wxString::Format(_(HOME_SERVERS_TEXT), (int) servers_count, default_server_name));
    
    contacts_count = 0;
    for (size_t i = 0; i < servers.size(); ++i) {
        accounts = theSeruroConfig::Get().GetAddressList(servers[i]);
        contacts_count += theSeruroConfig::Get().GetContactsList(servers[i]).size();
        for (size_t j = 0; j < accounts.size(); ++j) {
            if (accounts_string != wxEmptyString) {
                accounts_string = wxString::Format(_("%s, "), accounts_string);
            }
            accounts_string = wxString::Format(_("%s%s"), accounts_string, accounts[j]);
        }
    }
    
    if (accounts_string == wxEmptyString) {
        accounts_string = _("(None)");
    }
    
    automatic_contacts = (theSeruroConfig::Get().GetOption("auto_download") == "true");
    accounts_welcome->SetLabel(wxString::Format(_(HOME_ACCOUNTS_TEXT), accounts_string));
    contacts_welcome->SetLabel(wxString::Format(_(HOME_CONTACTS_TEXT), (int) contacts_count,
        (automatic_contacts) ? "automatic" : "manual"));
    if (! automatic_contacts) {
        contacts_welcome->SetLabel(wxString::Format(_("%s%s"), contacts_welcome->GetLabel(), _(HOME_CONTACTS_MANUAL_TEXT)));
    }
}

void SeruroPanelHome::GenerateApplicationBox()
{
    wxString apps_string;
    wxArrayString apps, app_accounts;
    /* Not used. */
    wxString server_uuid;
    
    apps = theSeruroApps::Get().GetAppList();
    for (size_t i = 0; i < apps.size(); ++i) {
        app_accounts = theSeruroApps::Get().GetAccountList(apps[i]);
        for (size_t j = 0; j < app_accounts.size(); ++j) {
            if (theSeruroApps::Get().IdentityStatus(apps[i], app_accounts[j], server_uuid) != APP_ASSIGNED) {
                continue;
            }
            
            if (apps_string != wxEmptyString) {
                apps_string = wxString::Format(_("%s, "), apps_string);
            }
            apps_string = wxString::Format(_("%s%s"), apps_string, apps[i]);
            break;
        }
    }
    
    if (apps_string == wxEmptyString) {
        apps_string = _("(None)");
    }
    
    applications_welcome->SetLabel(wxString::Format(_(HOME_APPLICATIONS_TEXT), apps_string));
}

SeruroPanelHome::SeruroPanelHome(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Home"))
{
    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *welcome_box = new wxStaticBoxSizer(wxHORIZONTAL, this);
    wxStaticBoxSizer *server_box = new wxStaticBoxSizer(wxHORIZONTAL, this);
    wxSizer *server_messages = new wxBoxSizer(wxVERTICAL);
    wxStaticBoxSizer *application_box = new wxStaticBoxSizer(wxHORIZONTAL, this);
    wxSizer *application_messages = new wxBoxSizer(wxVERTICAL);
    
    text_welcome = new Text(welcome_box->GetStaticBox(), wxEmptyString, false);
    text_welcome->Wrap(SERURO_APP_DEFAULT_WIDTH - 20);
    welcome_box->Add(text_welcome, DIALOGS_BOXSIZER_OPTIONS);
    
    /* Server messages / icon. */
    this->servers_welcome = new Text(server_box->GetStaticBox(), wxEmptyString, false);
    server_messages->Add(servers_welcome, DIALOGS_BOXSIZER_OPTIONS);
    servers_welcome->Wrap(SERURO_APP_DEFAULT_WIDTH - 20);
    this->accounts_welcome = new Text(server_box->GetStaticBox(), wxEmptyString, false);
    server_messages->Add(accounts_welcome, DIALOGS_BOXSIZER_OPTIONS);
    accounts_welcome->Wrap(SERURO_APP_DEFAULT_WIDTH - 20);
    this->contacts_welcome = new Text(server_box->GetStaticBox(), wxEmptyString, false);
    server_messages->Add(contacts_welcome, DIALOGS_BOXSIZER_OPTIONS);
    contacts_welcome->Wrap(SERURO_APP_DEFAULT_WIDTH - 20);
    
    /* Applications box. */
    this->applications_welcome = new Text(application_box->GetStaticBox(), wxEmptyString, false);
    application_messages->Add(applications_welcome, DIALOGS_BOXSIZER_OPTIONS);
    application_messages->Add(new Text(application_box->GetStaticBox(), _(HOME_HELP_TEXT), false), DIALOGS_BOXSIZER_OPTIONS);
    
    /* Apply original state. */
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    this->GenerateApplicationBox();
    
    /* Bindings. */
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnAccountStateChange, this, STATE_TYPE_ACCOUNT);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnServerStateChange, this, STATE_TYPE_SERVER);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnIdentityStateChange, this, STATE_TYPE_IDENTITY);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnContactStateChange, this, STATE_TYPE_CONTACT);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnApplicationStateChange, this, STATE_TYPE_APPLICATION);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &SeruroPanelHome::OnOptionStateChange, this, STATE_TYPE_OPTION);
    
    /* Plug sizers in. */
    sizer->Add(welcome_box, 1, wxEXPAND | wxALL, 10);
    sizer->Add(server_box, 1, wxEXPAND | wxALL, 10);
    server_box->Add(server_messages, DIALOGS_BOXSIZER_OPTIONS);
    sizer->Add(application_box, 1, wxEXPAND | wxALL, 10);
    application_box->Add(application_messages, DIALOGS_BOXSIZER_OPTIONS);
    this->SetSizer(sizer);
    this->Layout();
}

bool SeruroPanelHome::IsReady()
{
	wxArrayString servers, accounts, contacts, apps;
	wxString server_uuid;

	servers = theSeruroConfig::Get().GetServerList();
	if (servers.size() == 0) {
		return false;
	}

	apps = theSeruroApps::Get().GetAppList();

	bool has_account = false;
	bool has_contact = false;
	bool has_assigned = false;
	for (size_t i = 0; i < servers.size(); ++i) {
		accounts = theSeruroConfig::Get().GetAddressList(servers[i]);
		contacts = theSeruroConfig::Get().GetContactsList(servers[i]);
		has_contact = (has_contact || contacts.size() > 0);
		has_account = (has_account || accounts.size() > 0);

		if (has_assigned) continue;

		/* Check for at least one assigned account. */
		for (size_t j = 0; j < accounts.size(); ++j) {
			if (has_assigned) continue;
			for (size_t k = 0; k < apps.size(); ++k) {
				if (theSeruroApps::Get().IdentityStatus(apps[k], accounts[j], server_uuid) == APP_ASSIGNED) {
					has_assigned = true;
					break;
				}
			}
		}
	}

	if (has_account && has_contact && has_assigned) {
		return true;
	}

    return false;
}