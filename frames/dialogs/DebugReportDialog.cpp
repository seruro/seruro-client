
#if defined(__WXDEBUG__) || defined(RELEASE_DEBUG)

#include "DebugReportDialog.h"
#include "../UIDefs.h"
#include "../../AppDefs.h"

#include "../../SeruroClient.h"
#include "../../SeruroConfig.h"
#include "../../logging/SeruroLogger.h"

#include "../../wxJSON/wx/jsonval.h"
#include "../../wxJSON/wx/jsonwriter.h"

#include "../../api/Utils.h"
#include "../../api/SeruroRequest.h"

#include <wx/base64.h>

DECLARE_APP(SeruroClient);

DebugReportDialog::DebugReportDialog()
  : wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Debug Report")))
{
    wxSizer *const sizer = new wxBoxSizer(wxVERTICAL);
    wxSizer *const version_sizer = new wxBoxSizer(wxHORIZONTAL);
    
    sizer->Add(new Text(this,
        wxT("The activity log and the client configuration (without tokens) will be sent as a debug report. ")
        wxT("You may be contacted for additional details.")
    ), DIALOGS_SIZER_OPTIONS);

	version_sizer->Add(new Text(this, 
		wxString::Format(_("Build version: %s."), _(SERURO_VERSION))), DIALOGS_SIZER_OPTIONS);
    sizer->Add(version_sizer, DIALOGS_BOXSIZER_SIZER_OPTIONS);
    
    report = new wxTextCtrl(this, wxID_ANY, _("Optionally, describe this report..."),
        wxDefaultPosition, wxSize(300, 150), wxTE_MULTILINE);
    sizer->Add(report, wxSizerFlags().Expand().Border(wxTOP | wxRIGHT | wxLEFT, 10));
    this->log = wxEmptyString;
    
    sizer->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), DIALOGS_BUTTONS_OPTIONS);
    this->SetSizerAndFit(sizer);
}

void DebugReportDialog::SendReport()
{
    /* Do something with report. */
    wxString       content;
    wxMemoryBuffer content_buffer;
    wxJSONValue params, content_data;
    
    content_data["config"] = theSeruroConfig::Get().GetConfig();
    content_data["log"] = this->log;
    content_data["version"] = _(SERURO_VERSION);
    content_data["report"] = this->report->GetValue();
    
    /* Flatten the json data. */
	wxJSONWriter content_writer(wxJSONWRITER_NONE);
	content_writer.Write(content_data, content);
    /* Create a key/value pair for to encode as post data. */
    content_data = wxJSONValue(wxJSONTYPE_OBJECT);
    content_buffer.AppendData(content.mb_str(wxConvUTF8), content.length());
    content_data["report"] = wxBase64Encode(content_buffer);
    
    params["server"]["host"] = _(DEBUG_REPORT_SERVER);
    params["server"]["port"] = _(DEBUG_REPORT_PORT);
    params["verb"] = _("POST");
    params["object"] = _(DEBUG_REPORT_URL);
    params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
    params["not_api"] = true;
    params["data_string"] = content_data["report"];
    
    /* Allow the request to run off into the distance... */
    SeruroRequest *request = new SeruroRequest(params, this->GetEventHandler(), SERURO_REQUEST_CALLBACK_NONE);
    request->Create();
    
    request->Run();
    DEBUG_LOG(_("DebugReportDialog> (SendReport) sent debug report."));
}

#endif