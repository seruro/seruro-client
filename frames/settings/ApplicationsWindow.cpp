
#include "SettingsWindows.h"
#include "../SeruroSettings.h"
#include "../UIDefs.h"

#include "../../SeruroClient.h"
#include "../../api/SeruroStateEvents.h"

#include <wx/log.h>

/* Include image data. */
#include "../../resources/images/blank.png.h"
#include "../../resources/images/certificate_icon_12_flat.png.h"
#include "../../resources/images/identity_icon_12_flat.png.h"

//#define APP_ACCOUNTS_LIST_NAME_COLUMN 1
//#define APP_ACCOUNTS_LIST_APP_COLUMN 2

BEGIN_EVENT_TABLE(ApplicationsWindow, SettingsView)
    EVT_LIST_ITEM_SELECTED(SETTINGS_APPS_LIST_ID, ApplicationsWindow::OnAppSelected)
    EVT_LIST_ITEM_SELECTED(APPACCOUNT_LIST_ID, ApplicationsWindow::OnAccountSelected)

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
    
    /* This might cause some headache, but it's the easiest at the moment. */
    AppAccountList::DeselectAccounts();
    
    AppAccountList::GenerateAccountsList();
	this->AlignLists();
    
	event.Skip();
}

void ApplicationsWindow::OnApplicationStateChange(SeruroStateEvent &event)
{
    event.Skip();
    
    /* Refresh applications. */
    if (event.GetAction() == STATE_ACTION_ADD || event.GetAction() == STATE_ACTION_REMOVE) {
        this->GenerateApplicationsList();
    }
    
    AppAccountList::GenerateAccountsList();
    this->AlignLists();
}

void ApplicationsWindow::OnIdentityStateChange(SeruroStateEvent &event)
{
    AppAccountList::OnIdentityStateChange(event);
    this->AlignLists();
    
    /* Check if the current selected app/name has changed, and update buttons. */
    event.Skip();
}

void ApplicationsWindow::OnAssign(wxCommandEvent &event)
{
    AppAccountList::Assign();
}

void ApplicationsWindow::OnUnassign(wxCommandEvent &event)
{
    AppAccountList::Unassign();
}

void ApplicationsWindow::OnRefresh(wxCommandEvent &event)
{
    /* Refresh applications list. */
    this->GenerateApplicationsList();
    
    /* Refresh accounts list. */
    AppAccountList::GenerateAccountsList();
    this->AlignLists();
}

void ApplicationsWindow::OnAppSelected(wxListEvent &event)
{
    /* Must deselect all accounts. */
    this->DeselectAccounts();
    
    this->assign_button->Enable(false);
    this->unassign_button->Enable(false);
}

void ApplicationsWindow::OnAccountSelected(wxListEvent &event)
{
    long index = event.GetIndex();
    
    if (! this->SelectAccount(index)) {
        event.Veto();
        return;
    }
    
    /* Quick-looked of which type is selected. */
    this->DeselectApps();
    this->account_selected = true;
    
    /* Check if identity is unstalled. */
    assign_button->Enable(
        theSeruroApps::Get().CanAssign(this->app_name) &&
        accounts_list->GetItemData(index) != APP_ASSIGNED
    );
    
    unassign_button->Enable(
        theSeruroApps::Get().CanUnassign(this->app_name) &&
        accounts_list->	GetItemData(index) == APP_ASSIGNED
    );
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
    
    apps = theSeruroApps::Get().GetAppList();
    
    apps_list->DeleteAllItems();
	for (size_t i = 0; i < apps.size(); i++) {
        item_index = apps_list->InsertItem(0, _(""), 0);
		apps_list->SetItem(item_index, 1, _(apps[i]));
        
        /* Todo: set image based on status. */
        app_info = theSeruroApps::Get().GetApp(apps[i]);
        apps_list->SetItem(item_index, 2, app_info["version"].AsString());
        apps_list->SetItem(item_index, 3, app_info["status"].AsString());
	}
}

ApplicationsWindow::ApplicationsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
    //AppAccountList::AppAccountList(this, false);
    AppAccountList::Create(this, true);
    //AppAccountList::CreateHelper();
 
    apps_list_images = new wxImageList(12, 12, true);
    apps_list_images->Add(wxGetBitmapFromMemory(blank));
	apps_list_images->Add(wxGetBitmapFromMemory(certificate_icon_12_flat));
	apps_list_images->Add(wxGetBitmapFromMemory(identity_icon_12_flat));
    
	apps_list = new wxListCtrl(this, SETTINGS_APPS_LIST_ID,
        wxDefaultPosition, wxSize(-1, SETTINGS_APPLICATION_LIST_HEIGHT), wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_THEME);
    apps_list->SetImageList(apps_list_images, wxIMAGE_LIST_SMALL);
 
    /* Add Image column. */
	wxListItem image_column;
	image_column.SetId(0);
	image_column.SetImage(1);
	apps_list->InsertColumn(0, image_column);
    apps_list->SetColumnWidth(0, 24);
    
    /* Add columns for applications list. */
	apps_list->InsertColumn(1, _("Application"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(2, _("Version"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
	apps_list->InsertColumn(3, _("Status"), wxLIST_FORMAT_LEFT, wxLIST_AUTOSIZE_USEHEADER);
    
    this->GenerateApplicationsList();
    sizer->Add(apps_list, DIALOGS_SIZER_OPTIONS);
    
    /* Use the AppAccounts controller. */
    AppAccountList::GenerateAccountsList();
    AppAccountList::AddAccountList(sizer);
    
	//sizer->Add(apps_list, DIALOGS_SIZER_OPTIONS);
    //sizer->Add(accounts_list, DIALOGS_SIZER_OPTIONS.Proportion(1).Top().Bottom());
    
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
    wxGetApp().Bind(SERURO_STATE_CHANGE, &ApplicationsWindow::OnIdentityStateChange, this, STATE_TYPE_IDENTITY);
    wxGetApp().Bind(SERURO_STATE_CHANGE, &ApplicationsWindow::OnApplicationStateChange, this, STATE_TYPE_APPLICATION);
    
    this->SetSizer(sizer);
    this->AlignLists();
}

ApplicationsWindow::~ApplicationsWindow()
{
    //delete apps_helper;
}

void ApplicationsWindow::AlignLists()
{
	wxListCtrl *lists[] = {apps_list, accounts_list};
	MaximizeAndAlignLists(lists, 2, 1);
	//delete lists;
}


