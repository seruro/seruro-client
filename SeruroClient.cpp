// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"
//C:\Program Files %28x86%29\Visual Leak Detector\lib\Win32
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "SeruroClient.h"
#include "SeruroConfig.h"

#include "crypto/SeruroCrypto.h"
#include "setup/SeruroSetup.h"
#include "api/SeruroRequest.h"

#include "frames/UIDefs.h"
#include "frames/SeruroFrameMain.h"

#include <wx/log.h>
#include <wx/image.h>

#define SERURO_DEBUG_SETUP 0

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

	/* Support for PNGs. */
	wxImage::AddHandler(new wxPNGHandler);

	/* Create a frame, but do not start sub frames, which may depend on config. */
	mainFrame = new SeruroFrameMain(wxT(SERURO_APP_NAME), 
		SERURO_APP_DEFAULT_WIDTH, SERURO_APP_DEFAULT_HEIGHT);
    this->SetTopWindow(mainFrame);

	/* Start logger */
	InitLogger();

	/* User config instance */
    this->config = new SeruroConfig();
    
    /* Listen for invalid request events (which require UI actions and a request-restart). */
    Bind(SERURO_REQUEST_RESULT, &SeruroClient::OnInvalidAuth, this, SERURO_REQUEST_CALLBACK_AUTH);

	/* Now safe to start sub-frames (panels). */
	mainFrame->AddPanels();

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

void SeruroClient::AddEvent(wxEvent &event)
{
	this->AddPendingEvent(event);
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

/*****************************************************************************************/
/************** GLOBAL REQUEST HANDLERS **************************************************/
/*****************************************************************************************/

void SeruroClient::OnInvalidAuth(SeruroRequestEvent &event)
{
    /* Perform UI actions, then create identical request. */
    //SeruroRequest(wxJSONValue params, wxEvtHandler *parent, int parentEvtId);
    
    /* If the user fails or cancels within the UI actions, send failed event from request. */
}
