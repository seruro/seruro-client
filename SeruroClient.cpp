
//#include <boost/thread.hpp>

#include "SeruroClient.h"

#include "frames/SeruroFrames.h"

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	SeruroFrameMain *mainFrame = new SeruroFrameMain(wxT("Seruro Client"));
	mainFrame->Show(); /* for debugging */

    return true;
}

SeruroPanel::SeruroPanel(wxBookCtrlBase *parent, const wxString& title) : wxPanel(parent, wxID_ANY)
{
	parent->AddPage(this, title, false, 0);
}


