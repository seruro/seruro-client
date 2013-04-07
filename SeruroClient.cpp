
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroSetup.h"

#include "frames/SeruroFrames.h"

/* Boost */
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

/* Testing */
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    /* User config instance */
    this->config = new SeruroConfig();

	SeruroFrameMain *mainFrame = new SeruroFrameMain(wxT("Seruro Client"));
	mainFrame->Show(); /* for debugging */
    
    /* Logging, for debug or otherwise. */
    //wxLogWindow *logger = new wxLogWindow(mainFrame, wxT("Logger"));
    //logger->GetFrame()->SetWindowStyle(wxDEFAULT_FRAME_STYLE|wxSTAY_ON_TOP);
    //logger->GetFrame()->SetSize( wxRect(0,50,400,250) );
    //wxLog::SetActiveTarget(logger);
    //wxLogMessage(wxT("Seruro Client started."));

	/* There is an optional setup wizard. */
	if (! this->config->HasConfig()) {
		//SeruroSetup setup(mainFrame);
		//setup.RunWizard(setup.GetFirstPage());
	}

    return true;
}

SeruroPanel::SeruroPanel(wxBookCtrlBase *parent, const wxString& title) : wxPanel(parent, wxID_ANY)
{
	parent->AddPage(this, title, false, 0);
}

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    wxStandardPaths *paths = new wxStandardPaths();
    this->configFile = new wxTextFile(paths->GetUserDataDir() + wxT("/") + wxT(SERURO_CONFIG_NAME));
    
    wxLogMessage(wxT("Config file: " + this->configFile->GetName()));
    if (! HasConfig())
        wxLogMessage(wxT("Config does not exist"));
    /* May be a good idea to remove loadConfig, and put the code here. */
    this->LoadConfig();
}

void SeruroConfig::LoadConfig()
{
    if (! this->HasConfig())
        /* Cannot load a non-existing config file. */
        return;
    
    /* Read entire file into string. */
    wxString configString;
    configFile->Open();
    while (! configFile->Eof()) {
        configString += configFile->GetNextLine();
    }
    wxLogMessage(configString);
    
    /* Parse the file into JSON. */
}

bool SeruroConfig::HasConfig()
{
    return this->configFile->Exists();
}


