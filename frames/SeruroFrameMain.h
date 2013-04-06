
#ifndef H_SeruroFrameMain
#define H_SeruroFrameMain

#include <wx/wizard.h>

#include "../SeruroClient.h"

// IDs for the controls and the menu commands
enum
{
    Event_Quit = wxID_EXIT,
    Event_About = wxID_ABOUT,
};

class SeruroTray;

class SeruroFrameMain : public SeruroFrame
{
public:
	SeruroFrameMain(const wxString &title);

	void OnIconize(wxIconizeEvent& event);
	void OnQuit(wxCommandEvent &event);
	void OnClose(wxCloseEvent &event);

	/* Optional setup events */
	void OnSetupRun(wxCommandEvent &event);
    void OnSetupCancel(wxWizardEvent& event);
    void OnSetupFinished(wxWizardEvent& event);

protected:
	SeruroTray *tray;

private:
	DECLARE_EVENT_TABLE()
};

#endif
