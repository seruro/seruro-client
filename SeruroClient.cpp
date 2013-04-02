
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroTray.h"


// ----------------------------------------------------------------------------
// event tables and other macros for wxWidgets
// ----------------------------------------------------------------------------

// the event tables connect the wxWidgets events with the functions (event
// handlers) which process them. It can be also done at run-time, but for the
// simple menu events like this the static method is much simpler.
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU	(Event_Quit,	MainFrame::OnQuit)
    EVT_MENU	(Event_About,	MainFrame::OnAbout)
	// When the window is minimized
	EVT_ICONIZE	(				MainFrame::OnIconize)
	// When the X button is used:
	EVT_CLOSE	(				MainFrame::OnClose)
END_EVENT_TABLE()

// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. SeruroClient and
// not wxApp)
IMPLEMENT_APP(SeruroClient)

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
bool SeruroClient::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

    // create the main application window
    MainFrame *frame = new MainFrame(wxT("Seruro Client"));

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

//const wxString MainFrame::SeruroIconFile = wxT("resources\\icon_good.png");

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	#if defined(__WXMSW__)
		SetIcon(wxICON(main));
		tray->SetIcon(wxICON(main));
	#endif

#if wxUSE_MENUS
    // create a menu bar
    wxMenu *fileMenu = new wxMenu;

    // the "About" item should be in the help menu
    wxMenu *helpMenu = new wxMenu;
    helpMenu->Append(Event_About, wxT("&About\tF1"), wxT("About Seruro"));

    fileMenu->Append(Event_Quit, wxT("E&xit\tAlt-X"), wxT("Quit"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(helpMenu, wxT("&Help"));

    // ... and attach this menu bar to the frame
    SetMenuBar(menuBar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(2);
    SetStatusText(wxT("Seuro Client version 0.1"));
#endif // wxUSE_STATUSBAR

	wxBoxSizer *VertSizer, *HorzSizer;
	VertSizer = new wxBoxSizer(wxVERTICAL);
	HorzSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText *SearchLabel;
	SearchLabel = new wxStaticText(this, wxID_ANY, wxT("Name or Email:"), wxDefaultPosition, wxDefaultSize, 0);
	HorzSizer->Add(SearchLabel, wxRIGHT, 5);

	//SearchControl = new wxSearchCtrl

}


// event handlers

// The user "closes" the window frame, which causes the app to continue to persist in the system tray
// This is not a "quit" app event.
void MainFrame::OnClose(wxCloseEvent &event)
{
	if (event.CanVeto()) {
		Show(false);
		event.Veto();
		return;
	}

	// Problem
	tray->RemoveIcon();
	tray->Destroy();
	Destroy();
}

void MainFrame::OnIconize(wxIconizeEvent& WXUNUSED(event))
{
	// Remove program from taskbar
	Hide();
}

void MainFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}

void MainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageBox(wxString::Format(
		wxT("Welcome to %s!\n\nThis is the minimal wxWidgets sample\nrunning under %s."), 
		wxVERSION_STRING, wxGetOsDescription()), 
		wxT("About wxWidgets minimal sample"), wxOK | wxICON_INFORMATION, this);
}
