
#include "../dialogs/RemoveDialog.h"
#include "../../setup/SeruroSetup.h"
#include "../../crypto/SeruroCrypto.h"
#include "../../api/SeruroStateEvents.h"
#include "../../api/SeruroServerAPI.h"
#include "../../SeruroClient.h"

#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

#include <wx/log.h>

/* Include image data. */
#include "../../resources/images/blank.png.h"
#include "../../resources/images/certificate_icon_12_flat.png.h"
#include "../../resources/images/identity_icon_12_flat.png.h"

/* Set these to help search for strings on selection. */
#define SERVERS_LIST_NAME_COLUMN 1
#define ACCOUNTS_LIST_NAME_COLUMN 1
#define ACCOUNTS_LIST_SERVER_COLUMN 2

BEGIN_EVENT_TABLE(AccountsWindow, SettingsView)
	EVT_LIST_ITEM_SELECTED(SETTINGS_SERVERS_LIST_ID, AccountsWindow::OnServerSelected)
	EVT_LIST_ITEM_SELECTED(SETTINGS_ACCOUNTS_LIST_ID, AccountsWindow::OnAccountSelected)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, AccountsWindow::OnDeselect)

    EVT_SERURO_REQUEST(SERURO_API_CALLBACK_CA, AccountsWindow::OnCAResult)

	/* When components / OS actions change seruro data. */
	/* Defined using a dynamic bind. */
	//EVT_SERURO_STATE(STATE_TYPE_SERVER, AccountsWindow::OnServerStateChange)

	EVT_BUTTON(BUTTON_ADD_SERVER, AccountsWindow::OnAddServer)
	EVT_BUTTON(BUTTON_ADD_ACCOUNT, AccountsWindow::OnAddAccount)
	EVT_BUTTON(BUTTON_UPDATE, AccountsWindow::OnUpdate)
	EVT_BUTTON(BUTTON_REMOVE, AccountsWindow::OnRemove)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void AccountsWindow::OnServerStateChange(SeruroStateEvent &event)
{
    wxLogMessage(_("AccountsWindow> (OnServerStateChange)"));
    wxListItem item_server;

    /* Set constant mask and column. */
    item_server.SetMask(wxLIST_MASK_TEXT);
    item_server.SetColumn(SERVERS_LIST_NAME_COLUMN);
    
    this->GenerateServersList();
	if (event.GetAction() == STATE_ACTION_REMOVE) {
		wxLogMessage(_("AccountsWindow> (OnServerStateChange) removing server (%s)."), event.GetServerName());
        
        /* The accounts list will change when a server is removed, without an explicit event. */
        this->GenerateAccountsList();
		this->DeselectServers();
		this->DoDeselect();
	}

	this->AlignLists();
    /* Allow other handlers. */
	event.Skip();
}

void AccountsWindow::OnAccountStateChange(SeruroStateEvent &event)
{
	wxLogMessage(_("AccountsWindow> (OnAccountStateChange)"));
    
    this->GenerateAccountsList();
	if (event.GetAction() == STATE_ACTION_REMOVE) {
		this->DeselectAccounts();
		this->DoDeselect();
	}

	this->AlignLists();

	event.Skip();
}

void AccountsWindow::OnServerSelected(wxListEvent &event)
{
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
        item.SetId(i);
        accounts_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void AccountsWindow::DoDeselect()
{
    this->server_name = wxEmptyString;
    this->address = wxEmptyString;
    
    this->update_button->Disable();
    this->remove_button->Disable();
}

void AccountsWindow::OnCAResult(SeruroRequestEvent &event)
{
	wxJSONValue response = event.GetResponse();

	/* Simple, there's no UI state to update based on the result. */
	SeruroServerAPI *api = new SeruroServerAPI(this->GetEventHandler());
	api->InstallCA(response);
    
    /* Create (potential) updated server event. */
    SeruroStateEvent state_event(STATE_TYPE_SERVER, STATE_ACTION_UPDATE);
    state_event.SetServerName(response["server_name"].AsString());
    this->ProcessWindowEvent(state_event);
    
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
	SeruroSetup identity_setup((wxFrame*) (wxGetApp().GetFrame()), SERURO_SETUP_IDENTITY,
        this->server_name, this->address);
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

void AccountsWindow::GenerateServersList()
{
	SeruroCrypto crypto;
	//bool cert_installed = false;
    long item_index;
	wxArrayString servers = wxGetApp().config->GetServerList();
    
    servers_list->DeleteAllItems();
	for (size_t i = 0; i < servers.size(); i++) {
		/* Check if the server certificate is installed. */
		item_index = servers_list->InsertItem(0, _(""), (crypto.HaveCA(servers[i])) ? 1 : 0);
		servers_list->SetItem(item_index, 1, _(servers[i]));
		servers_list->SetItem(item_index, 2, _("0"));
	}
}

void AccountsWindow::GenerateAccountsList()
{
    SeruroCrypto crypto;
    long item_index;
	wxArrayString accounts;
    wxArrayString servers = wxGetApp().config->GetServerList();
    
    accounts_list->DeleteAllItems();
	for (size_t i = 0; i < servers.size(); i++) {
		accounts = wxGetApp().config->GetAddressList(servers[i]);
		for (size_t j = 0; j < accounts.size(); j++) {
			//cert_installed = crypto_helper.HaveIdentity(servers[i],
			item_index = accounts_list->InsertItem(0, _(""),
                /* Display a key if the identity (cert/key) are installed. */
                (crypto.HaveIdentity(servers[i], accounts[j])) ? 2 : 0);
			accounts_list->SetItem(item_index, 1, accounts[j]);
			accounts_list->SetItem(item_index, 2, servers[i]);
			accounts_list->SetItem(item_index, 3, _("Never"));
		}
	}
}

void AccountsWindow::AlignLists()
{
	wxListCtrl *lists[] = {servers_list, accounts_list};
	MaximizeAndAlignLists(lists, 2, 1);
	//delete lists;
}

AccountsWindow::AccountsWindow(SeruroPanelSettings *window) : SettingsView(window),
	account_selected(false)
{
	/* This will hold the column of lists. */
	wxSizer *const lists_sizer = new wxBoxSizer(wxVERTICAL);

	wxImageList *list_images = new wxImageList(12, 12, true);
    list_images->Add(wxGetBitmapFromMemory(blank));
	list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));

	servers_list = new wxListCtrl(this, SETTINGS_SERVERS_LIST_ID,
		wxDefaultPosition, wxDefaultSize, 
		wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
	servers_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);

	/* Server list columns. */
	wxListItem image_column;
	image_column.SetId(0);
	image_column.SetImage(1);
	servers_list->InsertColumn(0, image_column);

	servers_list->InsertColumn(1, _("Server Name"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	servers_list->InsertColumn(2, _("Expires"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

    this->GenerateServersList();

	/* Allow the server name column to take up the remaining space. */
	servers_list->SetColumnWidth(0, 24);
	if (servers_list->GetItemCount() > 0) {
		servers_list->SetColumnWidth(SERVERS_LIST_NAME_COLUMN, wxLIST_AUTOSIZE);
	}

	lists_sizer->Add(servers_list, DIALOGS_SIZER_OPTIONS);

	accounts_list = new wxListCtrl(this, SETTINGS_ACCOUNTS_LIST_ID,
		wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
	accounts_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);

	/* Column for an icon? */
	image_column.SetImage(2);
	accounts_list->InsertColumn(0, image_column);
	accounts_list->InsertColumn(1, _("Address"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	accounts_list->InsertColumn(2, _("Server"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	accounts_list->InsertColumn(3, _("Expires"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

    this->GenerateAccountsList();

	/* Allow the account/address name column to take up the remaining space. */
	accounts_list->SetColumnWidth(0, 24);
	if (accounts_list->GetItemCount() > 0) {
		/* Size the server first, then allow the account name to override. */
		accounts_list->SetColumnWidth(ACCOUNTS_LIST_SERVER_COLUMN, wxLIST_AUTOSIZE);
		accounts_list->SetColumnWidth(ACCOUNTS_LIST_NAME_COLUMN, wxLIST_AUTOSIZE);
	}

	lists_sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());

	/* A sizer for ACTION buttons. */
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

	lists_sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS.FixedMinSize().Bottom());

	/* Set up event handler bindings. */
	wxGetApp().Bind(SERURO_STATE_CHANGE, &AccountsWindow::OnServerStateChange, this, STATE_TYPE_SERVER);
	wxGetApp().Bind(SERURO_STATE_CHANGE, &AccountsWindow::OnAccountStateChange, this, STATE_TYPE_ACCOUNT);

	this->SetSizer(lists_sizer);

	this->AlignLists();
}

