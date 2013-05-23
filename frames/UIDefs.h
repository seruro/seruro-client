
#ifndef H_SeruroUIDefs
#define H_SeruroUIDefs

#include <wx/stattext.h>

/* Uniform UI options / configurations. */

#if defined(__WXMAC__)
#define SERURO_APP_DEFAULT_WIDTH  675
#else
#define SERURO_APP_DEFAULT_WIDTH  600
#endif
#define SERURO_APP_DEFAULT_HEIGHT 500

/* Settings view related definitions. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_MIN_WIDTH 175
#else
#define SERURO_SETTINGS_TREE_MIN_WIDTH 150
#endif
/* The settings tree event control id. */
#define SERURO_SETTINGS_TREE_ID 1009
#define SERURO_ADD_SERVER_PORT_ID 1010

/* OSX has larger indents. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_INDENT 8
#else
#define SERURO_SETTINGS_TREE_INDENT 12
#endif

/* Within the search results, the data columns should have a uniform width. */
#define SEARCH_PANEL_COLUMN_WIDTH (SERURO_APP_DEFAULT_WIDTH - 60) / 3

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
			SERURO_SETTINGS_TREE_MIN_WIDTH;
        this->Wrap(soft_wrap);
    }

private:
};

#endif