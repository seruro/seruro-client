
#ifndef H_SettingsPanels
#define H_SettingsPanels

#include "../../Defs.h"
#include "../UIDefs.h"

#include "../../api/SeruroRequest.h"

#include <wx/treectrl.h>
#include <wx/treebase.h>
#include <wx/panel.h>
#include <wx/stattext.h>
//#include <wx/log.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>

//#include <vector>

class SettingsPanelTree;
class SeruroPanelSettings;

/* Define the various types/classes of views in the settings. */
enum settings_view_type_t
{
	SETTINGS_VIEW_TYPE_SERVER			 = 0x01,
	SETTINGS_VIEW_TYPE_ADDRESS			 = 0x02,
	SETTINGS_VIEW_TYPE_APPLICATION		 = 0x03,

	/* Each root item has configuration settings too. */
	SETTINGS_VIEW_TYPE_ROOT_GENERAL		 = 0x04,
	SETTINGS_VIEW_TYPE_ROOT_ACCOUNTS	 = 0x05,
	SETTINGS_VIEW_TYPE_ROOT_APPLICATIONS = 0x06
};

//extern enum settings_view_type settings_view_type_t;

/* A default panel (view) for settings. */
class SettingsPanel : public wxScrolledWindow
{
public:
	/* We construct each settings panel with the calling panel
	 * this caller is a panel belonging to the application.
	 * This panel may implement a 'splitting' or 'sashing' window
	 * which will be the parent of all settings panels. 
	 */
	SettingsPanel(SeruroPanelSettings *instance_panel);

	/* Save a reference to the creating panel, to add additional
	 * sub-panels and update the current view.
	 */
	SeruroPanelSettings* MainPanel() { return main_panel; }

protected:
	SeruroPanelSettings *main_panel;
};

class SettingsPanelView : public SettingsPanel
{
public:
	SettingsPanelView(SeruroPanelSettings *instance_panel);
	
	/* Perform an abstract check against the data displayed
	 * inside the settings view, if the data has changed, then
	 * the caller should know how to refresh the view.
	 * Changed is similar to the controller in a MVC.
	 *
	 * Changed could be avoided if the view was just rendered
	 * every time, while refreshing data from the configuration
	 * put this would be UI intensive, and would possibly 
	 * cause flickering.
	 */
	virtual bool Changed() { return false; }
	/* The UI drawing takes place in Render. This is similar to
	 * a View in the MVC framework. 
	 */
	virtual void Render() {}

	/* Used once a panel view has been created. 
	 * This is mainly a helper to reduce code. 
	 */
	void ReRender() {
#if ! defined(__WXMSW__)
		this->Freeze();
#endif
		/* Free memory. */
        if (this->GetSizer()) this->GetSizer()->Clear(true);
		/* Perform panel-specific (virtual) render. */
		this->Render();
		this->Layout();
#if ! defined(__WXMSW__)
		this->Thaw();
#endif
	}

	void InitSizer() {
		/* Use this for scrollbars. */
		this->FitInside();
		this->SetScrollRate(5, 5);
	}
};

/* Given the above abstract class which provides a main_panel
 * object as the calling panel: child classes can call into
 * this main_panel to perform view changes or access configuration
 * settings.
 */
class SettingsPanel_Address : public SettingsPanelView
{
public:
	SettingsPanel_Address(SeruroPanelSettings *parent,
		const wxString &address, const wxString &server);
	bool Changed();
	void Render();
    
    /* Button actions (no edit). */
    void OnUpdate(wxCommandEvent &event);
    void OnRemove(wxCommandEvent &event);

	void OnUpdateResponse(SeruroRequestEvent &event);
    
private:
    wxString address;
    wxString server_name;
    
    DECLARE_EVENT_TABLE()
};

/* SERVER SETTINGS */
class SettingsPanel_Server : public SettingsPanelView
{
public:
	SettingsPanel_Server(SeruroPanelSettings *parent,
		const wxString &server);
	bool Changed();
	void Render();
    
    /* Button actions. */
	void OnUpdate(wxCommandEvent &event);
	void OnUpdateResult(SeruroRequestEvent &event);
	void OnEdit(wxCommandEvent &event);
	void OnRemove(wxCommandEvent &event);

private:
    wxString server_name;
    
	DECLARE_EVENT_TABLE()
};

/* GENERAL SETTINGS */
class SettingsPanel_RootGeneral : public SettingsPanelView
{
public:
	SettingsPanel_RootGeneral(SeruroPanelSettings *parent) :
	  SettingsPanelView(parent) {}
	void Render();
};

/* ACCOUNTS / SERVER SETTINGS */
class SettingsPanel_RootAccounts : public SettingsPanelView
{
public:
	SettingsPanel_RootAccounts(SeruroPanelSettings *parent) :
        SettingsPanelView(parent) {}
	bool Changed();
	void Render();

	void OnAddServer(wxCommandEvent &event);
	void OnAddAddress(wxCommandEvent &event);

	/* API/Actions callbacks. */
	void OnAddAddressResult(SeruroRequestEvent &event);

private:
	DECLARE_EVENT_TABLE();
};

class SettingsTree : public wxTreeCtrl
{
public:
    SettingsTree (SettingsPanelTree *panel);
    
	/* Event handler for selecting a panel item. */
	void OnSelectItem(wxTreeEvent &event);

private:
	/* Save reference to parent (panel) to use it's
	 * main_panel member. */
	SeruroPanelSettings *main_panel;
    DECLARE_EVENT_TABLE()
};

/* Show a tree-view for selecting various parts of the settings. */
class SettingsPanelTree : public SettingsPanel
{
public:
	SettingsPanelTree(SeruroPanelSettings *parent);

	void AddItem(settings_view_type_t type,
		wxString name = wxEmptyString, 
		wxString parent = wxEmptyString);

	void RemoveItem(settings_view_type_t type,
		wxString name = wxEmptyString, 
		wxString parent = wxEmptyString);
    
private:
	wxTreeItemId GetItemParent(settings_view_type_t type,
		wxString name = wxEmptyString, 
		wxString parent = wxEmptyString);
	wxTreeItemId GetItem(settings_view_type_t type,
		wxString name = wxEmptyString, 
		wxString parent = wxEmptyString);

	wxTreeCtrl *settings_tree;
	wxTreeItemId root;
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