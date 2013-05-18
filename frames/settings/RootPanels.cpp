
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

SettingsPanel_RootGeneral::SettingsPanel_RootGeneral(SeruroPanelSettings *parent) :	
	SettingsPanelView(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	vert_sizer->Add(test_button, 0, wxRIGHT, 5);

	this->SetSizer(vert_sizer);
}
