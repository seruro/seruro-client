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

/* Inlcude the Config header so all classes may use wxGetApp().config. */
#include "SeruroConfig.h"
#include "SeruroLogger.h"

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
		delete config;
		return 0;
	}

	void InitLogger();
    void SetLogger(wxLog *logger);
    wxString ReplaceLogger();
    wxString GetLog();
    
	wxWindow *GetFrame();
    
    void OnInvalidAuth(SeruroRequestEvent &event);

	/* Todo: Consider accessor methods */
	wxCriticalSection seruro_critSection;
	wxArrayThread seruro_threads;
	//wxSemaphore seruro_semFinished;

	void AddEvent(wxEvent &event);

public:
	SeruroConfig *config;

private:
	SeruroFrameMain *main_frame;
    SeruroLogger *default_logger;
};

/* Hack for accessing app instance singleton */
//const SeruroClient *wxTheSeruro = (SeruroClient *) wxTheApp;
/* See documentation for wxGetApp() and the DECLARE_APP macro */

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
