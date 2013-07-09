
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

void ApplicationsWindow::GenerateApplicationsList()
{
    long item_index;
	wxArrayString apps;
    wxJSONValue app_info;
    
    apps = apps_helper.GetAppList();
	for (size_t i = 0; i < apps.size(); i++) {
		item_index = apps_list->InsertItem(0, _(apps[i]));
        
        /* Todo: set image based on status. */
        app_info = apps_helper.GetApp(apps[i]);
        apps_list->SetItem(item_index, 1, app_info["version"].AsString());
        apps_list->SetItem(item_index, 2, app_info["status"].AsString());
	}
}

void ApplicationsWindow::GenerateAccountsList()
{
    long item_index;
    bool identity_installed;
    wxArrayString apps;
    wxArrayString accounts;
    
    apps = apps_helper.GetAppList();
    for (size_t i = 0; i < apps.size(); i++) {
        accounts = apps_helper.GetAccountList(apps[i]);
        
        for (size_t j = 0; j < accounts.size(); j++) {
            item_index = accounts_list->InsertItem(0, _(accounts[j]));
            identity_installed = apps_helper.IsIdentityInstalled(apps[i], accounts[j]);
            
            /* Todo: set image based on status. */
            accounts_list->SetItem(item_index, 1, _(apps[i]));
            accounts_list->SetItem(item_index, 2, (identity_installed) ? _("Installed") : _("Not installed"));
        }
    }
}

ApplicationsWindow::ApplicationsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
	apps_list = new wxListCtrl(this, SETTINGS_APPS_LIST_ID,
        wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    
    /* Add columns for applications list. */
	apps_list->InsertColumn(0, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(1, _("Version"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    this->GenerateApplicationsList();
    
    accounts_list = new wxListCtrl(this, SETTINGS_APP_ACCOUNTS_LIST_ID, wxDefaultPosition, wxDefaultSize,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    
    /* Add columns for accounts list. */
    accounts_list->InsertColumn(0, _("Account Address"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(1, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    accounts_list->InsertColumn(2, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    this->GenerateAccountsList();
    

	sizer->Add(apps_list, DIALOGS_SIZER_OPTIONS);
    sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());
    
    this->SetSizer(sizer);
}




