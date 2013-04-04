
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroTray.h"

#include "resources/icon_good.xpm"

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_MENU	(Event_Quit,	MainFrame::OnQuit)
    EVT_MENU	(Event_About,	MainFrame::OnAbout)
	// When the window is minimized
	EVT_ICONIZE	(				MainFrame::OnIconize)
	// When the X button is used:
	EVT_CLOSE	(				MainFrame::OnClose)
END_EVENT_TABLE()


IMPLEMENT_APP(SeruroClient)


bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    MainFrame *frame = new MainFrame(wxT("Seruro Client"));

    frame->Show(true);

    return true;
}

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
	tray = new SeruroTray();
	tray->SetMainFrame(this);

	#if defined(__WXMSW__)
		SetIcon(wxICON(main));
        tray->SetIcon(wxICON(main), wxT("Seruro Client"));
	#endif
    
    /* Round-about way of setting tray icon */
    #if defined(__WXMAC__)
        tray->SetIcon(wxIcon(icon_good), wxT("Seruro Client"));
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
