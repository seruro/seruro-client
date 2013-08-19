
#include "../../SeruroClient.h"
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

#include "../dialogs/DebugReportDialog.h"

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(LogWindow, SettingsView)
    EVT_BUTTON(wxID_ANY,  LogWindow::OnSendReport)
END_EVENT_TABLE()

LogWindow::LogWindow(SeruroPanelSettings *window) : SettingsView(window), SeruroLoggerTarget()
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    
    /* Inform the application that the log Window will be the log target. */
    wxGetApp().SetLoggerTarget(this);
    
    log_box = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
        wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP | wxHSCROLL);
    log_box->AppendText(wxGetApp().GetBufferedLog());
    
    sizer->Add(log_box, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 5));
    
    /* Check if this is a debug build, if so create a send log button. */
#if defined(__WXDEBUG__) || defined(RELEASE_DEBUG)
    wxSizer *const actions_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    send_button = new wxButton(this, wxID_ANY, _("Create Debug Report"));
    actions_sizer->Add(send_button, DIALOGS_SIZER_OPTIONS);
    actions_sizer->Add(new Text(this, _("Debug build enabled.")), DIALOGS_SIZER_OPTIONS);
    sizer->Add(actions_sizer, DIALOGS_SIZER_OPTIONS.FixedMinSize().Bottom());
#endif
    
    this->SetSizer(sizer);
}

void LogWindow::OnSendReport(wxCommandEvent &event)
{
#if defined(__WXDEBUG__) || defined(RELEASE_DEBUG)
    DebugReportDialog *dialog = new DebugReportDialog();
    dialog->SetLog(this->log_box->GetValue());
    
    if (dialog->ShowModal() == wxID_OK) {
		dialog->SendReport();
	}
	delete dialog;
#endif
}

void LogWindow::ProxyLog(wxLogLevel level, const wxString &msg)
{
    /* Check the level and apply a label. */
    
    /* Finally add the text to the log text control. */
    log_box->AppendText(wxString::Format("%s\n", msg));
}
