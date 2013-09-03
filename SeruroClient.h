#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#include "Defs.h"

#include <wx/wxprec.h>
#include <wx/wx.h>

#include <wx/thread.h>
#include <wx/event.h>
#include <wx/snglinst.h>

#include "wxJSON/wx/jsonval.h"

/* Inlcude the Config header so all classes may use wxGetApp().config. */
//#include "SeruroConfig.h"

//class SeruroConfig;
class SeruroMonitor;
class SeruroLogger;
class SeruroLoggerTarget;
class SeruroFrameMain;
class SeruroRequestEvent;
//class SeruroConfig;

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
	int OnExit();
    
    //void SetLogger(wxLog *logger);
    //wxString ReplaceLogger();
    void SetLoggerTarget(SeruroLoggerTarget *log_target);
    wxString GetBufferedLog();
    
	wxWindow *GetFrame();
    
    /* Seruro State callback events. */
    void OnInvalidAuth(SeruroRequestEvent &event);

	/* Writing to the config. */
	wxCriticalSection seruro_critsection_config;
    /* Adding/removing from the thread pool. */
	wxCriticalSection seruro_critsection_thread;
    wxCriticalSection seruro_critsection_monitor;
    /* Writing/reading a token. */
	wxCriticalSection seruro_critsection_token;
    /* Writing to the log file. */
	wxCriticalSection seruro_critsection_log;
    /* Assigning and identity to an application. */
    wxCriticalSection seruro_critsection_assign;

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

    /* Monitoring functions. */
    void StartMonitor();
    void StopMonitor();
    /* Allow the monitor to reach in an remove the pointer. */
    void PauseMonitor();
    void ResumeMonitor();	
    void DeleteMonitor() {
        this->seruro_monitor = NULL;
    }
    
private:
    /* Abstraction for instance limiter. */
    bool IsAnotherRunning();
    
    /* Logging functions. */
	void InitLogger();
    
	/* Shows a message and prompts to send an error report. */
	void ReportAndExit(wxJSONValue report, 
		wxString msg = wxEmptyString, bool close_app = true);

	SeruroFrameMain *main_frame;
    SeruroLogger *default_logger;
    
    /* A "monitoring" thread which polls for various external updates. */
    SeruroMonitor *seruro_monitor;
    
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
