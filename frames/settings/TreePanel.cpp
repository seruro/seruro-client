
#include "SettingsPanels.h"

#include "../SeruroPanelSettings.h"
#include "../../SeruroClient.h"

/* Required for Configuration checking. */
DECLARE_APP(SeruroClient);

//enum settings_view_type settings_view_type_t;

BEGIN_EVENT_TABLE(SettingsTree, wxTreeCtrl)
	EVT_TREE_SEL_CHANGED(SERURO_SETTINGS_TREE_ID, SettingsTree::OnSelectItem)
END_EVENT_TABLE()

#if 0
/* For all text used within panels. */
BEGIN_EVENT_TABLE(Text, wxStaticText)
	EVT_SIZE(Text::OnSize)
END_EVENT_TABLE()
#endif

SettingsTree::SettingsTree(SettingsPanelTree *parent) 
	: wxTreeCtrl ((wxWindow *) parent, SERURO_SETTINGS_TREE_ID,
	wxDefaultPosition, wxDefaultSize,
	wxTR_HIDE_ROOT | wxTR_TWIST_BUTTONS | wxTR_HAS_BUTTONS | wxTR_NO_LINES | wxTR_LINES_AT_ROOT),
	main_panel(parent->MainPanel()) {}

void SettingsTree::OnSelectItem(wxTreeEvent &event)
{
	wxTreeItemId item = event.GetItem();
	SettingsTreeItem *data = (SettingsTreeItem*) this->GetItemData(item);

	wxLogMessage(wxT("SeruroSettingsPanel> Selected (type= %d) (parent= %s): %s."), 
		data->item_type, data->item_parent, data->item_name); 

	/* Using the item type check or create the view (initially, lazily) */
	switch (data->item_type) {
	case SETTINGS_VIEW_TYPE_SERVER: 
		//this->ShowView_Server(data);
        if (! this->main_panel->HasPanel(SETTINGS_VIEW_TYPE_SERVER, data->item_name)) {
            SettingsPanel_Server *server_panel = new SettingsPanel_Server(this->main_panel, data->item_name);
            this->main_panel->AddPanel(server_panel, SETTINGS_VIEW_TYPE_SERVER, data->item_name);
        }
		break;
	case SETTINGS_VIEW_TYPE_ADDRESS: 
		//this->ShowView_Address(data); 
		if (! this->main_panel->HasPanel(SETTINGS_VIEW_TYPE_ADDRESS, data->item_name,  data->item_parent)) {
			SettingsPanel_Address *address_panel = new SettingsPanel_Address(this->main_panel,  
				data->item_name, data->item_parent);
			this->main_panel->AddPanel(address_panel, SETTINGS_VIEW_TYPE_ADDRESS,
				data->item_name, data->item_parent);
		}
		break;
	case SETTINGS_VIEW_TYPE_APPLICATION: 
		//this->ShowView_Application(data); 
		break;

	case SETTINGS_VIEW_TYPE_ROOT_GENERAL: 
		//this->ShowView_RootGeneral(); 
		break;
	case SETTINGS_VIEW_TYPE_ROOT_ACCOUNTS:
		//this->ShowView_RootServers(); 
		break;
	case SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS: 
		//this->ShowView_RootApplications(); 
		break;
	}

	/* Finally show the requested view. */
	this->main_panel->ShowPanel(data->item_type, data->item_name, data->item_parent);
}

SettingsPanelTree::SettingsPanelTree(SeruroPanelSettings *parent) : 
	SettingsPanel(parent)
{
	wxBoxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);

	/* Create a nice left-hand-side tree for all the setting views (use static ID for events). */
	this->settings_tree = new SettingsTree(this);
    this->settings_tree->SetIndent(SERURO_SETTINGS_TREE_INDENT);

	/* We want a multi-root tree, so create a hidden tree root. */
	wxTreeItemId root = this->settings_tree->AddRoot(wxT("_"));

	/* The basic controls are called general. */
	wxTreeItemId root_general_item = this->settings_tree->AppendItem(root, wxT("General"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_GENERAL));

	/* List each server, which has settings, and each account within that server. */
	wxTreeItemId root_servers_item = this->settings_tree->AppendItem(root, wxT("Accounts and Servers"), -1, -1,
		new SettingsTreeItem(SETTINGS_VIEW_TYPE_ROOT_ACCOUNTS));
	
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

/* Wrap the text to the size of the parent. */
#if 0
void Text::OnSize(wxSizeEvent &event) {
	int new_width = parent->GetClientSize().x;
	int last_break = 0;
	bool reset_text = false;
		
	wxString previous_text;
	if (new_width < this->previous_width) {
		/* (If shrinking) Keep track of the string before the change occurs. */
			previous_text = this->GetLabelText();
	} else {
		while (this->breaks.size() > 0 && this->breaks.back() < new_width) {
			last_break = this->breaks.back();
			wxLogMessage(wxT("growing: %d is greater than last break: %d"), 
				new_width, last_break);
			this->breaks.pop_back();
			reset_text = true;
		}
	}

	this->Freeze();

	this->Wrap(parent->GetClientSize().x);
	wxLogMessage(wxT("compare: %d"), previous_text.compare(this->GetLabelText()));

	if (previous_text.compare(this->GetLabelText()) == 1) {
		/* A break was added. */
		wxLogMessage(wxT("broke at: %d"), new_width);
		this->breaks.push_back(new_width);
	} else if (reset_text) {
		/* The text has grown such that an inserted break should be removed. */
		this->SetLabelText(this->original_text);
	} else if (new_width > this->previous_width) {
		/* Final case: if multiple breaks we munged, check against original text. */
		wxLogMessage(wxT("og compare: %d"), this->original_text.compare(this->GetLabelText()));
		if (this->original_text.compare(this->GetLabelText()) == 1) {
			this->SetLabelText(this->original_text);
		}
	}

	this->Thaw();
	this->parent->Refresh();
	//}

	this->previous_width = new_width;
}
#endif
