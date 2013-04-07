
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroSetup.h"
#include "SeruroConfig.h"

#include "frames/SeruroFrames.h"

#include <wx/log.h>

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	mainFrame = new SeruroFrameMain(wxT("Seruro Client"));

	/* Start logger */
	InitLogger();

	/* User config instance */
    this->config = new SeruroConfig();

	mainFrame->Show(); /* for debugging */

	/* There is an optional setup wizard. */
	if (! this->config->HasConfig()) {
		SeruroSetup setup(mainFrame);
		setup.RunWizard(setup.GetFirstPage());
	}

    return true;
}

void SeruroClient::InitLogger()
{
	wxLogWindow *logger = new wxLogWindow(this->mainFrame, wxT("Logger"));

    logger->GetFrame()->SetWindowStyle(wxDEFAULT_FRAME_STYLE|wxSTAY_ON_TOP);
    logger->GetFrame()->SetSize( wxRect(0,50,400,250) );
    wxLog::SetActiveTarget(logger);
    wxLogStatus(wxT("Seruro Client started."));
}