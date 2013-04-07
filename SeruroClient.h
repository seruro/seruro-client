#pragma once

#ifndef H_SeruroClient
#define H_SeruroClient

#define SERURO_CONFIG_NAME  "SeruroClient.config"
#define SERURO_APP_NAME     "Seruro Client"

#include "wx/wxprec.h"
#include <wx/wx.h>

#include <wx/notebook.h>

/* Remove when finished developing. */
#ifndef __WXDEBUG__
#define __WXDEBUG__ 1
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#if defined(__VLD__)
//#include <vld.h>
#endif

/* Sample icon to use for everything while testing. */
#include "resources/icon_good.xpm"

class SeruroConfig;
class SeruroFrameMain;

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    
    /* Run networking thread from OnInit() */
    virtual bool OnInit();

	void InitLogger();

private:
	SeruroConfig *config;
	SeruroFrameMain *mainFrame;
};

class SeruroFrame : public wxFrame
{
public:
	SeruroFrame(const wxString &title) : wxFrame(NULL, wxID_ANY, title) {}
};

class SeruroPanel : public wxPanel
{
public:
	SeruroPanel(wxBookCtrlBase *parent, const wxString &title) :
	  wxPanel(parent, wxID_ANY) {
		  /* Todo, the last param is the imageID, it's currently static. */
		  parent->AddPage(this, title, false, 0);
	  }
};

#endif