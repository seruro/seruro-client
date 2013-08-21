#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#include "Defs.h"

#include <wx/wxprec.h>
#include <wx/wx.h>

#include <wx/notebook.h>
#include <wx/thread.h>
#include <wx/event.h>
#include <wx/log.h>
#include <wx/snglinst.h>

/* Inlcude the Config header so all classes may use wxGetApp().config. */
#include "SeruroConfig.h"
#include "logging/SeruroLogger.h"

//class SeruroConfig;
class SeruroFrameMain;
class SeruroRequestEvent;
class SeruroConfig;

/* Source: thread samples */
#include <wx/dynarray.h>
WX_DEFINE_ARRAY_PTR(wxThread *, wxArrayThread);

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    
    /* Run networking thread from OnInit() */
    virtual bool OnInit();
	int OnExit() {
        delete instance_limiter;
		delete config;
		return 0;
	}
    
    /* Abstraction for instance limiter. */
    bool IsAnotherRunning();

    /* Logging functions. */
	void InitLogger();
    //void SetLogger(wxLog *logger);
    //wxString ReplaceLogger();
    void SetLoggerTarget(SeruroLoggerTarget *log_target);
    wxString GetBufferedLog();
    
	wxWindow *GetFrame();
    
    void OnInvalidAuth(SeruroRequestEvent &event);

	/* Todo: Consider accessor methods */
	wxCriticalSection seruro_critsection_config;
	wxCriticalSection seruro_critsection_thread;
	wxCriticalSection seruro_critsection_token;
	wxCriticalSection seruro_critsection_log;

	wxArrayThread seruro_threads;

	void AddEvent(wxEvent &event);

	/* Error/exception/assertion handling. */
	void OnAssertFailure(const wxChar *file, int line, 
		const wxChar *func, const wxChar *cond,
		const wxChar *msg);
	bool OnExceptionInMainLoop();
	void OnUnhandledException();
	void OnFatalException();

	/* For other components to use (for handled exceptions. */
	void ErrorAndExit(wxString msg);

public:
	SeruroConfig *config;

private:
	/* Shows a message and prompts to send an error report. */
	void ReportAndExit(wxJSONValue report, 
		wxString msg = wxEmptyString, bool close_app = true);

	SeruroFrameMain *main_frame;
    SeruroLogger *default_logger;
    
    /* Assure only one instance of the application is running. */
    wxSingleInstanceChecker *instance_limiter;
};

/* All MSW to enable Virtual Leak Detection. */
#if defined(__VLD__) && defined(__WXMSW__)
    #undef GetClassInfoW
    #include <wx/msw/winundef.h>
    #include <vld.h>
#else
    void VLDDisable (void);
    void VLDEnable (void);
#endif

#endif
