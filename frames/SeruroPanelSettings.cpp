
#include "SeruroPanelSettings.h"
#include "../api/SeruroServerAPI.h"
#include "../SeruroClient.h"

#include <wx/stattext.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/button.h>

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(SettingsPanelTree, wxTreeCtrl)
	EVT_TREE_SEL_CHANGED(SERURO_SETTINGS_TREE_ID, SettingsPanelTree::OnSelectItem)
END_EVENT_TABLE()

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

/* SETTINGS PANEL TREE */

void SettingsPanelTree::OnSelectItem(wxTreeEvent &event)
{
	wxTreeItemId item = event.GetItem();
	SettingsTreeItem *data = (SettingsTreeItem*) this->settings_tree->GetItemData(item);

	/* Based on the type in the data object, show the corresponding view, some of which
	 * require the item index to present the right view.
	 */
	switch (data->item_type) {
	case SETTINGS_VIEW_TYPE_SERVER: this->ShowView_Server(item); break;
	case SETTINGS_VIEW_TYPE_ADDRESS: this->ShowView_Address(item); break;
	case SETTINGS_VIEW_TYPE_APPLICATION: this->ShowView_Application(item); break;

	case SETTINGS_VIEW_TYPE_ROOT_GENERAL: this->ShowView_RootGeneral(); break;
	case SETTINGS_VIEW_TYPE_ROOT_SERVERS: this->ShowView_RootServers(); break;
	case SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS: this->ShowView_RootApplications(); break;
	}
}

void SettingsPanelTree::ShowView_Server(wxTreeItemId item)
{

}

void SettingsPanelTree::ShowView_Address(wxTreeItemId item)
{

}

void SettingsPanelTree::ShowView_Application(wxTreeItemId item)
{

}

void SettingsPanelTree::ShowView_RootGeneral()
{

}

void SettingsPanelTree::ShowView_RootServers()
{

}

void SettingsPanelTree::ShowView_RootApplications()
{

}

SettingsPanelTree::SettingsPanelTree(wxWindow *parent) : wxPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	/* Create a nice left-hand-side tree for all the setting views (use static ID for events). */
	this->settings_tree = new wxTreeCtrl(this, SERURO_SETTINGS_TREE_ID,
        wxDefaultPosition, wxDefaultSize, 
		wxTR_HIDE_ROOT | wxTR_TWIST_BUTTONS | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_LINES_AT_ROOT);
    this->settings_tree->SetIndent(12);
	/* We want a multi-root tree, so create a hidden tree root. */
	wxTreeItemId root = this->settings_tree->AddRoot(wxT("_"));

	/* The basic controls are called general. */
	wxTreeItemId root_general_item = this->settings_tree->AppendItem(root, wxT("General"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_GENERAL));

	/* List each server, which has settings, and each account within that server. */
	/* Todo: consider adding servers from this 'root', then adding accounts from each server. */
	wxTreeItemId root_servers_item = this->settings_tree->AppendItem(root, wxT("Accounts and Servers"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_SERVERS));
	wxArrayString servers_list = wxGetApp().config->GetServerList();
	wxArrayString addresses_list;
	wxTreeItemId server_item, address_item;
	for (size_t i = 0; i < servers_list.size(); i++) {
		server_item = this->settings_tree->AppendItem(root_servers_item, servers_list[i], -1, -1, 
			new SettingsTreeItem(SETTINGS_VIEW_TYPE_SERVER, servers_list[i]));
		addresses_list = wxGetApp().config->GetAddressList(servers_list[i]);
		for (size_t j = 0; j < addresses_list.size(); j++) {
			/* Add the address, note: in the settings-tree-item set the parent to the server. */
			address_item = this->settings_tree->AppendItem(server_item, addresses_list[j], -1, -1,
				new SettingsTreeItem(SETTINGS_VIEW_TYPE_ADDRESS, addresses_list[j], servers_list[i]));
		}
	}

	/* Applications include mail apps which may benefit from auto configuration. */
	wxTreeItemId root_applications_item = this->settings_tree->AppendItem(root, wxT("Applications"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS));

	wxTreeItemId app;
	app = this->settings_tree->AppendItem(root_applications_item, wxT("Microsoft Outlook"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_APPLICATION, wxT("Microsoft Outlook")));
	app = this->settings_tree->AppendItem(root_applications_item, wxT("Mozilla Thunderbird"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_APPLICATION, wxT("Mozilla Thunderbird")));

	/* Let the tree decide the best width? */
	this->settings_tree->SetQuickBestSize(false);

	vert_sizer->Add(this->settings_tree, 1, wxEXPAND | wxRIGHT, 0);
	this->SetSizer(vert_sizer);
}

SettingsPanel::SettingsPanel(wxWindow *parent) : wxPanel(parent, wxID_ANY)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Test"));

	vert_sizer->Add(test_button, 1, wxLEFT, 0);
	this->SetSizer(vert_sizer);
}

