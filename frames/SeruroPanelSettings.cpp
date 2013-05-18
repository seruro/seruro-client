
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
	SettingsPanelView *root_general_panel = new SettingsPanel_RootGeneral(this);

	//wxString panel_name = wxT("root_general");
	this->AddPanel(root_general_panel, SETTINGS_VIEW_TYPE_ROOT_GENERAL);
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
void SeruroPanelSettings::AddPanel(SettingsPanelView *panel_ptr, settings_view_type_t type,
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
        /* Store the index + 1, when retreiving a 0 is a failure, the minimum is 1. */
        panel_list[panel_list_size++] = panel_ptr;
		this->panels[type][parent][name] = panel_list_size;
		return;
	}

	/* Show scrollbars for panel. */
	panel_ptr->InitSizer();

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
		/* Very odd state. */
		wxLogMessage(wxT("SeruroPanelSettings> Something is terribly wrong, got a NULL pointer from panels."));
		return;
	}

	/* Subtract 1, since the "index store" avoids using 0. */
	SettingsPanelView *new_panel(panel_list[panel_ptr-1]);

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
