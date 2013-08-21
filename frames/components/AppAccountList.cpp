
#include "AppAccountList.h"

#include "../UIDefs.h"
#include "../../SeruroClient.h"

#include <wx/event.h>

/* Include image data. */
#include "../../resources/images/blank.png.h"
#include "../../resources/images/certificate_icon_12_flat.png.h"
#include "../../resources/images/identity_icon_12_flat.png.h"

#define APPACCOUNT_LIST_NAME_COLUMN 1
#define APPACCOUNT_LIST_APP_COLUMN  2

DECLARE_APP(SeruroClient);

AppAccountList::AppAccountList(wxWindow *parent, bool use_address)
  : parent(parent), use_address(use_address)
{
    //wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    this->created_appshelper = false;
    this->apps_helper = 0;
    
    /* Create image list */
    list_images = new wxImageList(12, 12, true);
    list_images->Add(wxGetBitmapFromMemory(blank));
	list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));

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
	wxGetApp().Bind(wxEVT_LIST_ITEM_SELECTED, &AppAccountList::OnSelect, this, APPACCOUNT_LIST_ID);
    wxGetApp().Bind(wxEVT_LIST_COL_BEGIN_DRAG, &AppAccountList::OnColumnDrag, this, APPACCOUNT_LIST_ID);
    wxGetApp().Bind(wxEVT_LIST_ITEM_DESELECTED, &AppAccountList::OnDeselect, this, APPACCOUNT_LIST_ID);
}

void AppAccountList::AddAccount(wxString app, wxString account)
{
    long item_index;
    account_status_t identity_status;
    
    wxString display_name, server_uuid;
    
    if (this->apps_helper == 0) {
        /* The caller must set or create the apps helper before generating accounts. */
        return;
    }
    
    /* Note: do not apply whitelist, it should have already been applied. */
    item_index = this->accounts_list->InsertItem(0, _(""), 0);
    
    /* Get the account status, and fill-in the uuid. */
    identity_status = apps_helper->IdentityStatus(app, account, server_uuid);
    this->accounts_list->SetItemData(item_index, (long) identity_status);
    
    /* Todo: set image based on status. */
    display_name = (this->use_address) ? account : this->apps_helper->GetAccountName(app, account);
    this->accounts_list->SetItem(item_index, 1, display_name);
    this->accounts_list->SetItem(item_index, 2, app);
    
    /* Check if account exists, and if not, "disable the row". */
    if (! wxGetApp().config->AddressExists(account)) {
        accounts_list->SetItemTextColour(item_index, wxColour(DISABLED_TEXT_COLOR));
    }
    
    /* Set the account status. */
    if (identity_status == APP_ASSIGNED) {
        accounts_list->SetItem(item_index, 3, wxGetApp().config->GetServerName(server_uuid));
    } else if (identity_status == APP_UNASSIGNED) {
        accounts_list->SetItem(item_index, 3, _("Unassigned"));
    } else {
        accounts_list->SetItem(item_index, 3, _("Using Unmanaged Identity"));
    }
}

void AppAccountList::GenerateAccountsList()
{
    wxArrayString apps;
    wxArrayString accounts;
    
    if (this->apps_helper == 0) {
        /* The caller must set or create the apps helper before generating accounts. */
        return;
    }
    
    /* List of apps, then applied to whitelist, if any. */
    apps = apps_helper->GetAppList(this->app_whitelist);
    
    /* List of all accounts under those apps, applied to whitelist. */
    accounts_list->DeleteAllItems();
    for (size_t i = 0; i < apps.size(); i++) {
        accounts = apps_helper->GetAccountList(apps[i], this->account_whitelist);
        
        for (size_t j = 0; j < accounts.size(); j++) {
            this->AddAccount(apps[i], accounts[j]);
        }
    }
}

void AppAccountList::OnSelect(wxListEvent &event)
{
    wxListItem item;
    
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetId(event.GetIndex());
    item.SetColumn(APPACCOUNT_LIST_NAME_COLUMN);
    
    if (! accounts_list->GetItem(item)) {
        wxLogMessage(_("AppAccountList> (OnSelect) could not get address."));
        return;
    }
    
    /* This item will be shown as disabled. */
    if (! wxGetApp().config->AddressExists(item.GetText())) {
        accounts_list->SetItemState(event.GetIndex(), 0, wxLIST_STATE_SELECTED);
        event.Veto();
        return;
    }
    
    /* Only one app or account can be selected at a time. */
    //DeselectApps();
    
    this->account = item.GetText();
    //this->account_selected = true;
    
    /* Also need the server name for this account. */
    item.SetColumn(APPACCOUNT_LIST_APP_COLUMN);
    accounts_list->GetItem(item);
    this->app_name = item.GetText();
    
    /* Check if identity is unstalled. */
    //assign_button->Enable(
    //    apps_helper->CanAssign(app_name) &&
    //    accounts_list->GetItemData(event.GetIndex()) != APP_ASSIGNED
    //);
    
    //unassign_button->Enable(
    //    apps_helper->CanUnassign(app_name) &&
    //    accounts_list->	GetItemData(event.GetIndex()) == APP_ASSIGNED
    //);
}

void AppAccountList::OnDeselect(wxListEvent &event)
{
    this->DoDeselect();
}

void AppAccountList::OnColumnDrag(wxListEvent &event)
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

void AppAccountList::DoDeselect()
{
    this->app_name = wxEmptyString;
    this->account = wxEmptyString;
    
    //this->assign_button->Disable();
    //this->unassign_button->Disable();
}



void AppAccountList::AddList(wxSizer *sizer)
{
    if (this->accounts_list != 0) {
        sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());
    }
}

void AppAccountList::OnAccountStateChange(SeruroStateEvent &event)
{
    /* Check if any accounts in the list need updating (account_whitelist). */
}



