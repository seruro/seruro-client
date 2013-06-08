
#ifndef H_SeruroPanelSettings
#define H_SeruroPanelSettings

#include "SeruroFrame.h"
#include "settings/SettingsPanels.h"

#include "../wxJSON/wx/jsonval.h"

#include <wx/splitter.h>

/* Provided by settings/SettingsPanels.h. */
class SettingsPanel;
//extern enum settings_view_type settings_view_type_t;

// Define a new frame type: this is going to be our main frame
class SeruroPanelSettings : public SeruroPanel
{
public:
    SeruroPanelSettings(wxBookCtrlBase *book);

	/* Return a pointer to the tree/view window splitter. */
	wxSplitterWindow* GetViewer();
	/* Each of the following includes a type of panel,
	 * An optional name, and optional parent. */
	bool HasPanel(settings_view_type_t type,
		const wxString &name   = wxString(wxEmptyString), 
		const wxString &parent = wxString(wxEmptyString));
	/* Assign a panel view to a lookup table. */
	void AddPanel(SettingsPanelView *panel_ptr, 
		settings_view_type_t type,
		const wxString &name   = wxString(wxEmptyString), 
		const wxString &parent = wxString(wxEmptyString));
	/* Render a panel view. */
	void ShowPanel(settings_view_type_t type,
		const wxString &name   = wxString(wxEmptyString), 
		const wxString &parent = wxString(wxEmptyString));

	/* Create the "root" panels. */
	void AddFirstPanel();
	/* Add a new item to the settings tree. */
	void AddTreeItem(settings_view_type_t type,
		const wxString &name   = wxString(wxEmptyString), 
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
	SettingsPanelTree *tree_panel;
};

#endif