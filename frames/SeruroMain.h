
#ifndef H_SeruroFrameMain
#define H_SeruroFrameMain

#include <wx/wizard.h>
#include <wx/notebook.h>

#include "SeruroFrame.h"
#include "../SeruroTray.h"

#include "../api/SeruroStateEvents.h"

enum extra_panel_ids_t
{
    SERURO_PANEL_HOME_ID = 1024,
    SERURO_PANEL_HELP_ID = 1025
};


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
    void StartSetup(bool force = true);
    void StopSetup();
	//void OnSetupRun(wxCommandEvent &event);
    void OnSetupCancel(wxWizardEvent& event);
    void OnSetupFinished(wxWizardEvent& event);
	bool IsSetupRunning() {
		return this->setup_running;
	}

	/* When the panels change, their UI elements may need updating. */
	void OnChange(wxBookCtrlEvent &event);

	void ChangePanel(int panel_id);
    
    /* Wait for "auto_download" option changes. */
    void OnOptionChange(SeruroStateEvent &event);
	void OnServerStateChange(SeruroStateEvent &event);

protected:
	SeruroTray *tray;
	wxNotebook *book;
    
    wxPanel *contacts_panel;
    wxPanel *search_panel;
    wxPanel *settings_panel;
    
    wxPanel *home_panel;
    wxPanel *help_panel;
    
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
	bool setup_running;
    
	DECLARE_EVENT_TABLE()
};

#endif
