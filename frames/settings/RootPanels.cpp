
#include "SettingsPanels.h"
#include "../SeruroPanelSettings.h"

SettingsPanel::SettingsPanel(SeruroPanelSettings *instance_panel)
	: wxPanel(instance_panel->GetViewer(), wxID_ANY), main_panel(instance_panel) {}

SettingsPanel_RootGeneral::SettingsPanel_RootGeneral(SeruroPanelSettings *parent) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	vert_sizer->Add(test_button, 0, wxRIGHT, 5);

	this->SetSizer(vert_sizer);
}
