#pragma once

#include "wx/wxprec.h"
#include <wx/wx.h>

#include <wx/icon.h>

#if defined(__VLD__)
#include <vld.h>
#endif

#ifndef H_SeruroClient
#define H_SeruroClient

class SeruroTray;

// IDs for the controls and the menu commands
enum
{
    Event_Quit = wxID_EXIT,
    Event_About = wxID_ABOUT,
};

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

};

// Define a new frame type: this is going to be our main frame
class SeruroFrame : public wxFrame
{
public:
    // ctor(s)
    SeruroFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
	void OnClose(wxCloseEvent &event);
	void OnIconize(wxIconizeEvent& event);
	void OnQuit(wxCommandEvent& event);

protected:
	SeruroTray *tray;

private:
    DECLARE_EVENT_TABLE()
};

#endif