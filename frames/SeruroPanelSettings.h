
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

#include <wx/treebase.h>

#define SERURO_SETTINGS_MIN_WIDTH 150
/* The tree event control id. */
#define SERURO_SETTINGS_TREE_ID 1009

// Define a new frame type: this is going to be our main frame
class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);
};

/* A default panel (view) for settings. */
class SettingsPanel : public wxPanel
{
public:
	SettingsPanel(wxWindow *parent);
};

/* Show a tree-view for selecting various parts of the settings. */
class SettingsPanelTree : public wxPanel
{
public:
	SettingsPanelTree(wxWindow *parent);

	/* Event handler for selecting a panel item. */
	void OnSelectItem(wxTreeEvent &event);

private:
	wxTreeCtrl *settings_tree;

	DECLARE_EVENT_TABLE()
};

/* Define the various types/classes of views in the settings. */
enum settings_view_type_t
{
	SETTINGS_VIEW_TYPE_SERVER,
	SETTINGS_VIEW_TYPE_ADDRESS,
	SETTINGS_VIEW_TYPE_APPLICATION,

	/* Each root item has configuration settings too. */
	SETTINGS_VIEW_TYPE_ROOT_GENERAL,
	SETTINGS_VIEW_TYPE_ROOT_SERVERS,
	SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS
};

/* Selecting a tree item will change the settings view, meaning the
 * right-side panel, which presents the REAL settings controls. 
 */
class SettingsTreeItem : public wxTreeItemData
{
public:
	/* Each settings item has a "type" and a name, the name being
	 * the real data, most-like an ID of sorts.
	 * The item may have an optional parent, which is not the root
	 * of its item "type", for instance an address which has a 
	 * server parent.
	 */
	SettingsTreeItem(settings_view_type_t type, 
		wxString name = wxEmptyString, 
		wxString parent = wxEmptyString)
	//item_name(name), item_parent(parent), item_type = type {}
	{
		item_parent = parent;
		item_name = name;
		item_type = type;
	}

public:
	settings_view_type_t item_type;
	wxString item_name;
	/* This might not be needed. */
	wxString item_parent;
};

#endif