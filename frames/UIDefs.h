
#ifndef H_SeruroUIDefs
#define H_SeruroUIDefs

#include <wx/stattext.h>
#include <wx/mstream.h>
#include <wx/bitmap.h>

#define DISABLED_BACKGROUND_COLOR "light grey"
#define DISABLED_TEXT_COLOR "grey"

/* Menu item text */
#define SERURO_MENU_HOME      "Show Seruro Home"
#define SERURO_MENU_CONTACTS  "Show Contacts List"
#define SERURO_MENU_SEARCH    "Open Contact Search..."
#define SERURO_MENU_HELP      "Open &Help..."
#define SERURO_MENU_SETTINGS  "Open Seruro Settings..."
#define SERURO_MENU_EXIT      "E&xit"


/* Element IDs (where wxID_ANY is not appropriate. */
#define SERURO_SETTINGS_TREE_ID		1009
#define SERURO_ADD_SERVER_PORT_ID	1010
#define SERURO_NOTEBOOK_ID			1030
//#define SERURO_SETUP_ID				1031
#define SERURO_PANEL_CONTACTS_ID    1019
#define SERURO_PANEL_SETTINGS_ID	1020
#define SERURO_PANEL_SEARCH_ID		1021
#define SERURO_PANEL_ENCRYPT_ID		1022
#define SERURO_PANEL_DECRYPT_ID		1023
#define SERURO_EXIT_ID				6667

/* Uniform UI options / configurations. */
#if defined(__WXMAC__)
#define SERURO_APP_DEFAULT_WIDTH  675
#else
#define SERURO_APP_DEFAULT_WIDTH  600
#endif
#define SERURO_APP_DEFAULT_HEIGHT 500

/* Settings view related definitions. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_MIN_WIDTH 150
#else
#define SERURO_SETTINGS_TREE_MIN_WIDTH 125
#endif

/* When a flex-sizer is used the "label" is the first column (for each row). */
#define SERURO_SETTINGS_FLEX_LABEL_WIDTH SERURO_SETTINGS_TREE_MIN_WIDTH

/* OSX has larger indents. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_INDENT 8
#else
#define SERURO_SETTINGS_TREE_INDENT 12
#endif

/* Within the search results, the data columns should have a uniform width. */
#define SEARCH_PANEL_COLUMN_WIDTH (SERURO_APP_DEFAULT_WIDTH - 60)

#define SETTINGS_PANEL_SIZER_OPTIONS \
wxSizerFlags().Expand().Border(wxALL, 5)
#define SETTINGS_PANEL_BOXSIZER_OPTIONS \
wxSizerFlags().Expand().Border(wxBOTTOM)
#define SETTINGS_PANEL_BUTTONS_OPTIONS \
wxSizerFlags().Right().Expand()

#define DIALOGS_BOXSIZER_OPTIONS SETTINGS_PANEL_BOXSIZER_OPTIONS
#define DIALOGS_SIZER_OPTIONS SETTINGS_PANEL_SIZER_OPTIONS
#define DIALOGS_BOXSIZER_SIZER_OPTIONS \
wxSizerFlags().Expand().Border(wxTOP | wxRIGHT | wxLEFT, 5)
#define DIALOGS_BUTTONS_OPTIONS \
wxSizerFlags().Right().Border()

#define GRID_SIZER_WIDTH 5
#define GRID_SIZER_HEIGHT 5
#define DIALOG_WRAP_WIDTH 300

/* Helper class for text within panels, which will auto wrap to fit
 * the size of the second panel. (in TreePanel)
 */
class Text : public wxStaticText
{
public:
	Text(wxWindow *parent_ctrl, const wxString &text, bool auto_wrap = true) : 
		wxStaticText(parent_ctrl, wxID_ANY, text, 
			wxDefaultPosition, wxDefaultSize)
    {
		if (! auto_wrap) { return; }
        int soft_wrap = SERURO_APP_DEFAULT_WIDTH - 
			SERURO_SETTINGS_TREE_MIN_WIDTH - 100;
        this->Wrap(soft_wrap);
    }

private:
};

/* Reading in PNGs from header files. */
#define wxGetBitmapFromMemory(name) \
_wxGetBitmapFromMemory(name ## _png, sizeof(name ## _png))

/* The image handler for PNG is enabled in SeruroClient.cpp. */
inline wxBitmap _wxGetBitmapFromMemory(const unsigned char *data, 
	int length) {
	wxMemoryInputStream is(data, length);
	return wxBitmap(wxImage(is, wxBITMAP_TYPE_ANY, -1), -1);
}

#endif