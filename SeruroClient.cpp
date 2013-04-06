
//#include <boost/thread.hpp>

#include "SeruroClient.h"
#include "SeruroSetup.h"

#include "frames/SeruroFrames.h"

/* Boost */
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

IMPLEMENT_APP(SeruroClient)

bool SeruroClient::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

	this->FindConfig();

	SeruroFrameMain *mainFrame = new SeruroFrameMain(wxT("Seruro Client"));
	mainFrame->Show(); /* for debugging */

	/* There is an optional setup wizard. */
	if (! this->hasConfig) {
		SeruroSetup setup(mainFrame);
		setup.RunWizard(setup.GetFirstPage());
	}

    return true;
}

SeruroPanel::SeruroPanel(wxBookCtrlBase *parent, const wxString& title) : wxPanel(parent, wxID_ANY)
{
	parent->AddPage(this, title, false, 0);
}

void SeruroClient::FindConfig()
{
	this->hasConfig = true;
	boost::filesystem::path exePath(std::string(this->argv[0]));
	boost::filesystem::path configFile("SeruroClient.config");
	boost::filesystem::ofstream log("SeruroClient.log");
	//log << exePath.parent_path().string() << std::endl;
	//std::cout << configFile.is_relative() << std::endl;
	if (! boost::filesystem::exists(exePath.parent_path() / configFile)) {
		/* The client will prompt the user with a 'SetupWizard' to create a config. */
		/* This is abnormal, clients should generate a config during install. */		  
		this->hasConfig = false;
		return;
	}
}


