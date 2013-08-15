
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include <wx/wizard.h>
//#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/textctrl.h>
#include <wx/event.h>

#include "../wxJSON/wx/jsonval.h"

#include "../SeruroClient.h"
#include "../api/SeruroServerAPI.h"
#include "../frames/UIDefs.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
#include "../frames/dialogs/AddAccountDialog.h"
#include "../frames/dialogs/DecryptDialog.h"

enum setup_type_t
{
	SERURO_SETUP_SERVER,
	SERURO_SETUP_ACCOUNT,
	SERURO_SETUP_IDENTITY,

	SERURO_SETUP_INITIAL
};

enum setup_ids_t
{
    SERURO_SETUP_ID
};

class SeruroSetup;

/* Helper fuction for pasting into various text controls. */
void PasteIntoControl(wxTextCtrl *control);

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
		setup_type_t type = SERURO_SETUP_INITIAL,
        wxString server_uuid = wxEmptyString,
        wxString address = wxEmptyString);
		//bool add_server= false, bool add_address= false);

    wxWizardPage *GetInitialPage() const { 
		return initial_page; 
	}

	//SetupPage* GetServerPage() { return server_page; }
	SetupPage* GetAccountPage() { return account_page; }
	bool IsNewServer() {
        /* Is there a server_uuid provided? */
		return (this->setup_type == SERURO_SETUP_INITIAL || this->setup_type == SERURO_SETUP_SERVER);
    }
    wxJSONValue GetServerInfo();
    wxString GetAccount();
    
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
	setup_type_t setup_type;

    SetupPage *initial_page;
    SetupPage *account_page;
	SetupPage *identity_page;
    SetupPage *applications_page;
    SetupPage *settings_page;
    
    /* Set by caller when setup begins after server or account. */
    wxString server_uuid;
    wxString account;

    DECLARE_EVENT_TABLE()
};

class InitialPage : public SetupPage
{
public:
    InitialPage (SeruroSetup *parent);
};

/* Should be very similar to /frames/dialogs/AddAccountDialog */
class AccountPage : public SetupPage, 
	public AddAccountForm, public AddServerForm
{
public:
    AccountPage (SeruroSetup *parent);
	bool GoNext(bool from_callback = false);

	/* API Call handlers. */
	void OnPingResult(SeruroRequestEvent &event);
	void OnCAResult(SeruroRequestEvent &event);

	/* Manage server/CA lookups. */
	void OnSelectServer(wxCommandEvent &event);
    void OnCustomPort(wxCommandEvent &event);
    void OnPastePassword(wxClipboardTextEvent& event);
    wxString GetServerUUID() { return this->server_uuid; }
    
	/* Check if the name from the server page changed (and reselect). */
	void DoFocus();

	/* Help update the UI to reflect callback actions. */
	void DisablePage();
	void EnablePage();

	/* Help update the UI to reflect callback status. */
	void SetAccountStatus(wxString status) {
		this->account_status->SetLabelText(status);
	}

private:
	bool login_success;
	bool has_ca;

	/* Server list is a choice, even if there is only one. */
	wxChoice *server_menu;
	wxString server_uuid;

	/* Textual messages indicating address/server install status. */
	//Text *server_status;
	Text *account_status;

	DECLARE_EVENT_TABLE()
};

class RequestPage  : public SetupPage
{
    /* The request page allows a user to generate and upload a digital identity CSR.
     * If the server has disabled the this feature then setup will bypass the request page.
     */
public:
    RequestPage (SeruroSetup *parent);
    
};

class IdentityPage : public SetupPage, public DecryptForm
{
public:
	IdentityPage (SeruroSetup *parent, 
		bool force_download = false);

	/* Check the 'install' identity box. */
	void OnP12sResponse(SeruroRequestEvent &event);
    void OnDownloadIdentity(wxCommandEvent &event);
	void OnPasteUnlock(wxClipboardTextEvent& event);
	
    /* Can be called from event or from a forced download. */
	void DownloadIdentity();

	/* Show the status, including the method of key retrevial. */
	void SetIdentityStatus(wxString status) {
		this->identity_status->SetLabelText(status);
	}
    
    /* The key form and download button are enabled/disabled. */
	void DoFocus();
    void DisablePage();
    void EnablePage();
    
    /* The user tries to install the identity 
	 * (after entering their key). */
    bool GoNext(bool from_callback = false);

private:
    wxButton *download_button;
	wxJSONValue download_response;
	
    /* The client's certificate store is queried at the response of a P12.
     * This determine's if 'install' should install none, both, either of the P12s.
     */
    bool install_encipherment;
    bool install_authentication;
    
    bool identity_downloaded;
	bool force_download;
	bool identity_installed;
	Text *identity_status;

	DECLARE_EVENT_TABLE()
};

class ApplicationsPage : public SetupPage
{
public:
	ApplicationsPage(SeruroSetup *parent) : SetupPage(parent) {}
};

#endif
