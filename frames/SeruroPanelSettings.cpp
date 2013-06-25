
#include "SeruroPanelSettings.h"
#include "UIDefs.h"
#include "../SeruroClient.h"

#include "dialogs/RemoveDialog.h"
#include "../setup/SeruroSetup.h"
#include "../crypto/SeruroCrypto.h"

#include <wx/button.h>
#include <wx/log.h>
#include <wx/listctrl.h>

/* Include image data. */
#include "../resources/images/general_icon_42_flat.png.h"
#include "../resources/images/accounts_icon_42_flat.png.h"
#include "../resources/images/applications_icon_42_flat.png.h"
#include "../resources/images/addons_icon_42_flat.png.h"
#include "../resources/images/certificate_icon_12_flat.png.h"
#include "../resources/images/identity_icon_12_flat.png.h"

enum {
    SETTINGS_MENU_ID,
	SETTINGS_SERVERS_LIST_ID,
	SETTINGS_ACCOUNTS_LIST_ID,
    SETTINGS_APPS_LIST_ID,

	BUTTON_ADD_SERVER,
	BUTTON_ADD_ACCOUNT,
	BUTTON_UPDATE,
	BUTTON_REMOVE
};

#define SETTINGS_MENU_WIDTH 200

/* Set these to help search for strings on selection. */
#define SERVERS_LIST_NAME_COLUMN 1
#define ACCOUNTS_LIST_NAME_COLUMN 1
#define ACCOUNTS_LIST_SERVER_COLUMN 2

BEGIN_EVENT_TABLE(SeruroPanelSettings, SeruroPanel)
	EVT_LIST_ITEM_SELECTED(SETTINGS_MENU_ID, SeruroPanelSettings::OnSelected)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(AccountsWindow, SettingsView)
	EVT_LIST_ITEM_SELECTED(SETTINGS_SERVERS_LIST_ID, AccountsWindow::OnServerSelected)
	EVT_LIST_ITEM_SELECTED(SETTINGS_ACCOUNTS_LIST_ID, AccountsWindow::OnAccountSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, AccountsWindow::OnDeselect)

    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CA, AccountsWindow::OnCAResult)
    //EVT_SERURO_REQUEST(SERURO_API_CALLBACK_P12S, AccountsWindow::OnP12sResult)

	EVT_BUTTON(BUTTON_ADD_SERVER, AccountsWindow::OnAddServer)
	EVT_BUTTON(BUTTON_ADD_ACCOUNT, AccountsWindow::OnAddAccount)
	EVT_BUTTON(BUTTON_UPDATE, AccountsWindow::OnUpdate)
	EVT_BUTTON(BUTTON_REMOVE, AccountsWindow::OnRemove)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

//#undef DIALOGS_SIZER_OPTIONS
//#undef DIALOGS_BOXSIZER_SIZER_OPTIONS
//#define DIALOGS_SIZER_OPTIONS wxSizerFlags().Border(wxTOP | wxRIGHT | wxLEFT, 5)
//#define DIALOGS_BOXSIZER_SIZER_OPTIONS DIALOGS_SIZER_OPTIONS

void SeruroPanelSettings::OnSelected(wxListEvent &event)
{
	/* Hide all windows, then show the selected. */
	this->Freeze();
	general_window->Hide();
	accounts_window->Hide();
	applications_window->Hide();
	extensions_window->Hide();
	if (event.GetItem() == 0) general_window->Show();
	if (event.GetItem() == 1) accounts_window->Show();
	if (event.GetItem() == 2) applications_window->Show();
	if (event.GetItem() == 3) extensions_window->Show();
	this->Thaw();
	/* Ask the window/sizer to position/size the now-shown window correctly. */
	this->Layout();
}

SeruroPanelSettings::SeruroPanelSettings(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Settings"))
{
	/* Override default sizer. */
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    
	/* Construct menu. */
	this->AddMenu(sizer);

    general_window = new GeneralWindow(this);
    sizer->Add(general_window, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 10);
    general_window->Hide();

	/* Only the general window is not hidden when created. */
    accounts_window = new AccountsWindow(this);
    sizer->Add(accounts_window, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 10);
    accounts_window->Hide();
    
	applications_window = new ApplicationsWindow(this);
    sizer->Add(applications_window, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 10);
    applications_window->Hide();
    
	extensions_window = new ExtensionsWindow(this);
    sizer->Add(extensions_window, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 10);
    extensions_window->Hide();

    this->SetSizer(sizer);
    //container_sizer->SetSizerHints(this);
}

void SeruroPanelSettings::AddMenu(wxSizer *sizer)
{
	/* Hold each image for settings category. */
    wxImageList *image_list = new wxImageList(42, 30, true);
	image_list->Add(wxGetBitmapFromMemory(general_icon_42_flat));
	image_list->Add(wxGetBitmapFromMemory(accounts_icon_42_flat));
	image_list->Add(wxGetBitmapFromMemory(applications_icon_42_flat));
	image_list->Add(wxGetBitmapFromMemory(addons_icon_42_flat));

    menu = new wxListCtrl(this, SETTINGS_MENU_ID, 
		/* We want a static width, and allow the sizer to determine the height. */
		wxDefaultPosition, wxSize(SERURO_SETTINGS_TREE_MIN_WIDTH, -1),
		/* Report / virtual will allow row colors in the future (2.9.5). */
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER | wxBORDER_SIMPLE);
	/* Set the image list for the selections (must be small). */
	menu->SetImageList(image_list, wxIMAGE_LIST_SMALL);

    wxListItem column;
    menu->InsertColumn(0, column);
	/* We want each selection to highlight the entire row. */
	menu->SetColumnWidth(0, SERURO_SETTINGS_TREE_MIN_WIDTH);

	menu->InsertItem(0, _("General"), 0);
    menu->InsertItem(1, _("Accounts"), 1);
	menu->InsertItem(2, _("Applications"), 2);
	menu->InsertItem(3, _("Extensions"), 3);
    
	sizer->Add(menu, 0, wxEXPAND | wxALL, 10);
}

GeneralWindow::GeneralWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
	//wxButton *button = new wxButton(this, wxID_ANY, _("General"));
    //sizer->Add(button, DIALOGS_SIZER_OPTIONS);

    this->SetSizer(sizer);
}

void AccountsWindow::OnServerSelected(wxListEvent &event)
{
    //wxString server_name;
    wxListItem item;
    
    /* Set the mask as the type of information requested using GetItem. */
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetId(event.GetIndex());
    item.SetColumn(SERVERS_LIST_NAME_COLUMN);
    
    if (! servers_list->GetItem(item)) {
        wxLogMessage(_("AccountsWindow> (OnServerSelected) could not get server name."));
        return;
    }
    
    /* Must deselect all accounts. */
    DeselectAccounts();
    
    /* Server name is now available. */
    this->server_name = item.GetText();
    this->account_selected = false;
    
    if (! wxGetApp().config->HaveCA(server_name)) {
        SetActionLabel(_("Install"));
    } else {
        SetActionLabel(_("Update"));
    }

    this->update_button->Enable(true);
    this->remove_button->Enable(true);
}

void AccountsWindow::OnAccountSelected(wxListEvent &event)
{
    wxListItem item;
    
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetId(event.GetIndex());
    item.SetColumn(ACCOUNTS_LIST_NAME_COLUMN);
    
    if (! accounts_list->GetItem(item)) {
        wxLogMessage(_("AccountsWindow> (OnAddressSelected) could not get address."));
        return;
    }
    
    /* Only one server or account can be selected at a time. */
    DeselectServers();
    
    this->address = item.GetText();
    this->account_selected = true;
    
    /* Also need the server name for this account. */
    item.SetColumn(ACCOUNTS_LIST_SERVER_COLUMN);
    accounts_list->GetItem(item);
    this->server_name = item.GetText();
    
    if (! wxGetApp().config->HaveIdentity(server_name, address)) {
        SetActionLabel(_("Install"));
    } else {
        SetActionLabel(_("Update"));
    }
    
    this->update_button->Enable(true);
    this->remove_button->Enable(true);
}

void AccountsWindow::DeselectServers()
{
    /* Todo: hopefully this does not cause a deselection event */
    wxListItem item;
    int servers_count = servers_list->GetItemCount();
    
    for (int i = 0; i < servers_count; i++) {
        //wxLogMessage(_("AccountsWindow> (DeselectServers) deselecting (%d)."), i);
        item.SetId(i);
        servers_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void AccountsWindow::DeselectAccounts()
{
    /* Todo: hopefully this does not account a deselect event. */
    wxListItem item;
    int accounts_count = accounts_list->GetItemCount();
    
    for (int i = 0; i < accounts_count; i++) {
        //wxLogMessage(_("AccountsWindow> (DeselectAccounts) deselecting (%d)."), i);
        item.SetId(i);
        accounts_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void AccountsWindow::OnDeselect(wxListEvent &event)
{
    this->server_name = wxEmptyString;
    this->address = wxEmptyString;
    
    //wxLogMessage(_("AccountsWindow> (OnDeselect) server/address deselected."));
    
    this->update_button->Disable();
    this->remove_button->Disable();
}

void AccountsWindow::OnCAResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* Simple, there's no UI state to update based on the result. */
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	api->InstallCA(response);
	delete api;
}

void AccountsWindow::OnUpdate(wxCommandEvent &event)
{
    /* All actions must have a server to act on. */
    if (this->server_name.compare(wxEmptyString) == 0) {
        return;
    }
    
    wxJSONValue params;
    SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
    
    params["server"] = wxGetApp().config->GetServer(this->server_name);
    
    /* If updating/installing the server certificate. */
    if (! this->account_selected) {
        /* Create API request to download certificate. */
        /* On callback update the UI based on the status of the cert. */
        
        api->CreateRequest(SERURO_API_CA, params, SERURO_API_CALLBACK_CA)->Run();
        delete api;
        
        return;
    }
    
    /* Open the setup wizard on the identity page. */
    /* Todo: have an event which updates the status of an identity. */
	SeruroSetup identity_setup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_IDENTITY);
	identity_setup.RunWizard(identity_setup.GetInitialPage());
}

void AccountsWindow::OnRemove(wxCommandEvent &event)
{
    /* All actions must have a server to act on. */
    if (this->server_name.compare(wxEmptyString) == 0) {
        return;
    }
    
    /* UI is trying to remove a selected server. */
    if (! this->account_selected) {
        RemoveDialog *dialog = new RemoveDialog(this->server_name);
        if (dialog->ShowModal() == wxID_OK) {
            wxLogMessage(wxT("ServerPanel> (OnRemove) OK"));
            dialog->DoRemove();
            
            /* Todo: update the list of servers/accounts. */
        }
        delete dialog;
        return;
    }
    
    /* UI is trying to remove a selected account. */
    if (SERURO_MUST_HAVE_ACCOUNT &&
        wxGetApp().config->GetAddressList(server_name).size() == 1) {
        /* Don't allow this account to be removed. */
        wxLogMessage(_("AddressPanel> (OnRemove) Cannot remove account for server (%s)."),
                     server_name);
        /* Todo: display warnning message? */
        return;
    }
    
    RemoveDialog *dialog = new RemoveDialog(this->server_name, this->address);
    if (dialog->ShowModal() == wxID_OK) {
        wxLogMessage(wxT("AddressPanel> (OnRemove) OK"));
        dialog->DoRemove();
        
        /* Todo: update the list of accounts. */
    }
    delete dialog;
}

void AccountsWindow::OnAddServer(wxCommandEvent &event)
{
    /* Todo: consider having a new account/new server event? */
    
	/* Testing wizard-implementation. */
	SeruroSetup add_server_setup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_SERVER);
	add_server_setup.RunWizard(add_server_setup.GetInitialPage());
	return;
}

void AccountsWindow::OnAddAccount(wxCommandEvent &event)
{
	/* Testing wizard-implementation. */
	SeruroSetup add_account_setup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_ACCOUNT);
	add_account_setup.RunWizard(add_account_setup.GetInitialPage());
	return;
}

AccountsWindow::AccountsWindow(SeruroPanelSettings *window) : SettingsView(window),
	account_selected(false)
{
    //wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
	//wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
	long item_index;
    
	/* This will hold the column of lists. */
	wxSizer *const lists_sizer = new wxBoxSizer(wxVERTICAL);

	wxImageList *list_images = new wxImageList(12, 12, true);
	list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));

	//wxSizer *const servers_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Servers List"));
	servers_list = new wxListCtrl(this, SETTINGS_SERVERS_LIST_ID,
		wxDefaultPosition, wxDefaultSize, 
		wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
	servers_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);

	/* Server list columns. */
	wxListItem image_column;
	//image_column.SetText(_(""));
	image_column.SetId(0);
	image_column.SetImage(0);
	servers_list->InsertColumn(0, image_column);

	servers_list->InsertColumn(1, _("Server Name"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	servers_list->InsertColumn(2, _("Expires"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	//servers_list->InsertColumn(3, _("Contacts"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

	SeruroCrypto crypto; 
	//bool cert_installed = false;
	wxArrayString servers = wxGetApp().config->GetServerList();
	for (size_t i = 0; i < servers.size(); i++) {
		/* Check if the server certificate is installed. */
		//cert_installed = crypto_helper.HaveCA(servers[i]);
		item_index = servers_list->InsertItem(0, _(""), (crypto.HaveCA(servers[i])) ? 0 : -1);
		servers_list->SetItem(item_index, 1, _(servers[i]));
		servers_list->SetItem(item_index, 2, _("0"));
		//servers_list->SetItem(item_index, 3, _("0"));
	}

	/* Allow the server name column to take up the remaining space. */
	//servers_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	servers_list->SetColumnWidth(0, 24);

	//servers_box->Add(servers_list, DIALOGS_SIZER_OPTIONS);
	//lists_sizer->Add(servers_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	lists_sizer->Add(servers_list, DIALOGS_SIZER_OPTIONS);

	//wxSizer *const accounts_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Accounts List"));
	accounts_list = new wxListCtrl(this, SETTINGS_ACCOUNTS_LIST_ID,
		wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
	accounts_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);

	/* Column for an icon? */
	image_column.SetImage(1);
	accounts_list->InsertColumn(0, image_column);
	accounts_list->InsertColumn(1, _("Address"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	accounts_list->InsertColumn(2, _("Server"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	accounts_list->InsertColumn(3, _("Expires"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

	wxArrayString accounts;
	for (size_t i = 0; i < servers.size(); i++) {
		accounts = wxGetApp().config->GetAddressList(servers[i]);
		for (size_t j = 0; j < accounts.size(); j++) {
			//cert_installed = crypto_helper.HaveIdentity(servers[i], 
			item_index = accounts_list->InsertItem(0, _(""), 
				/* Display a key if the identity (cert/key) are installed. */
				(crypto.HaveIdentity(servers[i], accounts[j])) ? 1 : -1);
			accounts_list->SetItem(item_index, 1, accounts[j]);
			accounts_list->SetItem(item_index, 2, servers[i]);
			accounts_list->SetItem(item_index, 3, _("Never"));
		}
	}

	/* Allow the account/address name column to take up the remaining space. */
	//accounts_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	accounts_list->SetColumnWidth(0, 24);

	//accounts_box->Add(accounts_list, DIALOGS_SIZER_OPTIONS);
	//lists_sizer->Add(accounts_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	lists_sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());

	/* A sizer for ACTION buttons. */
	//wxSizer *const actions_sizer = new wxBoxSizer(wxVERTICAL);
	wxSizer *const actions_sizer = new wxBoxSizer(wxHORIZONTAL);
	update_button = new wxButton(this, BUTTON_UPDATE, _("Update"));
	update_button->Disable();
	remove_button = new wxButton(this, BUTTON_REMOVE, _("Remove"));
	remove_button->Disable();
	wxButton *add_server_button = new wxButton(this, BUTTON_ADD_SERVER, _("Add Server"));
	wxButton *add_account_button = new wxButton(this, BUTTON_ADD_ACCOUNT, _("Add Account"));
	actions_sizer->Add(update_button, DIALOGS_SIZER_OPTIONS);
	actions_sizer->Add(remove_button, DIALOGS_SIZER_OPTIONS);
	actions_sizer->Add(add_server_button, DIALOGS_SIZER_OPTIONS);
	actions_sizer->Add(add_account_button, DIALOGS_SIZER_OPTIONS);

	//sizer->Add(lists_sizer, DIALOGS_SIZER_OPTIONS);
	//sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS);
	//sizer->Add(lists_sizer, DIALOGS_SIZER_OPTIONS);
	lists_sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS.FixedMinSize().Bottom());


	/* Try to set the max height of the lists. */
	//size_t list_height;
	//wxSize list_max_size(-1, -1);
	//wxRect item_rect;
	/* Server list. */
	//if (servers_list->GetItemCount() > 0) {
	//	if (servers_list->GetItemRect(0, item_rect)) {
	//		list_height = item_rect.GetHeight();
	//	} else {
			/* Guess what the height should be? */
	//		list_height = 20;
	//	}
	//}
	/* some number of items, plus an offset for the header. */
	//list_height = (list_height * 3) + 20;
	//list_max_size.SetHeight(list_height);
	//servers_list->SetSize(list_max_size);

	//wxSize actions_size(-1, 30);
	//actions_sizer->SetMinSize(actions_size);

    //this->SetSizer(sizer);
	this->SetSizer(lists_sizer);
}

ApplicationsWindow::ApplicationsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
	//wxSizer *const servers_box = new wxStaticBoxSizer(wxVERTICAL, this, _("Servers List"));
	apps_list = new wxListCtrl(this, SETTINGS_APPS_LIST_ID,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
	//apps_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);
    
	/* Server list columns. */
	//wxListItem image_column;
	//image_column.SetText(_(""));
	//image_column.SetId(0);
	//image_column.SetImage(0);
	//servers_list->InsertColumn(0, image_column);
    
	apps_list->InsertColumn(0, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(1, _("Version"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    /* Application access helper. */
    //apps_helper = SeruroApps();
    long item_index;
	wxArrayString apps = apps_helper.GetAppList();
    wxJSONValue app_info;
	for (size_t i = 0; i < apps.size(); i++) {
		item_index = apps_list->InsertItem(0, _(apps[i]));
        
        app_info = apps_helper.GetApp(apps[i]);
        apps_list->SetItem(item_index, 1, app_info["version"].AsString());
        apps_list->SetItem(item_index, 2, app_info["status"].AsString());
	}
    
    wxArrayString accounts;
    for (size_t i = 0; i < apps.size(); i++) {
        accounts = apps_helper.GetAccountList(apps[i]);
    }
	/* Allow the server name column to take up the remaining space. */
	//servers_list->SetColumnWidth(0, wxLIST_AUTOSIZE);
	//servers_list->SetColumnWidth(0, 24);
    
	//servers_box->Add(servers_list, DIALOGS_SIZER_OPTIONS);
	//lists_sizer->Add(servers_box, DIALOGS_BOXSIZER_SIZER_OPTIONS);
	sizer->Add(apps_list, DIALOGS_SIZER_OPTIONS);
    
    this->SetSizer(sizer);
}

ExtensionsWindow::ExtensionsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
    Text *warning = new Text(this, _("Extensions are disabled."));
    sizer->Add(warning, DIALOGS_SIZER_OPTIONS);
    
    this->SetSizer(sizer);
}
