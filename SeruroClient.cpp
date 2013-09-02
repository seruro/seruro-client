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
#include "SeruroMonitor.h"
#include "SeruroConfig.h"
#include "apps/SeruroApps.h"
#include "logging/SeruroLogger.h"
#include "crypto/SeruroCrypto.h"
#include "api/SeruroRequest.h"

#include "frames/UIDefs.h"
#include "frames/SeruroMain.h"
#include "frames/dialogs/AlertDialog.h"
#include "frames/dialogs/ErrorDialog.h"

#include <wx/image.h>

#include <wx/notifmsg.h>

#if !defined(__VLD__)
    /* (Hack) If the Virtual Leak Detector is not enabled (a MSW DLL only) then create dummy
     * symbols for control functions used in this application. */
    void VLDDisable (void) {}
    void VLDEnable (void) {}
#endif

#define SERURO_MONITOR_MILLI_DELAY 4000

IMPLEMENT_APP(SeruroClient)

int SeruroClient::OnExit()
{
    {
        /* Assure the thread still exists when it is checked. */
        wxCriticalSectionLocker enter(seruro_critsection_monitor);
        if (this->seruro_monitor) {
            if (this->seruro_monitor->Delete() != wxTHREAD_NO_ERROR) {
                /* This is a problem. */
            }
        }
        /* Allow the thread to enter it's destructor. */
    }
    
    while (1) {
        {
            wxCriticalSectionLocker enter(seruro_critsection_monitor);
            if (! this->seruro_monitor) break;
        }
        /* Wait for the destructor to function. */
        wxThread::This()->Sleep(1);
    }
    
    delete instance_limiter;
	return 0;
}

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    if (this->IsAnotherRunning()) {
        return false;
    }
        
	/* Support for PNGs. */
	wxImage::AddHandler(new wxPNGHandler);

	/* Create a frame, but do not start sub frames, which may depend on config. */
	main_frame = new SeruroFrameMain(wxT(SERURO_APP_NAME),
		SERURO_APP_DEFAULT_WIDTH, SERURO_APP_DEFAULT_HEIGHT);
    this->SetTopWindow(main_frame);
    
	/* Start logger */
	InitLogger();
    
    /* Listen for invalid request events (which require UI actions and a request-restart). */
    Bind(SERURO_REQUEST_RESPONSE, &SeruroClient::OnInvalidAuth, this, SERURO_REQUEST_CALLBACK_AUTH);

    /* Start external state monitor. */
    StartMonitor();
    
	/* Now safe to start sub-frames (panels). */
	main_frame->AddPanels();
    /* Check to see if the application is running for the first time. */
    main_frame->StartSetup(false);

    //AlertDialog *alert = new AlertDialog();
    //this->SetTopWindow(alert);
    //alert->ShowModal();

    return true;
}

void SeruroClient::PauseMonitor()
{
    /* Assure the thread pointer is not deleted while pausing. */
    wxCriticalSectionLocker enter(seruro_critsection_monitor);
    if (this->seruro_monitor) {
        /* The thread may return, or the app my cause a delete. */
        if (this->seruro_monitor->Pause() != wxTHREAD_NO_ERROR) {
            DEBUG_LOG(_("SeruroClient> (PauseMonitor) Cannot pause monitor."));
        }
    }
}

void SeruroClient::StartMonitor()
{
    /* Start monitoring for external state changes. */
    seruro_monitor = new SeruroMonitor(this, SERURO_MONITOR_MILLI_DELAY);
    if (seruro_monitor->Run() != wxTHREAD_NO_ERROR) {
        DEBUG_LOG(_("SeruroClient> (StartMonitor) Cannot create monitor."));
        
        delete seruro_monitor;
        DeleteMonitor();
    } else {
        DEBUG_LOG(_("SeruroClient> (StartMonitor) external event monitor started."));
    }
}

bool SeruroClient::IsAnotherRunning()
{
    wxLogNull log_silencer;
    this->instance_limiter = new wxSingleInstanceChecker;
    instance_limiter->Create(wxString::Format(_("%s_%s"), GetAppName(), wxGetUserId()), GetAppDir());
    log_silencer.~wxLogNull();
    
    if (instance_limiter->IsAnotherRunning()) {
        /* On MSW and Unix, make sure only one Seruro runs per-user. */
        /* OnExit will not be called. */
        delete instance_limiter;
        instance_limiter = NULL;
        
        return true;
    }
    
    return false;
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
		error->SendReport();
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
