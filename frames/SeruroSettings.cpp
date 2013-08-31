
#include "SeruroSettings.h"
#include "../logging/SeruroLogger.h"

#include "UIDefs.h"
#include "settings/SettingsWindows.h"

#include <wx/log.h>
#include <wx/imaglist.h>

#include "../resources/images/general_icon_42_flat.png.h"
#include "../resources/images/accounts_icon_42_flat.png.h"
#include "../resources/images/applications_icon_42_flat.png.h"
#include "../resources/images/addons_icon_42_flat.png.h"
#include "../resources/images/log_icon_42_flat.png.h"

#define SETTINGS_MENU_WIDTH 200

BEGIN_EVENT_TABLE(SeruroPanelSettings, SeruroPanel)
	EVT_LIST_ITEM_SELECTED(SETTINGS_MENU_ID, SeruroPanelSettings::OnSelected)
END_EVENT_TABLE()

void SeruroPanelSettings::OnSelected(wxListEvent &event)
{
	/* Hide all windows, then show the selected. */
	this->Freeze();
	general_window->Hide();
	accounts_window->Hide();
	applications_window->Hide();
	extensions_window->Hide();
    if (SERURO_USE_SETTINGSLOG) {
        log_window->Hide();
    }
	if (event.GetItem() == 0) general_window->Show();
	if (event.GetItem() == 1) accounts_window->Show();
	if (event.GetItem() == 2) applications_window->Show();
	if (event.GetItem() == 3) extensions_window->Show();
    if (event.GetItem() == 4 && SERURO_USE_SETTINGSLOG) log_window->Show();
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
    //general_window->Hide();

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
    
    if (SERURO_USE_SETTINGSLOG) {
        log_window = new LogWindow(this);
        sizer->Add(log_window, 1, wxEXPAND | wxRIGHT | wxTOP | wxBOTTOM, 10);
        log_window->Hide();
    }
    
    /* Select the first item, General. */
    //menu->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

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
    if (SERURO_USE_SETTINGSLOG) {
        image_list->Add(wxGetBitmapFromMemory(log_icon_42_flat));
    }

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
    menu->SetItemState(0, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    
    menu->InsertItem(1, _("Accounts"), 1);
	menu->InsertItem(2, _("Applications"), 2);
	menu->InsertItem(3, _("Extensions"), 3);
    if (SERURO_USE_SETTINGSLOG) {
        menu->InsertItem(4, _("Log"), 4);
    }
    
	sizer->Add(menu, 0, wxEXPAND | wxALL, 10);
}
