
#ifndef H_SeruroErrorDialog
#define H_SeruroErrorDialog

#include <wx/msgdlg.h>

/***
 * The error dialog is a message dialog that overrides the 
 * YES / NO to a Send Error, Exit
 * Where both options will eventually close the application, but 
 * 'Send Error' will allow the user to upload the crash report.
 *
 * Note: these 'function responses' must be implemented elsewhere.
 */

class ErrorDialog : public wxMessageDialog
{
public:
	ErrorDialog(wxWindow *parent, const wxString &message, 
		bool allow_report = true);
};

#endif