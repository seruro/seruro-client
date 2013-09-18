
#ifndef H_SeruroRestartAppDialog
#define H_SeruroRestartAppDialog

#include <wx/msgdlg.h>

#define TEXT_RESTART_WARNING "The application \"%s\" must be restarted \
to apply the configuration changes. \
Please save your work and quit the application yourself or use the button below. \
If you choose the cancel the configuration changes may be lost."

class RestartAppDialog : public wxMessageDialog
{
public:
	RestartAppDialog(wxWindow *parent, const wxString &app_name);
    wxString GetAppName();
    
    void SetIdentity(wxString address);
    void DialogClosed(wxWindowModalDialogEvent& event);
    
private:
    wxString app_name;
    
    /* If this restart is bound to an identity-configuration. */
    wxString identity;
};

#endif