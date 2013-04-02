#pragma once

#include "wx/wxprec.h"

#include <wx/icon.h>

class SeruroTray;

// IDs for the controls and the menu commands
enum
{
    // menu items
    Event_Quit = wxID_EXIT,

    // it is important for the id corresponding to the "About" command to have
    // this standard value as otherwise it won't be handled properly under Mac
    // (where it is special and put into the "Apple" menu)
    Event_About = wxID_ABOUT,
};

// Define a new application type, each program should derive a class from wxApp
class SeruroClient : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

};

// Define a new frame type: this is going to be our main frame
class MainFrame : public wxFrame
{
public:
    // ctor(s)
    MainFrame(const wxString& title);

    // event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
	void MainFrame::OnClose(wxCloseEvent &event);
    void OnAbout(wxCommandEvent& event);
	void OnIconize(wxIconizeEvent& event);

protected:
	SeruroTray *tray;
	
	//wxIcon SeruroIcon;
	//static const wxString SeruroIconFile;

private:
    // any class wishing to process wxWidgets events must use this macro
    DECLARE_EVENT_TABLE()
};
