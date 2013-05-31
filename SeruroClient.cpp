
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroConfig.h"

#include "crypto/SeruroCrypto.h"
#include "setup/SeruroSetup.h"

#include "frames/UIDefs.h"
#include "frames/SeruroFrameMain.h"

#include <wx/log.h>

#define SERURO_DEBUG_SETUP 1

#if !defined(__VLD__)
    /* (Hack) If the Virtual Leak Detector is not enabled (a MSW DLL only) then create dummy
     * symbols for control functions used in this application. */
    void VLDDisable (void) {}
    void VLDEnable (void) {}
#endif

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	/* Create a frame, but do not start sub frames, which may depend on config. */
	mainFrame = new SeruroFrameMain(wxT(SERURO_APP_NAME), 
		SERURO_APP_DEFAULT_WIDTH, SERURO_APP_DEFAULT_HEIGHT);

	/* Start logger */
//#if defined(__DEBUG__) || defined(__WXDEBUG__)
	InitLogger();
//#endif

	/* User config instance */
    this->config = new SeruroConfig();

	/* Now safe to start sub-frames (panels). */
	mainFrame->AddPanels();

	//wxArrayString servers = config->GetServers();
	//wxLogStatus(servers[0]);

	//mainFrame->Show(); /* for debugging */

	/* There is an optional setup wizard. */
	if (! this->config->HasConfig() || SERURO_DEBUG_SETUP) {
        /* The panels and "book view" should be hidden while the wizard is running. */
        mainFrame->Show(false);
        
        SeruroSetup setup(mainFrame);
		setup.RunWizard(setup.GetInitialPage());
	} else {
        mainFrame->Show(true);
    }

    return true;
}

wxWindow* SeruroClient::GetFrame()
{
	return (wxWindow *) this->mainFrame;
}

void SeruroClient::InitLogger()
{
	wxLogWindow *logger = new wxLogWindow(this->mainFrame, wxT("Logger"));

    logger->GetFrame()->SetWindowStyle(wxDEFAULT_FRAME_STYLE);
    logger->GetFrame()->SetSize( wxRect(800,350,500,500) );
    wxLog::SetActiveTarget(logger);
    wxLogStatus(wxT("Seruro Client started."));
}
