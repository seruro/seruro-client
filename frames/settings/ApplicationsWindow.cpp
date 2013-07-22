
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../apps/SeruroApps.h"
#include "../../api/SeruroStateEvents.h"

#include <wx/log.h>
#include <wx/imaglist.h>

/* Include image data. */
#include "../../resources/images/blank.png.h"
#include "../../resources/images/certificate_icon_12_flat.png.h"
#include "../../resources/images/identity_icon_12_flat.png.h"

#define APP_ACCOUNTS_LIST_NAME_COLUMN 1
#define APP_ACCOUNTS_LIST_APP_COLUMN 2

BEGIN_EVENT_TABLE(ApplicationsWindow, SettingsView)
    EVT_LIST_ITEM_SELECTED(SETTINGS_APPS_LIST_ID, ApplicationsWindow::OnAppSelected)
    EVT_LIST_ITEM_SELECTED(SETTINGS_APP_ACCOUNTS_LIST_ID, ApplicationsWindow::OnAccountSelected)
    EVT_LIST_COL_BEGIN_DRAG(wxID_ANY, ApplicationsWindow::OnColumnDrag)
    EVT_LIST_ITEM_DESELECTED(wxID_ANY, ApplicationsWindow::OnDeselect)

    EVT_BUTTON(BUTTON_ASSIGN,   ApplicationsWindow::OnAssign)
    EVT_BUTTON(BUTTON_UNASSIGN, ApplicationsWindow::OnUnassign)
    EVT_BUTTON(BUTTON_REFRESH,  ApplicationsWindow::OnRefresh)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

void ApplicationsWindow::OnAccountStateChange(SeruroStateEvent &event)
{
	wxLogMessage(_("ApplicationsWindow> (OnAccountStateChange)"));
    
    this->GenerateAccountsList();    
	this->AlignLists();
    
	event.Skip();
}

void ApplicationsWindow::OnAssign(wxCommandEvent &event)
{
    
}

void ApplicationsWindow::OnUnassign(wxCommandEvent &event)
{
    
}

void ApplicationsWindow::OnRefresh(wxCommandEvent &event)
{
    
}

void ApplicationsWindow::OnAppSelected(wxListEvent &event)
{
    /* Must deselect all accounts. */
    DeselectAccounts();
    
    this->assign_button->Enable(false);
    this->unassign_button->Enable(false);
}

void ApplicationsWindow::OnAccountSelected(wxListEvent &event)
{
    wxListItem item;
    
    item.SetMask(wxLIST_MASK_TEXT);
    item.SetId(event.GetIndex());
    item.SetColumn(APP_ACCOUNTS_LIST_NAME_COLUMN);
    
    if (! accounts_list->GetItem(item)) {
        wxLogMessage(_("ApplicationsWindow> (OnAccountSelected) could not get address."));
        return;
    }
    
    /* Only one app or account can be selected at a time. */
    DeselectApps();
    
    this->account = item.GetText();
    //this->account_selected = true;
    
    /* Also need the server name for this account. */
    item.SetColumn(APP_ACCOUNTS_LIST_APP_COLUMN);
    accounts_list->GetItem(item);
    this->app_name = item.GetText();
    
    /* Check if identity is unstalled. */
    this->assign_button->Enable(true);
    this->unassign_button->Enable(true);
}

void ApplicationsWindow::DeselectApps()
{
    /* Todo: hopefully this does not cause a deselection event */
    wxListItem item;
    int apps_count = apps_list->GetItemCount();
    
    for (int i = 0; i < apps_count; i++) {
        item.SetId(i);
        apps_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void ApplicationsWindow::DeselectAccounts()
{
    /* Todo: hopefully this does not account a deselect event. */
    wxListItem item;
    int accounts_count = accounts_list->GetItemCount();
    
    for (int i = 0; i < accounts_count; i++) {
        item.SetId(i);
        accounts_list->SetItemState(item, 0, wxLIST_STATE_SELECTED);
    }
}

void ApplicationsWindow::DoDeselect()
{
    this->app_name = wxEmptyString;
    this->account = wxEmptyString;
    
    this->assign_button->Disable();
    this->unassign_button->Disable();
}


void ApplicationsWindow::GenerateApplicationsList()
{
    long item_index;
	wxArrayString apps;
    wxJSONValue app_info;
    
    apps = apps_helper->GetAppList();
    
    apps_list->DeleteAllItems();
	for (size_t i = 0; i < apps.size(); i++) {
        item_index = apps_list->InsertItem(0, _(""), 0);
		apps_list->SetItem(item_index, 1, _(apps[i]));
        
        /* Todo: set image based on status. */
        app_info = apps_helper->GetApp(apps[i]);
        apps_list->SetItem(item_index, 2, app_info["version"].AsString());
        apps_list->SetItem(item_index, 3, app_info["status"].AsString());
	}
}

void ApplicationsWindow::GenerateAccountsList()
{
    long item_index;
    bool identity_installed;
    wxArrayString apps;
    wxArrayString accounts;
    
    apps = apps_helper->GetAppList();
    
    accounts_list->DeleteAllItems();
    for (size_t i = 0; i < apps.size(); i++) {
        accounts = apps_helper->GetAccountList(apps[i]);
        
        for (size_t j = 0; j < accounts.size(); j++) {
            item_index = accounts_list->InsertItem(0, _(""), 0);
            accounts_list->SetItem(item_index, 1, _(accounts[j]));
            identity_installed = apps_helper->IsIdentityInstalled(apps[i], accounts[j]);
            
            /* Todo: set image based on status. */
            accounts_list->SetItem(item_index, 2, _(apps[i]));
            accounts_list->SetItem(item_index, 3, (identity_installed) ? _("Installed") : _("Not installed"));
        }
    }
}

ApplicationsWindow::ApplicationsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
    apps_helper = new SeruroApps();
 
    wxImageList *list_images = new wxImageList(12, 12, true);
    list_images->Add(wxGetBitmapFromMemory(blank));
	list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));
    
	apps_list = new wxListCtrl(this, SETTINGS_APPS_LIST_ID, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    apps_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);
 
    /* Add Image column. */
	wxListItem image_column;
	image_column.SetId(0);
	image_column.SetImage(1);
	apps_list->InsertColumn(0, image_column);
    
    /* Add columns for applications list. */
	apps_list->InsertColumn(1, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(2, _("Version"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(3, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    this->GenerateApplicationsList();
    apps_list->SetColumnWidth(0, 24);
    
    accounts_list = new wxListCtrl(this, SETTINGS_APP_ACCOUNTS_LIST_ID, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    accounts_list->SetImageList(list_images, wxIMAGE_LIST_SMALL);
    
    /* Add Image column. */
    image_column.SetImage(2);
	accounts_list->InsertColumn(0, image_column);
    
    /* Add columns for accounts list. */
    accounts_list->InsertColumn(1, _("Account Address"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(2, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(3, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    this->GenerateAccountsList();
    accounts_list->SetColumnWidth(0, 24);
    
	sizer->Add(apps_list, DIALOGS_SIZER_OPTIONS);
    sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());
    
    /* A sizer for ACTION buttons. */
	wxSizer *const actions_sizer = new wxBoxSizer(wxHORIZONTAL);
	assign_button = new wxButton(this, BUTTON_ASSIGN, _("Assign"));
	assign_button->Disable();
	unassign_button = new wxButton(this, BUTTON_UNASSIGN, _("Unassign"));
	unassign_button->Disable();
    wxButton *refresh_button = new wxButton(this, BUTTON_REFRESH, _("Refresh"));
    actions_sizer->Add(assign_button, DIALOGS_SIZER_OPTIONS);
    actions_sizer->Add(unassign_button, DIALOGS_SIZER_OPTIONS);
    actions_sizer->Add(refresh_button, DIALOGS_SIZER_OPTIONS);

    sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS.FixedMinSize().Bottom());
    
	/* Set up event handler bindings. */
	wxGetApp().Bind(SERURO_STATE_CHANGE, &ApplicationsWindow::OnAccountStateChange, this, STATE_TYPE_ACCOUNT);
    
    this->SetSizer(sizer);
    this->AlignLists();
}

ApplicationsWindow::~ApplicationsWindow()
{
    delete apps_helper;
}

void ApplicationsWindow::AlignLists()
{
	wxListCtrl *lists[] = {apps_list, accounts_list};
	MaximizeAndAlignLists(lists, 2, 1);
	//delete lists;
}


