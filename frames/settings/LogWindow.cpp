
#include "../../SeruroClient.h"
#include "SettingsWindows.h"
#include "../SeruroPanelSettings.h"
#include "../UIDefs.h"

#include "../dialogs/DebugReportDialog.h"

DECLARE_APP(SeruroClient);

BEGIN_EVENT_TABLE(LogWindow, SettingsView)
    EVT_BUTTON(wxID_ANY,  LogWindow::OnSendReport)
END_EVENT_TABLE()

LogWindow::LogWindow(SeruroPanelSettings *window) : SettingsView(window), SeruroLogger()
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    //wxSizer *const sizer = new wxSizer();
    //wxSizer *const horizontal_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    //Text *warning = new Text(this, _("Extensions are disabled."));
    //sizer->Add(warning, DIALOGS_SIZER_OPTIONS);
    
    /* Inform the application that the log Window will be the log target. */
    this->InitLogger();
    wxGetApp().SetLogger(this);
    
    log_box = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                             wxTE_MULTILINE | wxTE_READONLY | wxTE_DONTWRAP);
    log_box->AppendText(wxGetApp().ReplaceLogger());
    
    sizer->Add(log_box, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 5));
    
    /* Check if this is a debug build, if so create a send log button. */
#if defined(__WXDEBUG__) || defined(DEBUG)
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
    DebugReportDialog *dialog = new DebugReportDialog();
    dialog->SetLog(this->log_box->GetValue());
    
    if (dialog->ShowModal() == wxID_OK) {
		dialog->SendReport();
	}
	delete dialog;
}

void LogWindow::ProxyLog(wxLogLevel level, const wxString &msg)
{
    /* Check the level and apply a label. */
    
    /* Finally add the text to the log text control. */
    log_box->AppendText(wxString::Format("%s\n", msg));
}
