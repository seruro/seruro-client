
#include "SeruroPanelSettings.h"
#include "settings/SettingsPanels.h"

#include "../api/SeruroServerAPI.h"

#include <wx/stattext.h>
#include <wx/button.h>
#include <wx/log.h>

/* Keep a list of panel pointers, which should not be stored as JSON ints. */
SettingsPanelView *panel_list[32];
/* The JSON mapping stores the panel pointer as an integer index into the list (+1). */
int panel_list_size = 0;

#if 0
void DestoryAll(wxSizer *sizer)
{
	//wxSizerItemList children = sizer->GetChildren();
	//size_t num_items = sizer->GetItemCount();

	wxSizerItem *item;
	wxWindow *window;
	//for (size_t i = ; i < num_items; i++) {
	/*while (item = sizer->GetItem((size_t) 0)) {
		//item = sizer->GetItem(i);
		if (item->IsWindow()) {
			window = item->GetWindow();
			window->DestroyChildren();
			window->Destroy();
			//delete window;
		}
		sizer->Detach(0);
		//if (item) delete item;
	}*/
	sizer->Clear(true);
}
#endif

SeruroPanelSettings::SeruroPanelSettings(wxBookCtrlBase *book) : SeruroPanel(book, wxT("Settings"))
{
	/* Override default sizer. */
	wxBoxSizer *container_sizer = new wxBoxSizer(wxHORIZONTAL);

	//container_sizer->Add(settings_tree, 0, wxEXPAND, 5);
	//container_sizer->Add(test_panel, 0, wxEXPAND, 5);

	/* Create a resizeable window for the navigation pane (panel) and it's controlling view. */
	this->splitter = new wxSplitterWindow(this, wxID_ANY,
        wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH | wxSP_BORDER);

	/* Create a tree control as well as the first settings view (general). */
	SettingsPanelTree *settings_tree = new SettingsPanelTree(this);
    /* Seed the current panel with the Root panel: general. */
	this->AddFirstPanel();

	splitter->SplitVertically(settings_tree, this->current_panel);
    
    this->splitter->SetSize(GetClientSize());
	this->splitter->SetSashGravity(1.0);
	this->splitter->SetMinimumPaneSize(SERURO_SETTINGS_TREE_MIN_WIDTH);
    //settings_tree->Layout();
    
	container_sizer->Add(this->splitter, 1, wxEXPAND | wxALL, 10);
	this->SetSizer(container_sizer);
    //container_sizer->SetSizerHints(this);
}

/* To help with organization, perform the initialization of the first panel as it's own method.
 * In most cases, this is the 'General' root panel. 
 */
void SeruroPanelSettings::AddFirstPanel()
{
    /* The initial view, general settings, must be set as the current_panel as well as added
	 * to the list of 'instanciated' panels.
	 */
	SettingsPanelView *root_panel = new SettingsPanel_RootGeneral(this);

	//wxString panel_name = wxT("root_general");
	this->AddPanel(root_panel, SETTINGS_VIEW_TYPE_ROOT_GENERAL);
    this->AddPanel(new SettingsPanel_RootAccounts(this), SETTINGS_VIEW_TYPE_ROOT_ACCOUNTS);
    
    //this->ShowPanel(SETTINGS_VIEW_TYPE_ROOT_GENERAL);
    this->splitter->Layout();
    
	this->current_panel = root_panel;
}

wxSplitterWindow* SeruroPanelSettings::GetViewer()
{
	return (wxSplitterWindow*) this->splitter;
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
void SeruroPanelSettings::AddPanel(SettingsPanelView *panel_ptr, settings_view_type_t type,
	const wxString &name, const wxString &parent)
{
	wxJSONValue type_value;
	if (! this->panels.HasMember(type)) {
		/* See the JSON value with this type of panel. */
		this->panels[type] = type_value;
	}

	/* Show scrollbars for panel. (But be called when created for the "first panel" edge case.) */
	panel_ptr->InitSizer();
	panel_ptr->Render();
    
    wxLogMessage(wxT("Adding panel (name= %s) (parent= %s)."), name, parent);

	if (parent.compare(wxEmptyString) != 0) {
		if (! this->panels[type].HasMember(parent)) {
			wxJSONValue parent_value;
			this->panels[type][parent] = parent_value;
		}
        /* Store the index + 1, when retreiving a 0 is a failure, the minimum is 1. */
        panel_list[panel_list_size++] = panel_ptr;
		this->panels[type][parent][name] = panel_list_size;
		return;
	}

	panel_list[panel_list_size++] = panel_ptr;
	this->panels[type][name] = panel_list_size;
	return;
}

/* Search the JSON map for a panel index, then access via the indexes array. */
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
		/* Very odd state (something added a null pointer, which they thought was a panel). */
		wxLogMessage(wxT("SeruroPanelSettings> Something is terribly wrong, got a NULL pointer from panels."));
		return;
	}

	/* Subtract 1, since the "index store" avoids using 0. */
	SettingsPanelView *new_panel(panel_list[panel_ptr-1]);

	bool changed = this->splitter->ReplaceWindow(this->current_panel, new_panel);
	if (! changed) {
		/* The UI parent was unable to replace the current panel with the requested panel */
		wxLogMessage(wxT("SeruroPanelSettings> Something is terribly wrong, could not replace view."));
		return;
	}

	if (new_panel->Changed()) {
		/* Ask the panel to compare it's UI to the current application state, if the view
		 * has changed then ask the panel to re-render the UI.
		 */
		new_panel->ReRender();
	}

	/* Hide the previously shown panel, and show the requested view, finally record the pointer as current. */
	wxLogMessage(wxT("SeruroPanelSettings> View is switching."));
	this->current_panel->Show(false);
	new_panel->Show(true);
	this->current_panel = new_panel;
}
