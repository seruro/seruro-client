
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroTray.h"
#include "frames/SeruroFrameConfigure.h"

#include "resources/icon_good.xpm"

BEGIN_EVENT_TABLE(SeruroFrame, wxFrame)
    EVT_MENU	(Event_Quit,	SeruroFrame::OnQuit)
    //EVT_MENU	(Event_About,	SeruroFrame::OnAbout)
	// When the window is minimized
	EVT_ICONIZE	(				SeruroFrame::OnIconize)
	// When the X button is used:
	EVT_CLOSE	(				SeruroFrame::OnClose)
END_EVENT_TABLE()

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    SeruroFrameConfigure *frame = new SeruroFrameConfigure(wxT("Seruro Client: Configure"));

    frame->Show(true);

    return true;
}

SeruroFrame::SeruroFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title)
{
	#if defined(__WXMSW__)
		SetIcon(wxICON(main));
	#endif
}

void SeruroFrame::OnClose(wxCloseEvent &event)
{
	if (event.CanVeto()) {
		Show(false);
		event.Veto();
		return;
	}
	/* Using the "QUIT" */
	Destroy();
}

void SeruroFrame::OnIconize(wxIconizeEvent& WXUNUSED(event))
{
	// Remove program from taskbar
	Hide();
}

void SeruroFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    Close(true);
}
