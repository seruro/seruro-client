#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#include "Defs.h"

#include <wx/wxprec.h>
#include <wx/wx.h>

#include <wx/notebook.h>
#include <wx/thread.h>

/* Sample icon to use for everything while testing. */
#include "resources/icon_good.xpm"
#include "SeruroConfig.h"

//class SeruroConfig;
class SeruroFrameMain;

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

	/* Todo: Consider accessor methods */
	wxCriticalSection seruro_critSection;
	wxArrayThread seruro_threads;
	//wxSemaphore seruro_semFinished;

public:
	SeruroConfig *config;

private:
	SeruroFrameMain *mainFrame;
};

/* Hack for accessing app instance singleton */
//const SeruroClient *wxTheSeruro = (SeruroClient *) wxTheApp;
/* See documentation for wxGetApp() and the DECLARE_APP macro */

#endif

#if defined(__VLD__)
	#undef GetClassInfoW
	#include <wx/msw/winundef.h>
	#include <vld.h>
#else
	void VLDDisable (void) {}
	void VLDEnable (void) {}
#endif