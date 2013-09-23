
#include "SeruroHome.h"

#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../apps/SeruroApps.h"
#include "UIDefs.h"

#include "SeruroMain.h"
#include "SeruroSetup.h"

#include <wx/statbmp.h>

#define HOME_SUCCESS_WELCOME_TEXT "You are ready to use secured email"
#define HOME_FAILURE_WELCOME_TEXT "Some steps need your attention"

//#define HOME_SERVERS_TEXT "You are using %d Seruro server(s), and the default server is %s."
//#define HOME_ACCOUNTS_TEXT "The following Seruro accounts are installed: %s."

//#define HOME_CONTACTS_TEXT "There are %d Seruro contact(s) installed. \
Contacts are added %s, you may view them using the \n'Contacts' tab above. "
//#define HOME_CONTACTS_MANUAL_TEXT "To add additional contacts use the 'Search' tab above."

//#define HOME_APPLICATIONS_TEXT "The following email applications: %s, \
are configured to send and receive secured email."

#define HOME_SERVERS_TEXT "Servers Connected: %d"
#define HOME_ACCOUNTS_TEXT "Secured Accounts: %d"
#define HOME_CONTACTS_TEXT "Contacts Installed: %d"
#define HOME_APPLICATIONS_TEXT "%s, %s configured to use Seruro"

#define HOME_HELP_TEXT "Thank you for using Seruro to secure your email. \
You can find help and tutorials on sending and receiving secure email \
anytime using the 'help' icons or 'Help' tab above. \
If you need to make changes to your Seruro identity please visit the web address of your Seruro application."

#include "../resources/images/logo_block_128_flat.png.h"
#include "../resources/images/home_servers.png.h"
#include "../resources/images/home_contacts.png.h"

enum home_buttons_t
{
    BUTTON_ACTION_ID,
};

BEGIN_EVENT_TABLE(SeruroPanelHome, SeruroPanel)
    EVT_BUTTON(BUTTON_ACTION_ID, SeruroPanelHome::OnAction)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void SeruroPanelHome::OnAction(wxCommandEvent &event)
{
    if (action_type == HOME_ACTION_SERVER) {
        SeruroSetup *add_server_setup = new SeruroSetup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_SERVER);
        ((SeruroFrameMain *) wxGetApp().GetFrame())->SetSetup(add_server_setup);
    } else if (action_type == HOME_ACTION_ACCOUNT) {
        SeruroSetup *add_account_setup = new SeruroSetup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_ACCOUNT);
        ((SeruroFrameMain *) wxGetApp().GetFrame())->SetSetup(add_account_setup);
    } else if (action_type == HOME_ACTION_IDENTITY) {
        /* Show identity setup page for first account? */
    } else if (action_type == HOME_ACTION_CONTACTS) {
        /* If the auto download is set, then fail, else go to search. */
        if (theSeruroConfig::Get().GetOption("auto_download") != "true") {
            /* Show search page. */
        }
    }
}

void SeruroPanelHome::OnAccountStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    
    this->Layout();
    event.Skip();
}

void SeruroPanelHome::OnServerStateChange(SeruroStateEvent &event)
{
    if (event.GetAction() == STATE_ACTION_ADD || event.GetAction() == STATE_ACTION_REMOVE) {
        this->GenerateWelcomeBox();
        this->GenerateServerBox();
    }
    
    this->Layout();
    event.Skip();
}

void SeruroPanelHome::OnContactStateChange(SeruroStateEvent &event)
{
    if (event.GetAction() == STATE_ACTION_ADD || event.GetAction() == STATE_ACTION_REMOVE) {
        this->GenerateWelcomeBox();
        this->GenerateServerBox();
    }

    this->Layout();
    event.Skip();
}

void SeruroPanelHome::OnApplicationStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateApplicationBox();

    this->Layout();
    event.Skip();
}

void SeruroPanelHome::OnIdentityStateChange(SeruroStateEvent &event)
{
    this->GenerateWelcomeBox();
    this->GenerateServerBox();
    this->GenerateApplicationBox();
    
    this->Layout();
    event.Skip();
}

void SeruroPanelHome::OnOptionStateChange(SeruroStateEvent &event)
{
    if (event.GetValue("option_name") == "auto_download") {
        this->GenerateServerBox();
        this->Layout();
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

    this->Layout();
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
    
    //servers_welcome->SetLabel(wxString::Format(_(HOME_SERVERS_TEXT), (int) servers_count, default_server_name));
    servers_welcome->SetLabel(wxString::Format(_(HOME_SERVERS_TEXT), (int) servers_count));
    
    
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
    
    //accounts_welcome->SetLabel(wxString::Format(_(HOME_ACCOUNTS_TEXT), accounts_string));
    accounts_welcome->SetLabel(wxString::Format(_(HOME_ACCOUNTS_TEXT), (int) accounts.size()));
    
    //contacts_welcome->SetLabel(wxString::Format(_(HOME_CONTACTS_TEXT), (int) contacts_count,
    //    (automatic_contacts) ? "automatic" : "manual"));
    contacts_welcome->SetLabel(wxString::Format(_(HOME_CONTACTS_TEXT), (int) contacts_count));
    
    if (! automatic_contacts) {
        //contacts_welcome->SetLabel(wxString::Format(_("%s%s"), contacts_welcome->GetLabel(), _(HOME_CONTACTS_MANUAL_TEXT)));
    }
}

void SeruroPanelHome::GenerateApplicationBox()
{
    wxString apps_string;
    wxArrayString apps, app_accounts;
    size_t configured_apps;
    /* Not used. */
    wxString server_uuid;
    
    configured_apps = 0;
    apps = theSeruroApps::Get().GetAppList();
    for (size_t i = 0; i < apps.size(); ++i) {
        app_accounts = theSeruroApps::Get().GetAccountList(apps[i]);
        for (size_t j = 0; j < app_accounts.size(); ++j) {
            if (theSeruroApps::Get().IdentityStatus(apps[i], app_accounts[j], server_uuid) != APP_ASSIGNED) {
                continue;
            }
            
            configured_apps += 1;
            if (apps_string != wxEmptyString) {
                apps_string = wxString::Format(_("%s, "), apps_string);
            }
            apps_string = wxString::Format(_("%s%s"), apps_string, apps[i]);
            break;
        }
    }
    
    if (apps_string == wxEmptyString) {
        apps_string = _("No applications");
    }
    
    //applications_welcome->SetLabel(wxString::Format(_(HOME_APPLICATIONS_TEXT), apps_string));
    applications_welcome->SetLabel(wxString::Format(_(HOME_APPLICATIONS_TEXT), apps_string, (configured_apps == 1) ? _("is") : _("are")));
}

SeruroPanelHome::SeruroPanelHome(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Home"))
{    
    /* Improved home. */
    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxSizer *status_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    /* Improved "world". */
    wxSizer *world_sizer = new wxBoxSizer(wxVERTICAL);
    //world_sizer->SetMinSize((SERURO_APP_DEFAULT_WIDTH-20)/3, SERURO_APP_DEFAULT_HEIGHT-20);
    wxStaticBitmap *world_image = new wxStaticBitmap(this, wxID_ANY, wxGetBitmapFromMemory(home_servers));
    world_sizer->Add(world_image, 1, wxALIGN_CENTER, 0);
    status_sizer->Add(world_sizer, 1, wxALIGN_CENTER, 1);
    world_sizer->AddSpacer(20);
    
    servers_welcome = new Text(this, wxEmptyString, false);
    servers_welcome->Wrap((SERURO_APP_DEFAULT_WIDTH-20)/3);
    world_sizer->Add(servers_welcome, 0, wxALIGN_CENTER, 0);
    
    accounts_welcome = new Text(this, wxEmptyString, false);
    accounts_welcome->Wrap((SERURO_APP_DEFAULT_WIDTH-20)/3);
    world_sizer->Add(accounts_welcome, 0, wxALIGN_CENTER, 0);
    
    world_sizer->Add(new Text(this, _("Notifications: None")), 0, wxALIGN_CENTER, 0);
    
    /* Improved "Seruro-center". */
    wxSizer *seruro_sizer = new wxBoxSizer(wxVERTICAL);
    //seruro_sizer->SetMinSize((SERURO_APP_DEFAULT_WIDTH-20)/3, SERURO_APP_DEFAULT_HEIGHT-50);
    wxStaticBitmap *seruro_image = new wxStaticBitmap(this, wxID_ANY, wxGetBitmapFromMemory(logo_block_128_flat));
    seruro_sizer->Add(seruro_image, 1, wxALIGN_CENTER, 0);
    status_sizer->Add(seruro_sizer, 0, wxALIGN_CENTER, 0);
    
    text_welcome = new Text(this, wxEmptyString, false);
    text_welcome->Wrap((SERURO_APP_DEFAULT_WIDTH-20)/3);
    seruro_sizer->Add(text_welcome, 0, wxALIGN_CENTER, 0);
    
    applications_welcome = new Text(this, wxEmptyString, false);
    applications_welcome->Wrap((SERURO_APP_DEFAULT_WIDTH-20)/3);
    seruro_sizer->Add(applications_welcome, 0, wxALIGN_CENTER, 0);
    
    /* Add a potential "action" button if there's an action required. */
    seruro_sizer->AddSpacer(20);
    this->action_button = new wxButton(this, BUTTON_ACTION_ID, wxEmptyString);
    seruro_sizer->Add(action_button, 0, wxALIGN_CENTER, 0);
    action_button->Hide();
    
    /* Improved "people". */
    wxSizer *people_sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticBitmap *people_image = new wxStaticBitmap(this, wxID_ANY, wxGetBitmapFromMemory(home_contacts));
    people_sizer->Add(people_image, 1, wxALIGN_CENTER, 0);
    status_sizer->Add(people_sizer, 1, wxALIGN_CENTER, 1);
    people_sizer->AddSpacer(20);
    
    contacts_welcome = new Text(this, wxEmptyString, false);
    contacts_welcome->Wrap((SERURO_APP_DEFAULT_WIDTH-20)/3);
    people_sizer->Add(contacts_welcome, 0, wxALIGN_CENTER, 0);
    
    people_sizer->Add(new Text(this, _("Contact Retrieval: Automatic")), 0, wxALIGN_CENTER, 0);
    people_sizer->Add(new Text(this, _("Extensions: None")), 0, wxALIGN_CENTER, 0);
    
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
    
    /* Bottom message / informational box. */
    wxStaticBoxSizer *message_box = new wxStaticBoxSizer(wxHORIZONTAL, this);
    message_box->SetMinSize(-1, 100);
    
    Text *message_text = new Text(message_box->GetStaticBox(), _(HOME_HELP_TEXT), false);
    //message_text->Wrap(SERURO_APP_DEFAULT_WIDTH-20-20);
    message_box->Add(message_text, DIALOGS_BOXSIZER_OPTIONS.Proportion(1));
    
    /* Plug in sizers. */
    sizer->Add(status_sizer, 1, wxEXPAND, 0);
    sizer->Add(message_box, 0, wxEXPAND | wxALL, 10);
    this->SetSizer(sizer);
    this->Layout();
}

void SeruroPanelHome::SetAction(home_actions_t action)
{
    this->action_type = action;
    
    if (action == HOME_ACTION_SERVER) {
        this->action_button->SetLabel(_("Add a Seruro Server"));
    } else if (action == HOME_ACTION_ACCOUNT) {
        this->action_button->SetLabel(_("Add a Seruro Account"));
    } else if (action == HOME_ACTION_IDENTITY) {
        this->action_button->SetLabel(_("Unlock your Seruro Identity"));
    } else if (action == HOME_ACTION_CONTACTS) {
        this->action_button->SetLabel(_("Install Seruro Contacts"));
    }
    this->action_button->Show();
}

bool SeruroPanelHome::IsReady()
{
	wxArrayString servers, accounts, contacts, apps;
	wxString server_uuid;

	servers = theSeruroConfig::Get().GetServerList();
	if (servers.size() == 0) {
        SetAction(HOME_ACTION_SERVER);
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

    /* If there was any cause for failure (false), set the appropriate action. */
    if (!has_account) {
        SetAction(HOME_ACTION_ACCOUNT);
        return false;
    } else if (!has_assigned) {
        SetAction(HOME_ACTION_IDENTITY);
        return false;
    } else if (!has_contact) {
        SetAction(HOME_ACTION_CONTACTS);
        return false;
    }
    
    this->action_button->Hide();
    return true;
}