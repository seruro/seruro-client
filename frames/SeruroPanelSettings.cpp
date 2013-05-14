
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
	this->AddFirstPanel();

	splitter->SplitVertically(settings_tree, this->current_panel, 1);
	/* Seed the current panel with the Root panel: general. */
	
	container_sizer->Add(this->splitter, 1, wxEXPAND | wxALL, 10);
	this->SetSizer(container_sizer);
}

/* To help with organization, perform the initialization of the first panel as it's own method.
 * In most cases, this is the 'General' root panel. 
 */
void SeruroPanelSettings::AddFirstPanel()
{
    /* The initial view, general settings, must be set as the current_panel as well as added
	 * to the list of 'instanciated' panels.
	 */
	SettingsPanel *root_general_panel = new SettingsPanel_RootGeneral(this);

	//wxString panel_name = wxT("root_general");
	this->AddPanel((int) root_general_panel, SETTINGS_VIEW_TYPE_ROOT_GENERAL);
	this->current_panel = root_general_panel;
}

wxWindow* SeruroPanelSettings::GetViewer()
{
	return (wxWindow*) this->splitter;
}

/* Return the existance of a multi-layered datum within the panels member. */
bool SeruroPanelSettings::HasPanel(settings_view_type_t type, 
	const wxString &name, const wxString &parent)
{
	/* If a parent is given. */
	if (parent.compare(wxEmptyString) != 0) {
		return (this->panels.HasMember(type) && 
			(this->panels[type].HasMember(parent) && this->panels[type][parent].HasMember(name)));
	}

	/* If only a name was given. */
	return (this->panels.HasMember(type) && this->panels[type].HasMember(name));
}

/* Record an int as name or parent/name. */
void SeruroPanelSettings::AddPanel(int panel_ptr, settings_view_type_t type,
	const wxString &name, const wxString &parent)
{
	wxJSONValue type_value;
	if (! this->panels.HasMember(type)) {
		/* See the JSON value with this type of panel. */
		this->panels[type] = type_value;
	}

	if (parent.compare(wxEmptyString) != 0) {
		if (! this->panels[type].HasMember(parent)) {
			wxJSONValue parent_value;
			this->panels[type][parent] = parent_value;
		}
		this->panels[type][parent][name] = panel_ptr;
		return;
	}

	this->panels[type][name] = panel_ptr;
	return;
}

/* This is where the magic hackery happens:
 *  (1) Cast an int as a SettingsPanel pointer.
 *  (2) Hope that the pointer is a valid panel.
 *  (3) Hold on to your butts.
 *  (4) Use the view splitter to swap the current shown panel with the newly 'found' panel.
 * Done.
 */
void SeruroPanelSettings::ShowPanel(settings_view_type_t type,
	const wxString &name, const wxString &parent)
{
	int panel_ptr;

	if (parent.compare(wxEmptyString) != 0) {
		if (! this->panels.HasMember(type) || 
			! this->panels[type].HasMember(parent) || ! this->panels[type][parent].HasMember(name)) {
			wxLogMessage(wxT("SeruroPanelSettings> Could not change views, unknown name or parent."));
			/* Calling ShowPanel without first calling AddPanel, very bad. */
			return;
		}
		panel_ptr = this->panels[type][parent][name].AsInt();
	} else {
		if (! this->panels.HasMember(type) || ! this->panels[type].HasMember(name)) {
			wxLogMessage(wxT("SeruroPanelSettings> Could not change views, unknown name."));
			/* Calling ShowPanel without first calling AddPanel, very bad. */
			return;
		}
		panel_ptr = this->panels[type][name].AsInt();
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

	wxLogMessage(wxT("SeruroSettingsPanel> Selected (type= %d) (parent= %s): %s."), 
		data->item_type, data->item_parent, data->item_name); 

	/* Using the item type check or create the view (initially, lazily) */
	switch (data->item_type) {
	case SETTINGS_VIEW_TYPE_SERVER: 
		//this->ShowView_Server(data);
		break;
	case SETTINGS_VIEW_TYPE_ADDRESS: 
		//this->ShowView_Address(data); 
		if (! this->main_panel->HasPanel(SETTINGS_VIEW_TYPE_ADDRESS, data->item_name,  data->item_parent)) {
			SettingsPanel_Address *address_panel = new SettingsPanel_Address(this->main_panel,  
				data->item_name, data->item_parent);
			this->main_panel->AddPanel((int) address_panel, SETTINGS_VIEW_TYPE_ADDRESS,
				data->item_name, data->item_parent);
		}
		break;
	case SETTINGS_VIEW_TYPE_APPLICATION: 
		//this->ShowView_Application(data); 
		break;

	case SETTINGS_VIEW_TYPE_ROOT_GENERAL: 
		//this->ShowView_RootGeneral(); 
		break;
	case SETTINGS_VIEW_TYPE_ROOT_SERVERS: 
		//this->ShowView_RootServers(); 
		break;
	case SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS: 
		//this->ShowView_RootApplications(); 
		break;
	}

	/* Finally show the requested view. */
	this->main_panel->ShowPanel(data->item_type, data->item_name, data->item_parent);
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

#if 0
	/* Applications include mail apps which may benefit from auto configuration. */
	wxTreeItemId root_applications_item = this->settings_tree->AppendItem(root, wxT("Applications"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS));

	wxTreeItemId app;
	app = this->settings_tree->AppendItem(root_applications_item, wxT("Microsoft Outlook"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_APPLICATION, wxT("Microsoft Outlook")));
	app = this->settings_tree->AppendItem(root_applications_item, wxT("Mozilla Thunderbird"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_APPLICATION, wxT("Mozilla Thunderbird")));
#endif 

	/* Let the tree decide the best width? */
	this->settings_tree->SetQuickBestSize(false);

	vert_sizer->Add(this->settings_tree, 1, wxEXPAND | wxRIGHT, 0);
	this->SetSizer(vert_sizer);
}

