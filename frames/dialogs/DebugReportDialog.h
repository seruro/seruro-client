
#ifndef H_SeruroDebugReportDialog
#define H_SeruroDebugReportDialog

#include <wx/textctrl.h>
#include <wx/dialog.h>
#include <wx/checkbox.h>

class DebugReportDialog : public wxDialog
{
public:
	DebugReportDialog();
    void SetLog(const wxString &log_source) { this->log = log_source; }
    
    /* Performs the send function. */
    void SendReport();
    
private:
    wxTextCtrl *report;
    wxString log;
};

#endif