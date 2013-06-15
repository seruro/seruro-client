
#include "SeruroPanelSettings.h"
//#include "settings/SettingsPanels.h"
#include "UIDefs.h"

#include "../api/SeruroServerAPI.h"

//#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/log.h>

#include <wx/listctrl.h>

enum {
    SETTINGS_MENU_ID
};

#define SETTINGS_MENU_WIDTH 200


SeruroPanelSettings::SeruroPanelSettings(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Settings"))
{
	/* Override default sizer. */
	wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    //splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    //    wxSP_3DSASH | wxSP_BORDER);
    /* First add the menu, followed by the views. */
    //this->AddMenu(container_sizer);
    //menu_window = new MenuWindow(this);
    //general_window = new GeneralWindow(this);
    
    //splitter->SplitVertically(menu_window, general_window);
    //splitter->SetSize(GetClientSize());
    //splitter->SetMinimumPaneSize(SETTINGS_MENU_WIDTH);
    //container_sizer->Add(splitter, 1, wxEXPAND | wxALL, 10);
	
    wxSize s;
    s.SetWidth(200);
    wxListCtrl *t_menu = new wxListCtrl(this, SETTINGS_MENU_ID, wxDefaultPosition, s,
        wxLC_ICON | wxLC_SINGLE_SEL | wxLC_NO_HEADER | wxBORDER_SIMPLE);
    //t_menu->SetWindowStyleFlag(t_menu->GetWindowStyleFlag() | wxBORDER_SIMPLE);
    //t_menu->EnableAlternateRowColours(true);
    //wxSize m_size;// = GetClientSize();
    //m_size.SetWidth(SETTINGS_MENU_WIDTH);
    //m_size.SetHeight(SERURO_APP_DEFAULT_HEIGHT);
    //m_size.SetHeight(-1);
    //t_menu->SetSize(m_size);
    wxListItem column;
    column.SetText(_(" "));
    t_menu->InsertColumn(1, column);
    
    long item_index;
    item_index = t_menu->InsertItem(0, _(" "));
    t_menu->SetItem(item_index, 1, _("General"));
    item_index = t_menu->InsertItem(0, _("ACCOUNTS"));
    t_menu->SetItem(item_index, 1, _("Accounts"));
    sizer->Add(t_menu, 0, wxEXPAND | wxALL, 10);
    
    GeneralWindow *general = new GeneralWindow(this);
    sizer->Add(general, 1, wxEXPAND | wxALL, 10);
    general->Hide();
    
    AccountsWindow *accounts = new AccountsWindow(this);
    sizer->Add(accounts, 1, wxEXPAND | wxALL, 10);
    //accounts->Hide();
    
    this->SetSizer(sizer);
    //container_sizer->SetSizerHints(this);
}
    
     
MenuWindow::MenuWindow(SeruroPanelSettings *window) : parent(window)
{
    /* Create the list control menu. */
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
    menu = new wxListCtrl(this, SETTINGS_MENU_ID, wxDefaultPosition, wxDefaultSize, wxLC_ICON);
    
    wxSize menu_size = menu->GetSize();
    menu_size.SetWidth(SETTINGS_MENU_WIDTH);
    menu->SetSize(menu_size);
    
    //wxListItem menu_item;
    //menu_item.SetText(_("General"));
    //menu->InsertItem(0, _("General"));
    
    //sizer->Add(menu, 1, wxALL | wxEXPAND, 5);
    this->SetSizer(sizer);
}

GeneralWindow::GeneralWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *button = new wxButton(this, wxID_ANY, _("Click me"));
    sizer->Add(button, DIALOGS_SIZER_OPTIONS);
    
    this->SetSizer(sizer);
}

AccountsWindow::AccountsWindow(SeruroPanelSettings *window) : SettingsView(window)
{
    wxSizer *const sizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxButton *button = new wxButton(this, wxID_ANY, _("Click you"));
    sizer->Add(button, DIALOGS_SIZER_OPTIONS);
    
    this->SetSizer(sizer);
}
