
#include "SeruroPanelSettings.h"
#include "../api/SeruroServerAPI.h"

#include <wx/stattext.h>
#include <wx/treectrl.h>

/* Todo: catch some custom SeruroRequestEvents (error, timeout, data) */

SeruroPanelSettings::SeruroPanelSettings(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Settings"))
{
	/* Override default sizer. */
	wxBoxSizer *container_sizer = new wxBoxSizer(wxHORIZONTAL);
	/* Create a nice left-hand-side tree for all the setting views. */
	wxTreeCtrl *settings_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT);
	/* We want a multi-root tree, so create a hidden tree root. */
	wxTreeItemId root = settings_tree->AddRoot(wxT("_"));

	wxTreeItemId general_item = settings_tree->AppendItem(root, wxT("General"));
	wxTreeItemId servers_item = settings_tree->AppendItem(root, wxT("Accounts and Servers"));
	wxTreeItemId applications_item = settings_tree->AppendItem(root, wxT("Applications"));

	/* Let the tree decide the best width? */
	settings_tree->SetQuickBestSize(false);

	SettingsPanel *test_panel = new SettingsPanel(this);

	container_sizer->Add(settings_tree, 0, wxEXPAND, 5);
	container_sizer->Add(test_panel, 0, wxEXPAND, 5);

	this->SetSizer(container_sizer);
}

SettingsPanelTree::SettingsPanelTree(SeruroPanelSettings *parent) : SettingsPanel(parent)
{

}

SettingsPanel::SettingsPanel(SeruroPanelSettings *parent) : wxPanel(parent, wxID_ANY)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Test"));

	vert_sizer->Add(test_button, 0, wxEXPAND, 5);
	this->SetSizer(vert_sizer);
}