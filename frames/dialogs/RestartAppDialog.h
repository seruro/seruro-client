
#ifndef H_SeruroRestartAppDialog
#define H_SeruroRestartAppDialog

#include <wx/msgdlg.h>

#include "../../apps/SeruroApps.h"

/***
 * The error dialog is a message dialog that overrides the
 * YES / NO to a Send Error, Exit
 * Where both options will eventually close the application, but
 * 'Send Error' will allow the user to upload the crash report.
 *
 * Note: these 'function responses' must be implemented elsewhere.
 */

class RestartAppDialog : public wxMessageDialog
{
public:
	RestartAppDialog(wxWindow *parent, const wxString &app_name);
};

#endif