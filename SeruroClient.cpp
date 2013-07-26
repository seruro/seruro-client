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

/* Note: SeruroConfig/SeruroLogger are placed in the header. */

#include "crypto/SeruroCrypto.h"
#include "setup/SeruroSetup.h"
#include "api/SeruroRequest.h"

#include "frames/UIDefs.h"
#include "frames/SeruroFrameMain.h"

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
	main_frame = new SeruroFrameMain(wxT(SERURO_APP_NAME),
		SERURO_APP_DEFAULT_WIDTH, SERURO_APP_DEFAULT_HEIGHT);
    this->SetTopWindow(main_frame);

	/* Start logger */
	InitLogger();

	/* User config instance */
    this->config = new SeruroConfig();
    
    /* Listen for invalid request events (which require UI actions and a request-restart). */
    Bind(SERURO_REQUEST_RESPONSE, &SeruroClient::OnInvalidAuth, this, SERURO_REQUEST_CALLBACK_AUTH);

	/* Now safe to start sub-frames (panels). */
	main_frame->AddPanels();

	/* There is an optional setup wizard. */
	if (! this->config->HasConfig() || SERURO_DEBUG_SETUP) {
        /* The panels and "book view" should be hidden while the wizard is running. */
        main_frame->Hide();
        
        SeruroSetup setup(main_frame);
		setup.RunWizard(setup.GetInitialPage());
	} else {
        main_frame->Show(true);
    }

    return true;
}

void SeruroClient::AddEvent(wxEvent &event)
{
	this->AddPendingEvent(event);
}

wxWindow* SeruroClient::GetFrame()
{
	return (wxWindow *) this->main_frame;
}

wxString SeruroClient::ReplaceLogger()
{
    /* Remove the existing one while returning its buffer. */
    wxString log_buffer = this->default_logger->GetBuffer();
    delete this->default_logger;
    
    return log_buffer;
}

void SeruroClient::SetLogger(wxLog* logger)
{
    /* Wrapper to allow other classes to change the logger. */
    wxLog::SetActiveTarget(logger);
}

void SeruroClient::InitLogger()
{
    if (SERURO_USE_LOGGER) {
        this->default_logger = new SeruroLogger();
        this->default_logger->InitLogger();
            
        this->SetLogger(this->default_logger);

        /* Tell the logger to buffer input. */
        if (SERURO_USE_SETTINGSLOG) {
            this->default_logger->ToggleBuffer();
        }
    } else {
        wxLogWindow *logger = new wxLogWindow(this->main_frame, wxT("Logger"));

        logger->GetFrame()->SetWindowStyle(wxDEFAULT_FRAME_STYLE);
        logger->GetFrame()->SetSize( wxRect(700,100,700,700) );
        
        this->SetLogger(logger);
    }
        
#if defined(__WXDEBUG__)
    wxLog::SetLogLevel(wxLOG_Debug);
    wxLog::SetVerbose(true);
    LOG(_("Seruro (debug-build) started."));
#else
    LOG(wxT("Seruro started."));
#endif
}

/*****************************************************************************************/
/************** GLOBAL REQUEST HANDLERS **************************************************/
/*****************************************************************************************/

void SeruroClient::OnInvalidAuth(SeruroRequestEvent &event)
{
    /* (From api/SeruroRequest) Perform UI actions, then create identical request. */
    PerformRequestAuth(event);
}
