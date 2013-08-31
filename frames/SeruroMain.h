
#ifndef H_SeruroFrameMain
#define H_SeruroFrameMain

#include <wx/wizard.h>
#include <wx/notebook.h>

#include "SeruroFrame.h"
#include "../SeruroTray.h"

#include "../api/SeruroStateEvents.h"

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

	/* Setup events */
    void StartSetup();
    void StopSetup();
	void OnSetupRun(wxCommandEvent &event);
    void OnSetupCancel(wxWizardEvent& event);
    void OnSetupFinished(wxWizardEvent& event);

	/* When the panels change, their UI elements may need updating. */
	void OnChange(wxBookCtrlEvent &event);

	void ChangePanel(int panel_id);
    
    /* Wait for "auto_download" option changes. */
    void OnOptionChange(SeruroStateEvent &event);

protected:
	SeruroTray *tray;
	wxNotebook *book;
    
    wxPanel *contacts_panel;
    wxPanel *search_panel;
    wxPanel *settings_panel;
#if SERURO_ENABLE_CRYPT_PANELS
	wxPanel *encrypt;
	wxPanel *decrypt;
#endif
#if SERURO_ENABLE_DEBUG_PANELS
    wxPanel *test_panel;
#endif

private:
    /* An instance of a running setup. */
    wxTopLevelWindow *setup;
    
	DECLARE_EVENT_TABLE()
};

#endif
