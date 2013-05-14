
#include "SeruroPanelSettings.h"
#include "../api/SeruroServerAPI.h"

#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/button.h>

/* Todo: catch some custom SeruroRequestEvents (error, timeout, data) */

SeruroPanelSettings::SeruroPanelSettings(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Settings"))
{
	/* Override default sizer. */
	wxBoxSizer *container_sizer = new wxBoxSizer(wxHORIZONTAL);

	//container_sizer->Add(settings_tree, 0, wxEXPAND, 5);
	//container_sizer->Add(test_panel, 0, wxEXPAND, 5);

	/* Create a resizeable window for the navigation pane (panel) and it's controlling view. */
	wxSplitterWindow *splitter = new wxSplitterWindow(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH | wxSP_BORDER);
	splitter->SetSize(GetClientSize());
	splitter->SetSashGravity(1.0);
	splitter->SetMinimumPaneSize(SERURO_SETTINGS_MIN_WIDTH);

	/* Create a tree control as well as the first settings view (general). */
	SettingsPanelTree *settings_tree = new SettingsPanelTree(splitter);
	SettingsPanel *test_panel = new SettingsPanel(splitter);

	splitter->SplitVertically(settings_tree, test_panel, 1);

	container_sizer->Add(splitter, 1, wxEXPAND | wxALL, 10);
	this->SetSizer(container_sizer);
}

SettingsPanelTree::SettingsPanelTree(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	/* Create a nice left-hand-side tree for all the setting views. */
	wxTreeCtrl *settings_tree = new wxTreeCtrl(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxTR_HIDE_ROOT | wxTR_TWIST_BUTTONS | wxTR_HAS_BUTTONS | wxTR_NO_LINES);
    settings_tree->SetIndent(5);
	/* We want a multi-root tree, so create a hidden tree root. */
	wxTreeItemId root = settings_tree->AddRoot(wxT("_"));

	wxTreeItemId general_item = settings_tree->AppendItem(root, wxT("General"));
	wxTreeItemId servers_item = settings_tree->AppendItem(root, wxT("Accounts and Servers"));
	wxTreeItemId applications_item = settings_tree->AppendItem(root, wxT("Applications"));

	wxTreeItemId server1, server2;
	server1 = settings_tree->AppendItem(servers_item, wxT("Open Seruro"));
	server2 = settings_tree->AppendItem(servers_item, wxT("Seruro Test Server 1"));

	wxTreeItemId account;
	account = settings_tree->AppendItem(server1, wxT("ted@valdrea.com"));
	account = settings_tree->AppendItem(server1, wxT("teddy.reed@gmail.com"));
	account = settings_tree->AppendItem(server2, wxT("teddy@prosauce.org"));

	wxTreeItemId app;
	app = settings_tree->AppendItem(applications_item, wxT("Microsoft Outlook"));
	app = settings_tree->AppendItem(applications_item, wxT("Mozilla Thunderbird"));

	/* Let the tree decide the best width? */
	settings_tree->SetQuickBestSize(false);

	vert_sizer->Add(settings_tree, 1, wxEXPAND | wxRIGHT, 0);
	this->SetSizer(vert_sizer);
}

SettingsPanel::SettingsPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Test"));

	vert_sizer->Add(test_button, 1, wxLEFT, 0);
	this->SetSizer(vert_sizer);
}

