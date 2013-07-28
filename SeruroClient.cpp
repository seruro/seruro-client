// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
//C:\Program Files %28x86%29\Visual Leak Detector\lib\Win32
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SeruroClient.h"

/* Note: SeruroConfig/SeruroLogger are placed in the header. */

#include "crypto/SeruroCrypto.h"
#include "setup/SeruroSetup.h"
#include "api/SeruroRequest.h"
#include "frames/dialogs/ErrorDialog.h"

#include "frames/UIDefs.h"
#include "frames/SeruroFrameMain.h"

#include <wx/image.h>

#define SERURO_DEBUG_SETUP 0

#if !defined(__VLD__)
    /* (Hack) If the Virtual Leak Detector is not enabled (a MSW DLL only) then create dummy
     * symbols for control functions used in this application. */
    void VLDDisable (void) {}
    void VLDEnable (void) {}
#endif

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	/* Support for PNGs. */
	wxImage::AddHandler(new wxPNGHandler);

	/* Create a frame, but do not start sub frames, which may depend on config. */
	main_frame = new SeruroFrameMain(wxT(SERURO_APP_NAME),
		SERURO_APP_DEFAULT_WIDTH, SERURO_APP_DEFAULT_HEIGHT);
    this->SetTopWindow(main_frame);

	/* Start logger */
	InitLogger();

	/* User config instance */
    this->config = new SeruroConfig();
    
    /* Listen for invalid request events (which require UI actions and a request-restart). */
    Bind(SERURO_REQUEST_RESPONSE, &SeruroClient::OnInvalidAuth, this, SERURO_REQUEST_CALLBACK_AUTH);

	/* Now safe to start sub-frames (panels). */
	main_frame->AddPanels();

	/* There is an optional setup wizard. */
	if (! this->config->HasConfig() || SERURO_DEBUG_SETUP) {
        /* The panels and "book view" should be hidden while the wizard is running. */
        main_frame->Hide();
        
        SeruroSetup setup(main_frame);
		setup.RunWizard(setup.GetInitialPage());
	} else {
        main_frame->Show(true);
    }

    return true;
}

wxWindow* SeruroClient::GetFrame()
{
	return (wxWindow *) this->main_frame;
}

/*****************************************************************************************/
/************** LOGGING HANDLERS *********************************************************/
/*****************************************************************************************/

wxString SeruroClient::GetBufferedLog()
{
    /* Remove the existing one while returning its buffer. */
    wxString log_buffer = this->default_logger->GetBuffer();
    //delete this->default_logger;
    
    return log_buffer;
}

void SeruroClient::SetLoggerTarget(SeruroLoggerTarget *log_target)
{
    /* Wrapper to allow other classes to change the logger. */
    if (! SERURO_USE_LOGGER) {
        return;
    }
    
    /* Allow the logger to duplicate results to an additional target. */
    if (log_target == 0) {
        this->default_logger->RemoveProxyTarget();
    } else {
        this->default_logger->ToggleBuffer(false);
        this->default_logger->SetProxyTarget(log_target);
    }
}

void SeruroClient::InitLogger()
{
    if (SERURO_USE_LOGGER) {
        this->default_logger = new SeruroLogger();
        this->default_logger->InitLogger();
        wxLog::SetActiveTarget(this->default_logger);

        /* Tell the logger to buffer input. */
        this->default_logger->ToggleBuffer();
    } else {
        wxLogWindow *logger = new wxLogWindow(this->main_frame, wxT("Logger"));

        logger->GetFrame()->SetWindowStyle(wxDEFAULT_FRAME_STYLE);
        logger->GetFrame()->SetSize( wxRect(700,100,700,700) );
        wxLog::SetActiveTarget(logger);
    }
        
#if defined(__WXDEBUG__)
    wxLog::SetLogLevel(wxLOG_Debug);
    //wxLog::SetVerbose(true);
    LOG(_("Seruro (debug-build) started."));
#else
    LOG(_("Seruro started."));
#endif
}

/*****************************************************************************************/
/************** ERROR/EXCEPTIONS HANDLERS ************************************************/
/*****************************************************************************************/

void SeruroClient::OnAssertFailure(const wxChar *file, int line, 
	const wxChar *func, const wxChar *cond, const wxChar *msg)
{
	wxJSONValue report;
	wxString message;

	report["error_type"] = _("assertion_failure");
	report["file"] = file;
	report["line"] = line;
	report["func"] = func;
	report["cond"] = cond;

	message = wxString::Format(_("Assertion Failure\nFile: %s\nline: %s\nfunction: %s\ncondition:%s"), 
		report["file"].AsString(), report["line"].AsString(),
		report["func"].AsString(), report["cond"].AsString());

	if (wxIsMainThread()) {
		this->ReportAndExit(report, message, false);
	}
}

bool SeruroClient::OnExceptionInMainLoop()
{
	wxJSONValue report;
	
	report["error_type"] = _("mainloop_exception");
	try {
		throw;
	} catch ( ... ) {
		this->ReportAndExit(report);
	}

	return true;
}

void SeruroClient::OnUnhandledException()
{
	wxJSONValue report;

	report["error_type"] = _("unhandled_exception");
	try {
		throw;
	} catch ( ... ) {
		this->ReportAndExit(report);
	}
}

void SeruroClient::OnFatalException()
{
	wxJSONValue report;

#if wxUSE_ON_FATAL_EXCEPTION
	report["error_type"] = _("fatal_exception");
	this->ReportAndExit(report);
#endif
}

void SeruroClient::ErrorAndExit(wxString msg)
{
	ErrorDialog *error;

	error = new ErrorDialog(this->main_frame, msg, false);
	if (error->ShowModal() == wxID_NO) {}
	delete error;

	this->main_frame->Close(true);
}

void SeruroClient::ReportAndExit(wxJSONValue report, wxString msg, bool close_app)
{
	ErrorDialog *error;
	wxString prompt_msg;

	if (msg.compare(wxEmptyString) == 0) {
		prompt_msg = _("Seruro encountered an error, would you like to send a report?");
	} else {
		prompt_msg = msg;
	}

	error = new ErrorDialog(this->main_frame, prompt_msg);
	if (error->ShowModal() == wxID_YES) {
		/* The user would like to send an error report. */
	}
	delete error;

	if (close_app) {
		this->main_frame->Close(true);
		//delete this->main_frame;
	}
}

/*****************************************************************************************/
/************** GLOBAL REQUEST HANDLERS **************************************************/
/*****************************************************************************************/

void SeruroClient::AddEvent(wxEvent &event)
{
	this->AddPendingEvent(event);
}

void SeruroClient::OnInvalidAuth(SeruroRequestEvent &event)
{
    /* (From api/SeruroRequest) Perform UI actions, then create identical request. */
    PerformRequestAuth(event);
}
