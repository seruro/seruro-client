
#include "SeruroPanelSettings.h"
#include "../api/SeruroServerAPI.h"
#include "../SeruroClient.h"

#include <wx/stattext.h>
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
	this->splitter = new wxSplitterWindow(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH | wxSP_BORDER);
	this->splitter->SetSize(GetClientSize());
	this->splitter->SetSashGravity(1.0);
	this->splitter->SetMinimumPaneSize(SERURO_SETTINGS_MIN_WIDTH);

	/* Create a tree control as well as the first settings view (general). */
	SettingsPanelTree *settings_tree = new SettingsPanelTree(this);
	SettingsPanel *root_general_panel = new SettingsPanel_RootGeneral(this);

	splitter->SplitVertically(settings_tree, root_general_panel, 1);
	/* Seed the current panel with the Root panel: general. */
	this->current_panel = root_general_panel;

	container_sizer->Add(this->splitter, 1, wxEXPAND | wxALL, 10);
	this->SetSizer(container_sizer);
}

wxWindow* SeruroPanelSettings::GetViewer()
{
	return (wxWindow*) this->splitter;
}

/* Return the existance of a multi-layered datum within the panels member. */
bool SeruroPanelSettings::HasPanel(const wxString &name, const wxString &parent)
{
	/* If a parent is given. */
	if (parent.compare(wxEmptyString) != 0) {
		return (this->panels.HasMember(parent) && this->panels[parent].HasMember(name));
	}

	/* If only a name was given. */
	return (this->panels.HasMember(name));
}

/* Record an int as name or parent/name. */
void SeruroPanelSettings::AddPanel(int panel_ptr, const wxString &name, const wxString &parent)
{
	if (parent.compare(wxEmptyString) != 0) {
		if (! this->panels.HasMember(parent)) {
			wxJSONValue parent_value;
			this->panels[parent] = parent_value;
		}
		this->panels[parent][name] = panel_ptr;
		return;
	}

	this->panels[name] = panel_ptr;
	return;
}

/* This is where the magic hackery happens:
 *  (1) Cast an int as a SettingsPanel pointer.
 *  (2) Hope that the pointer is a valid panel.
 *  (3) Hold on to your butts.
 *  (4) Use the view splitter to swap the current shown panel with the newly 'found' panel.
 * Done.
 */
void SeruroPanelSettings::ShowPanel(const wxString &name, const wxString &parent)
{
	int panel_ptr;

	if (parent.compare(wxEmptyString) != 0) {
		if (! this->panels.HasMember(parent) || ! this->panels[parent].HasMember(name)) {
			wxLogMessage(wxT("SeruroPanelSettings> Could not change views, unknown name or parent."));
			/* Calling ShowPanel without first calling AddPanel, very bad. */
			return;
		}
		panel_ptr = this->panels[parent][name].AsInt();
	} else {
		if (! this->panels.HasMember(name)) {
			wxLogMessage(wxT("SeruroPanelSettings> Could not change views, unknown name."));
			/* Calling ShowPanel without first calling AddPanel, very bad. */
			return;
		}
		panel_ptr = this->panels[name].AsInt();
	}

	if (panel_ptr == 0) {
		/* Very odd state. */
		wxLogMessage(wxT("SeruroPanelSettings> Something is terribly wrong, got a NULL pointer from panels."));
		return;
	}

	SettingsPanel *new_panel((SettingsPanel*) panel_ptr);

	bool changed = this->splitter->ReplaceWindow(this->current_panel, new_panel);
	if (! changed) {
		wxLogMessage(wxT("SeruroPanelSettings> Something is terribly wrong, could not replace view."));
		return;
	}

	wxLogMessage(wxT("SeruroPanelSettings> View changed!"));
	this->current_panel->Show(false);
	new_panel->Show(true);
	this->current_panel = new_panel;
}

/* SETTINGS PANEL(S) VIEWS */

SettingsPanel_RootGeneral::SettingsPanel_RootGeneral(SeruroPanelSettings *parent) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton *test_button = new wxButton(this, wxID_ANY, wxT("Root General Button"));
	vert_sizer->Add(test_button, 0, wxRIGHT, 5);

	this->SetSizer(vert_sizer);
}

SettingsPanel_Address::SettingsPanel_Address(SeruroPanelSettings *parent,
	const wxString &address, const wxString &server) : SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	wxButton *address_button = new wxButton(this, wxID_ANY, address);
	wxButton *server_button = new wxButton(this, wxID_ANY, server);
	vert_sizer->Add(address_button, 0, wxRIGHT, 5);
	vert_sizer->Add(server_button, 0, wxRIGHT, 5);

	this->SetSizer(vert_sizer);
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
	case SETTINGS_VIEW_TYPE_SERVER: this->ShowView_Server(data); break;
	case SETTINGS_VIEW_TYPE_ADDRESS: this->ShowView_Address(data); break;
	case SETTINGS_VIEW_TYPE_APPLICATION: this->ShowView_Application(data); break;

	case SETTINGS_VIEW_TYPE_ROOT_GENERAL: this->ShowView_RootGeneral(); break;
	case SETTINGS_VIEW_TYPE_ROOT_SERVERS: this->ShowView_RootServers(); break;
	case SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS: this->ShowView_RootApplications(); break;
	}
}

void SettingsPanelTree::ShowView_Server(const SettingsTreeItem *data)
{

}

void SettingsPanelTree::ShowView_Address(const SettingsTreeItem *data)
{
	wxString server = data->item_parent;
	wxString address = data->item_name;

	wxLogMessage(wxT("SeruroSettingsPanel> Selected (server= %s): %s address."), server, address); 

	if (! this->main_panel->HasPanel(address, server)) {
		SettingsPanel_Address *address_panel = new SettingsPanel_Address(this->main_panel, address, server);
		this->main_panel->AddPanel((int) address_panel, address, server);
		wxLogMessage(wxT("SeruroSettingsPanel> Created address panel (server= %s) (%s)."), server, address);
	}

	this->main_panel->ShowPanel(address, server);
}

void SettingsPanelTree::ShowView_Application(const SettingsTreeItem *data)
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
	//wxJSONValue panel;
	//panel["string"].As
}

SettingsPanelTree::SettingsPanelTree(SeruroPanelSettings *parent) : SettingsPanel(parent)
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

