
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/splitter.h>
#include <wx/treectrl.h>
#include <wx/treebase.h>

#define SERURO_SETTINGS_MIN_WIDTH 150
/* The tree event control id. */
#define SERURO_SETTINGS_TREE_ID 1009

class SettingsTreeItem;
class SettingsPanel;

// Define a new frame type: this is going to be our main frame
class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);

	wxWindow* GetViewer();
	bool HasPanel(const wxString &name, 
		const wxString &parent = wxString(wxEmptyString));
	void AddPanel(int panel_ptr, const wxString &name, 
		const wxString &parent = wxString(wxEmptyString));
	void ShowPanel(const wxString &name, 
		const wxString &parent = wxString(wxEmptyString));

private:
	/* Keep all panels (lazily created) for easy switching.
	 * This also allows "non-saved" settings to persist in the UI.
	 */
	/* Warning: this may be hackish, to save pointers in JSON. */
	wxJSONValue panels;

	/* The splitter create the dual-view construct. */
	wxSplitterWindow *splitter;
	SettingsPanel *current_panel;
};

/* A default panel (view) for settings. */
class SettingsPanel : public wxPanel
{
public:
	/* We construct each settings panel with the calling panel
	 * this caller is a panel belonging to the application.
	 * This panel may implement a 'splitting' or 'sashing' window
	 * which will be the parent of all settings panels. 
	 */
	SettingsPanel(SeruroPanelSettings *instance_panel) :
	  wxPanel(instance_panel->GetViewer(), wxID_ANY),
	  main_panel(instance_panel) {}

protected:
	/* Save a reference to the creating panel, to add additional
	 * sub-panels and update the current view.
	 */
	SeruroPanelSettings *main_panel;
};

/* Given the above abstract class which provides a main_panel
 * object as the calling panel: child classes can call into
 * this main_panel to perform view changes or access configuration
 * settings.
 */
class SettingsPanel_Address : public SettingsPanel
{
public:
	SettingsPanel_Address(SeruroPanelSettings *parent,
		const wxString &address, const wxString &server);
};

class SettingsPanel_RootGeneral : public SettingsPanel
{
public:
	SettingsPanel_RootGeneral(SeruroPanelSettings *parent);
};

/* Show a tree-view for selecting various parts of the settings. */
class SettingsPanelTree : public SettingsPanel
{
public:
	SettingsPanelTree(SeruroPanelSettings *parent);

	/* Event handler for selecting a panel item. */
	void OnSelectItem(wxTreeEvent &event);

private:
	void ShowView_Server(const SettingsTreeItem *data);
	void ShowView_Address(const SettingsTreeItem *data);
	void ShowView_Application(const SettingsTreeItem *data);

	void ShowView_RootServers();
	void ShowView_RootGeneral();
	void ShowView_RootApplications();

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