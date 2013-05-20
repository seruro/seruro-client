
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"

#include <wx/button.h>
#include <wx/scrolwin.h>

SettingsPanel::SettingsPanel(SeruroPanelSettings *instance_panel) : 
	wxScrolledWindow(instance_panel->GetViewer(), wxID_ANY), 
	main_panel(instance_panel) {}

SettingsPanelView::SettingsPanelView(SeruroPanelSettings *instance_panel) :
	SettingsPanel(instance_panel)
{
	this->SetWindowStyle(wxBORDER_SIMPLE);
}

SettingsPanel_RootAccounts::SettingsPanel_RootAccounts(SeruroPanelSettings *parent) :
    SettingsPanelView(parent)
{
    wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    Text *msg = new Text(this, wxT("The Seruro Client can be configured to support multiple Seruro Servers."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);
    
    this->SetSizer(vert_sizer);
}

SettingsPanel_RootGeneral::SettingsPanel_RootGeneral(SeruroPanelSettings *parent) :	
	SettingsPanelView(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	//wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	//vert_sizer->Add(test_button, 0, wxRIGHT, 5);
    
    Text *msg = new Text(this, wxT("The Seruro Client has many optional features which can be controlled below."
        "Use the navigation tree on the left to add additional servers and accounts, check the status of servers and accounts,"
                                  "configuration application settings, and control Seruro extensions."));
    vert_sizer->Add(msg, SETTINGS_PANEL_SIZER_OPTIONS);

	this->SetSizer(vert_sizer);
}
