
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include "wx/wizard.h"
#include "wx/textctrl.h"

#include "../wxJSON/wx/jsonval.h"

#include "../SeruroClient.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
#include "../frames/dialogs/AddAccountDialog.h"
//#include "frames/SeruroFrameMain.h"

enum seruro_setup_ids
{
	//seruroID_SETUP_ALERT,
	//seruroID_SETUP_DOWNLOAD,
	//seruroID_SETUP_DECRYPT
    SETUP_SERVER_ID,
    SETUP_ACCOUNT_ID,
    SETUP_APPLICATIONS_ID,
    SETUP_SETTINGS_ID
};

class SeruroSetup;

class SetupPage : public wxWizardPageSimple
{
public:
    SetupPage (wxWizard *parent) : 
	  wxWizardPageSimple(parent), wizard((SeruroSetup*)parent) {}
    //virtual wxJSONValue GetValues();
private:
	wxString next_button;
	wxString prev_button;
	SeruroSetup *wizard;
};

class SeruroSetup : public wxWizard
{
public:
	/* Todo: disable "Exit" while wizard is running. */

	/* The setup wizard may have three initialization states, 
	 * for the first account the user adds (their first setup 
	 * of Seruro), for a new server, and for a new account (address).
	 */
	SeruroSetup(wxFrame *parent, 
		bool add_server= false, bool add_address= false);

    wxWizardPage *GetInitialPage() const { 
		return initial_page; 
	}
    
    /* Over write without validation for backbutton */
    void GoBack(wxCommandEvent& event);

	/* Special functions for over-writing the button text (per-page). */
	void SetButtonText(wxString back, wxString next) {
		if (next.compare(wxEmptyString) == 0) {
			this->m_btnNext->SetLabel(next_button_orig);
		} else {
			this->m_btnNext->SetLabel(next);
		}

		if (back.compare(wxEmptyString) == 0) {
			this->m_btnPrev->SetLabel(next_button_orig);
		} else {
			this->m_btnPrev->SetLabel(back);
		}
	}

private:
	/* If pages edit the button text, save the original values to reset. */
	wxString next_button_orig;
	wxString prev_button_orig;

	/* Allow the constructor to create pages based on a setup type. */
	bool server_setup;
	bool address_setup;
	//wxWizardPageSimple *manualConfigPage;
	//wxWizardPageSimple *downloadP12Page;
	//wxWizardPageSimple *decryptP12Page;
    SetupPage *initial_page;
    
    SetupPage *server_page;
    SetupPage *account_page;
    SetupPage *applications_page;
    SetupPage *settings_page;
    
    DECLARE_EVENT_TABLE()
};

class InitialPage : public SetupPage
{
public:
    InitialPage (SeruroSetup *parent);
};

/* Should be very similar to /frames/dialogs/AddServerDialog */
class ServerPage : public SetupPage, public AddServerForm
{
public:
    ServerPage(SeruroSetup *parent);

	/* Handle the single checkbox click. */
	void OnForm_OnCustomPort(wxCommandEvent &event);

private:
	DECLARE_EVENT_TABLE()
};

/* Should be very similar to /frames/dialogs/AddAccountDialog */
class AccountPage : public SetupPage, public AddAccountForm
{
public:
    AccountPage (SeruroSetup *parent);
};

#endif
