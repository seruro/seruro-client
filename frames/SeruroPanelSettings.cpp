
#include "SeruroPanelSettings.h"
//#include "settings/SettingsPanels.h"
#include "UIDefs.h"

#include "../api/SeruroServerAPI.h"

//#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/log.h>

#include <wx/listctrl.h>
#include <wx/mstream.h>

/* Include image data. */
#include "../resources/images/glyphicons_270_shield.png.h"

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
	wxImageList *image_list = new wxImageList(22, 24, true);
	wxMemoryInputStream istream(glyphicons_270_shield_png, sizeof glyphicons_270_shield_png);
	wxImage shield_img(istream, wxBITMAP_TYPE_PNG);
	//shield_img.InitAlpha();
	//wxBitmap shield_img(wxImage(istream, wxBITMAP_TYPE_PNG, -1), -1);
	image_list->Add(wxBitmap(shield_img));
	//image_list->Add(shield_img);
	wxLogMessage(_("%d"), image_list->GetImageCount());


    wxSize s;
    s.SetWidth(SERURO_SETTINGS_TREE_MIN_WIDTH);
    wxListCtrl *t_menu = new wxListCtrl(this, SETTINGS_MENU_ID, wxDefaultPosition, s,
        wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_NO_HEADER | wxBORDER_THEME);
	t_menu->SetImageList(image_list, wxIMAGE_LIST_SMALL);
	wxColour blue_color(_("blue"));
	t_menu->SetBackgroundColour(blue_color);
    //t_menu->SetWindowStyleFlag(t_menu->GetWindowStyleFlag() | wxBORDER_SIMPLE);
    //t_menu->EnableAlternateRowColours(true);
    //wxSize m_size;// = GetClientSize();
    //m_size.SetWidth(SETTINGS_MENU_WIDTH);
    //m_size.SetHeight(SERURO_APP_DEFAULT_HEIGHT);
    //m_size.SetHeight(-1);
    //t_menu->SetSize(m_size);
    wxListItem column;
    column.SetText(_("HI"));
	column.SetImage(0);
    t_menu->InsertColumn(0, column);
	t_menu->SetColumnWidth(0, SERURO_SETTINGS_TREE_MIN_WIDTH);
	//t_menu->SetIma
    
	wxListItem item;
	item.SetId(0);
	item.SetColumn(0);
	item.SetImage(0);
	item.SetText(_("General"));
	//t_menu->SetItem(0, 0, _("General"), 0);
	//t_menu->SetItem(1, 0, _("Accounts"), 0);

	t_menu->InsertItem(0, _("General"));
	t_menu->InsertItem(1, _("Accounts"));
    //long item_index;
    //item_index = t_menu->InsertItem(0, _(" "));
    //t_menu->SetItem(item_index, 0, _("General"));
	//t_menu->SetItemImage(item_index, 0);

    //item_index = t_menu->InsertItem(0, _(" "));
    //t_menu->SetItem(item_index, 0, _("Accounts"));

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
