
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include <wx/wizard.h>
#include <wx/checkbox.h>

#include "../wxJSON/wx/jsonval.h"

#include "../SeruroClient.h"
#include "../api/SeruroServerAPI.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
#include "../frames/dialogs/AddAccountDialog.h"

class SeruroSetup;

class SetupPage : public wxWizardPageSimple
{
public:
    SetupPage (wxWizard *parent) 
		: wxWizardPageSimple(parent), wizard((SeruroSetup*)parent) {}
	/* The 'GoForward' method is called when the user, or some callback
	 * function tries to proceed the wizard. If from_callback is true
	 * then don't issue another request. 
	 */
	virtual bool GoForward(bool from_callback = false) { return true; } 

	wxString next_button;
	wxString prev_button;
protected:
	/* Parent wizard */
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

	SetupPage* GetServerPage() { return server_page; }
	SetupPage* GetAccountPage() { return account_page; }
	bool HasServerInfo() { 
		return (! this->address_setup);
	}
    
    /* Over write without validation for backbutton */
    void GoBack(wxCommandEvent& event);
	void ForceForward();
	/* Restore/set button text when a new page is displayed. */
	void OnChanged(wxWizardEvent &event);
	void OnChanging(wxWizardEvent &event);

	/* Special functions for over-writing the button text (per-page). */
	void SetButtonText(wxString prev, wxString next);

private:
	/* If pages edit the button text, save the original values to reset. */
	wxString next_button_orig;
	wxString prev_button_orig;

	/* Allow the constructor to create pages based on a setup type. */
	bool server_setup;
	bool address_setup;

    SetupPage *initial_page;
    SetupPage *server_page;
    SetupPage *account_page;
	SetupPage *identity_page;
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
	bool GoForward(bool from_callback = false);

	void OnPingResult(SeruroRequestEvent &event);

private:
	bool login_success;

	DECLARE_EVENT_TABLE()
};

class IdentityPage : public SetupPage
{
public:
	IdentityPage (SeruroSetup *parent);

private:
	wxCheckBox *install_identity;

};

#endif
