
#include "AppAccountList.h"

#include "../UIDefs.h"
#include "../../SeruroClient.h"
#include "../../SeruroConfig.h"

#include <wx/event.h>

/* Include image data. */
#include "../../resources/images/blank.png.h"
#include "../../resources/images/certificate_icon_12_flat.png.h"
#include "../../resources/images/identity_icon_12_flat.png.h"

/* Status (for accounts/identities) */
#include "../../resources/images/check_icon_12_flat.png.h"
#include "../../resources/images/cross_icon_12_flat.png.h"

#define ITEM_IMAGE_EXISTS 3
#define ITEM_IMAGE_NOT_EXISTS 4

#define APPACCOUNT_LIST_NAME_COLUMN 1
#define APPACCOUNT_LIST_APP_COLUMN  2

#define APP_ASSIGNED_TEXT ""
#define APP_PENDING_RESTART_TEXT "Requires Configuration"
#define APP_ALTERNATE_ASSIGNED_TEXT "Using alternate Identity"
#define APP_UNASSIGNED_TEXT "Not Configured"

DECLARE_APP(SeruroClient);

void AppAccountList::OnAccountStateChange(SeruroStateEvent &event)
{    
    if (event.GetAction() == STATE_ACTION_REMOVE) {
        /* Since accounts may not be listed by their address. */
        this->RemoveAccount(event.GetAccount());
    } else {
        /* For add/update. */
        this->GenerateAccountsList();
    }
    
    /* Do not skip, this is not part of the event binding chain. */
}

void AppAccountList::OnIdentityStateChange(SeruroStateEvent &event)
{
    wxListItem app_item, account_item;
    wxString display_name, server_uuid;
    bool pending_override;
    
    account_item.SetMask(wxLIST_MASK_TEXT);
    account_item.SetColumn(1);
    app_item.SetMask(wxLIST_MASK_TEXT);
    app_item.SetColumn(2);
    
    display_name = (use_address) ? event.GetAccount() : this->address_map[event.GetAccount()].AsString();

    for (int i = 0; i < this->accounts_list->GetItemCount(); i++) {
        account_item.SetId(i);
        app_item.SetId(i);
        
        if (! accounts_list->GetItem(account_item) || ! accounts_list->GetItem(app_item)) {
            continue;
        }
        
        if (display_name != account_item.GetText() || event.GetValue("app") != app_item.GetText()) {
            continue;
        }
        
        /* Allow the event to override a state-less identity status check. */
        pending_override = event.HasValue("assign_override");
        
        /* If found, the event may have a custom status, otherwise check and set. */
        if (event.HasValue("status")) {
            accounts_list->SetItem(i, 3, event.GetValue("status"));
            accounts_list->SetItemData(i, (long) APP_CUSTOM);
        } else {
            this->SetAccountStatus(i, event.GetValue("app"), event.GetAccount(), pending_override);
        } 
    }
    /* Do not skip, this is not part of the event binding chain. */
}

void AppAccountList::SetAccountStatus(long index, const wxString &app, const wxString &account, bool pending_override)
{
    wxString server_uuid;
    wxString server_name;
    account_status_t identity_status;
    
    identity_status = theSeruroApps::Get().IdentityStatus(app, account, server_uuid, this->is_initial);
    
	/* Since Apps which auto-configure do not maintain state of their 'once-configured' accounts. */
	if (identity_status == APP_PENDING_RESTART && pending_override) {
        identity_status = APP_ASSIGNED;
	}
    
    /* Save 'assigned' status for potentially stateless-app running responses (refresh list button). */
	if (identity_status == APP_ASSIGNED) {
		this->pending_list[app][account] = wxJSONValue((unsigned int) APP_ASSIGNED);
	}
    
    /* Do not give false "unstateful" information about pending restarts (refresh list button). */
    if (this->pending_list[app].HasMember(account) && this->pending_list[app][account].AsUInt() == APP_ASSIGNED) {
        identity_status = APP_ASSIGNED;
    }

    accounts_list->SetItemData(index, (long) identity_status);
    if (identity_status == APP_ASSIGNED) {
        /* Show which server the account is using an Identity from. */
        server_name = theSeruroConfig::Get().GetServerName(server_uuid);
        accounts_list->SetItem(index, 3, wxString::Format(_("Using %s"), server_name));
        accounts_list->SetItemImage(index, ITEM_IMAGE_EXISTS);
    } else if (identity_status == APP_UNASSIGNED) {
        accounts_list->SetItem(index, 3, APP_UNASSIGNED_TEXT);
        accounts_list->SetItemImage(index, ITEM_IMAGE_NOT_EXISTS);
    } else if (identity_status == APP_PENDING_RESTART) {
        accounts_list->SetItem(index, 3, APP_PENDING_RESTART_TEXT);
        accounts_list->SetItemImage(index, ITEM_IMAGE_NOT_EXISTS);
    } else {
        accounts_list->SetItem(index, 3, APP_ALTERNATE_ASSIGNED_TEXT);
        accounts_list->SetItemImage(index, ITEM_IMAGE_NOT_EXISTS);
    }
}

bool AppAccountList::HasAnyAssigned()
{
    /* Check for at least one assigned account. */
    for (int i = 0; i < accounts_list->GetItemCount(); ++i) {
        if (accounts_list->GetItemData(i) == (long) APP_ASSIGNED) {
            return true;
        }
    }
    return false;
}

bool AppAccountList::Assign()
{
    /* Find server for account/address, if duplicates are found display a selection dialog. */
	wxArrayString servers;
	wxArrayString accounts, server_accounts;
	wxString server_uuid;
    
    /* Note: an account might belong to an application that cannot be assigned (meaning it's auto-assigned).
     *   But if that application is running then an assign will cause the application to restart.
     *   So... should this still check for the assignment ability?
     */
    
    //if (! theSeruroApps::Get().CanAssign(this->app_name)) {
    //    /* The application used by the selected account cannot be assigned. */
    //    return false;
    //}
    
	/* Check each servers' accounts, if a matching account is found, add the server to server_accounts. */
	servers = theSeruroConfig::Get().GetServerList();
	for (size_t i = 0; i < servers.size(); i++) {
		accounts = theSeruroConfig::Get().GetAddressList(servers[i]);
		for (size_t j = 0; j < accounts.size(); j++) {
			if (accounts[j] == this->account) {
				server_accounts.Add(servers[i]);
			}
		}
	}
    
	if (server_accounts.size() == 0) {
		/* No server found, the account is not installed? */
		return false;
	} else if (server_accounts.size() > 1) {
		/* More than one server found, create a selection dialog to replace server. */
		/* Todo: Set server_uuid. */
		return false;
	} else {
		server_uuid = server_accounts[0];
	}
    
	if (! theSeruroApps::Get().AssignIdentity(this->app_name, server_uuid, this->account)) {
		/* Todo: Display error message. */
        return false;
	}
    
    return true;
}

bool AppAccountList::Unassign()
{
	/* Forget assigned state. */
	//this->pending_list[this->app_name][this->account] = wxJSONValue((unsigned int) APP_UNASSIGNED);

    return false;
}

void AppAccountList::Create(wxWindow *parent, bool use_address, bool initial)
{
    this->parent = parent;
    this->use_address = use_address;
    /* Is this an initial applications list, created during setup? */
    this->is_initial = initial;
    
    /* Create image list */
    list_images = new wxImageList(12, 12, true);
    list_images->Add(wxGetBitmapFromMemory(blank));
	list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));
    list_images->Add(wxGetBitmapFromMemory(check_icon_12_flat));
    list_images->Add(wxGetBitmapFromMemory(cross_icon_12_flat));

    /* Create accounts list. */
    accounts_list = new wxListCtrl(parent, APPACCOUNT_LIST_ID,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    accounts_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);

    /* Add Image column. */
	wxListItem image_column;
	image_column.SetId(0);
    image_column.SetImage(2);
	accounts_list->InsertColumn(0, image_column);
    accounts_list->SetColumnWidth(0, 24);
    
    /* Add columns for accounts list. */
    accounts_list->InsertColumn(1, (use_address) ? _("Account Address") : _("Account Name"),
        wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(2, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(3, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);

	/* Set up event handler bindings. */
	//wxGetApp().Bind(wxEVT_LIST_ITEM_SELECTED, &AppAccountList::OnSelect, this, APPACCOUNT_LIST_ID);
    wxGetApp().Bind(wxEVT_LIST_COL_BEGIN_DRAG, &AppAccountList::OnAccountColumnDrag, this, APPACCOUNT_LIST_ID);
    //wxGetApp().Bind(wxEVT_LIST_ITEM_DESELECTED, &AppAccountList::OnDeselect, this, APPACCOUNT_LIST_ID);
}

void AppAccountList::RemoveAccount(wxString account)
{
    //long item_index;
    wxListItem app_item, account_item;
    wxString display_name;

    account_item.SetMask(wxLIST_MASK_TEXT);
    account_item.SetColumn(APPACCOUNT_LIST_NAME_COLUMN);
    //app_item.SetMask(wxLIST_MASK_TEXT);
    //app_item.SetColumn(APPACCOUNT_LIST_APP_COLUMN);
    
    /* This could be displaying the app's canonical 'account name'. */
    display_name = (use_address) ? account : this->address_map[account].AsString();
    
    for (int i = 0; i < this->accounts_list->GetItemCount(); i++) {
        account_item.SetId(i);
        //app_item.SetId(i);
        
        //if (! accounts_list->GetItem(account_item) || ! accounts_list->GetItem(app_item)) {
        if (! accounts_list->GetItem(account_item)) {
            continue;
        }
        
        /* If both match, remove. */
        //if (account_item.GetText() == account && app_item.GetText() == app) {
        if (account_item.GetText() == account) {
            accounts_list->DeleteItem(i);
        }
    }
}

void AppAccountList::AddAccount(wxString app, wxString account)
{
    long item_index;
    //account_status_t identity_status;
    
    wxString display_name, server_uuid;
    
    /* Note: do not apply whitelist, it should have already been applied. */
    item_index = this->accounts_list->InsertItem(0, _(""), 0);
    
    /* Get the account status, and fill-in the uuid. */
    //identity_status = theSeruroApps::Get().IdentityStatus(app, account, server_uuid, this->is_initial);
    //this->accounts_list->SetItemData(item_index, (long) identity_status);
    
    /* Todo: set image based on status. */
    display_name = (this->use_address) ? account : theSeruroApps::Get().GetAccountName(app, account);
    if (! this->use_address) {
        /* Use this for lookups (address -> account translations). */
        this->address_map[account] = display_name;
    }
    
    this->accounts_list->SetItem(item_index, 1, display_name);
    this->accounts_list->SetItem(item_index, 2, app);
    
    /* Check if account exists, and if not, "disable the row". */
    if (! theSeruroConfig::Get().AddressExists(account)) {
        accounts_list->SetItemTextColour(item_index, wxColour(DISABLED_TEXT_COLOR));
    }
    
    this->SetAccountStatus(item_index, app, account);
}

void AppAccountList::GenerateAccountsList()
{
    wxArrayString apps;
    wxArrayString accounts;
    
    /* List of apps, then applied to whitelist, if any. */
    apps = theSeruroApps::Get().GetAppList(this->app_whitelist);
    
    /* List of all accounts under those apps, applied to whitelist. */
    accounts_list->DeleteAllItems();
    for (size_t i = 0; i < apps.size(); i++) {
        accounts = theSeruroApps::Get().GetAccountList(apps[i], this->account_whitelist);
        
        for (size_t j = 0; j < accounts.size(); j++) {
            this->AddAccount(apps[i], accounts[j]);
        }
    }
}

bool AppAccountList::SelectAccount(long index)
{
    wxListItem item;
    
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetId(index);
    item.SetColumn(APPACCOUNT_LIST_NAME_COLUMN);
    
    if (! accounts_list->GetItem(item)) {
        wxLogMessage(_("AppAccountList> (OnSelect) could not get address."));
        return false;
    }
    
    /* This item will be shown as disabled. */
    if (! theSeruroConfig::Get().AddressExists(item.GetText())) {
        accounts_list->SetItemState(index, 0, wxLIST_STATE_SELECTED);
        return false;
    }
    
    this->account = item.GetText();
    
    /* Also need the server name for this account. */
    item.SetColumn(APPACCOUNT_LIST_APP_COLUMN);
    accounts_list->GetItem(item);
    this->app_name = item.GetText();
    
    return true;
}

void AppAccountList::OnAccountColumnDrag(wxListEvent &event)
{
    /* This could be better defined, assumes image=0. */
    if (event.GetColumn() == 0) {
        event.Veto();
    }
}

void AppAccountList::DeselectAccounts()
{
    /* Todo: hopefully this does not account a deselect event. */
    wxListItem item;
    int accounts_count = accounts_list->GetItemCount();
    
    for (int i = 0; i < accounts_count; i++) {
        item.SetId(i);
        this->accounts_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void AppAccountList::AddAccountList(wxSizer *sizer)
{
    if (this->accounts_list != 0) {
        sizer->Add(accounts_list, DEFAULT_SIZER_OPTIONS.Proportion(1).Top().Bottom());
    }
}


