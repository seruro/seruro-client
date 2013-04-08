
#ifndef H_SeruroFrameMain
#define H_SeruroFrameMain

#include <wx/wizard.h>

#include "SeruroFrame.h"

// IDs for the controls and the menu commands
enum
{
    Event_Quit = wxID_EXIT,
    Event_About = wxID_ABOUT,
};

/* From SeruroTray */
class SeruroTray;

enum tray_option_t;

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

	void ChangePanel(tray_option_t page);

protected:
	SeruroTray *tray;
	wxNotebook *book;

private:
	DECLARE_EVENT_TABLE()
};

#endif
