
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include <wx/wizard.h>
//#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/textctrl.h>

#include "../wxJSON/wx/jsonval.h"

#include "../SeruroClient.h"
#include "../api/SeruroServerAPI.h"
#include "../frames/UIDefs.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
#include "../frames/dialogs/AddAccountDialog.h"

class SeruroSetup;

class SetupPage : public wxWizardPageSimple
{
public:
    SetupPage (SeruroSetup *parent);
	/* The 'GoForward' method is called when the user, or some callback
	 * function tries to proceed the wizard. If from_callback is true
	 * then don't issue another request. 
	 */
	virtual bool GoNext(bool from_callback = false) { return true; }
	virtual void DoFocus() { return; }

	wxString next_button;
	wxString prev_button;
	bool enable_prev;
    bool enable_next;
	bool require_auth;
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
    void GoPrev(wxCommandEvent& event);
	void ForceNext();
	/* Restore/set button text when a new page is displayed. */
	void OnChanged(wxWizardEvent &event);
	void OnChanging(wxWizardEvent &event);

	/* Special functions for over-writing the button text (per-page). */
	void SetButtonText(wxString prev = wxEmptyString, 
		wxString next = wxEmptyString);
	void EnablePrev(bool enable) {
		//this->m_btnPrev->Disable();
		if (this->HasPrevPage(this->GetCurrentPage())) {
			/* Only permit override if previous page exists. */
			this->m_btnPrev->Enable(enable);
		} else {
			this->m_btnPrev->Enable(false);
		}
	}
	void EnableNext(bool enable) {
		this->m_btnNext->Enable(enable);
	}
	/* Since this process will potentially install a CA, optionally
	 * decorate the action button with an auth symbol. */
	void RequireAuth(bool require) {
		/* Testing, might not look nice on NON-MSW. */
		this->m_btnNext->SetAuthNeeded(require);
	}

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
	bool GoNext(bool from_callback = false);

	/* API Call handlers. */
	void OnPingResult(SeruroRequestEvent &event);
	void OnCAResult(SeruroRequestEvent &event);

	/* Manage server/CA lookups. */
	void OnSelectServer(wxCommandEvent &event);
	bool HasServerCertificate(wxString server_name);
	/* Check if the name from the server page changed (and reselect). */
	void DoFocus();

	/* Help update the UI to reflect callback actions. */
	void DisablePage();
	void EnablePage();

	/* Help update the UI to reflect callback status. */
	void SetServerStatus(wxString status) {
		this->server_status->SetLabelText(status);
	}
	void SetAccountStatus(wxString status) {
		this->account_status->SetLabelText(status);
	}

private:
	bool login_success;
	bool has_ca;

	/* Server list is a choice, even if there is only one. */
	wxChoice *server_menu;
	wxString server_name;

	/* Textual messages indicating address/server install status. */
	Text *server_status;
	Text *account_status;

	DECLARE_EVENT_TABLE()
};

class IdentityPage : public SetupPage
{
public:
	IdentityPage (SeruroSetup *parent, bool force_install = false);

	/* Check the 'install' identity box. */
	//void OnToggleInstall(wxCommandEvent &event);
    void OnP12sResponse(SeruroRequestEvent &event);
    void OnDownloadIdentity(wxCommandEvent &event);
    
    /* The key form and download button are enabled/disabled. */
    void DisablePage();
    void EnablePage();
    
    /* The user tries to install the identity (after entering their key). */
    bool GoNext(bool from_callback = false);

private:
    wxButton *download_identity;
	//wxCheckBox *install_identity;
    
    bool identity_downloaded;

	DECLARE_EVENT_TABLE()
};

#endif
