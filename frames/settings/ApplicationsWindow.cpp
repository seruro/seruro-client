
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

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

