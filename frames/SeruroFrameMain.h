
#ifndef H_SeruroFrameMain
#define H_SeruroFrameMain

#include <wx/wizard.h>

#include "SeruroFrame.h"
#include "../SeruroTray.h"

/* From SeruroTray */
class SeruroTray;

//extern enum tray_option_t;

class SeruroFrameMain : public SeruroFrame
{
public:
	SeruroFrameMain(const wxString &title, 
		int width, int height);

	void AddPanels();

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
    
    wxPanel *search_panel;
    wxPanel *settings_panel;
    wxPanel *test_panel;

private:
	DECLARE_EVENT_TABLE()
};

#endif
