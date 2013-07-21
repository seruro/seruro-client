
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

#include <wx/event.h>

#include "../resources/images/logo_block_128_flat.png.h"

BEGIN_EVENT_TABLE(SeruroSetup, wxWizard)
    EVT_BUTTON(wxID_BACKWARD, SeruroSetup::GoPrev)
	EVT_WIZARD_PAGE_CHANGED(SERURO_SETUP_ID, SeruroSetup::OnChanged)
	EVT_WIZARD_BEFORE_PAGE_CHANGED(SERURO_SETUP_ID, SeruroSetup::OnChanging)
END_EVENT_TABLE()

DECLARE_APP(SeruroClient);

SetupPage::SetupPage(SeruroSetup *parent) : wxWizardPageSimple(parent), wizard(parent),
	enable_prev(true), require_auth(false)
{
	//this->enable_back = (! (! this->wizard->GetInitialPage())) // return;
	//if (this->wizard->HasPrevPage(this->wizard->GetCurrentPage())) {
		//this->wizard->EnableBack(true);
	//}
	//this->wizard->RequireAuth(false);
}

InitialPage::InitialPage(SeruroSetup *parent) : SetupPage(parent)
{
    /* Show welcome message, and overview of the workflow to follow. */
    wxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    this->enable_prev = false;
    
    Text *msg = new Text(this, wxT("Welcome to Seruro! Let's take a moment and configure your client.\n")
		wxT("\n")
        wxT("If this is your first time installing the Seruro Client, please pay attention as some settings may")
        wxT("affect your privacy settings. This initial setup wizard will guide you through:\n")
        wxT("\n")
            wxT("\t 1. Connecting to your Seruro Server.\n")
            wxT("\t 2. Configuring your account and downloading your identity.\n")
            wxT("\t 3. Automatic setup of your email applications.\n")
            wxT("\t 4. Retreival and installation of contact identities.\n")
        wxT("\n")
        wxT("This setup may be canceled and restarted at a later time. After completing the setup you may")
        wxT("add additional servers and accounts as well as change any setting options."));
    vert_sizer->Add(msg, DIALOGS_BOXSIZER_OPTIONS);
    
    this->SetSizer(vert_sizer);
}

//SeruroSetup::SeruroSetup(wxFrame *parent, bool add_server, bool add_address) : 
SeruroSetup::SeruroSetup(wxFrame *parent, setup_type_t type,
    wxString server, wxString account) : setup_type(type), server_name(server), account(account)
	//server_setup(type), address_setup(type)
{
	/* Set title based on type of setup. */
	wxString setup_title = wxString(_(SERURO_APP_NAME)) + _(" Setup");
	if (type == SERURO_SETUP_SERVER) setup_title = _("Add Server Setup");
	if (type == SERURO_SETUP_ACCOUNT) setup_title = _("Add Account Setup");
	if (type == SERURO_SETUP_IDENTITY) setup_title = _("Install Identity");

    wxIcon setup_icon;
    setup_icon.CopyFromBitmap(wxGetBitmapFromMemory(logo_block_128_flat));
    this->Create(parent, SERURO_SETUP_ID, setup_title,
        /* Todo: replace icon */
        setup_icon, wxDefaultPosition,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	/* Set the default values of the navigation buttons. */
	this->next_button_orig = this->m_btnNext->GetLabelText();
	this->prev_button_orig = this->m_btnPrev->GetLabelText();

    /* Page creation, a welcome page for the initial setup. */
	if (type == SERURO_SETUP_INITIAL) {
		this->initial_page  = new InitialPage(this);
        //this->server_page   = new ServerPage(this);
		this->account_page  = new AccountPage(this);

		initial_page->SetNext(account_page);
		//server_page->SetPrev(initial_page);
		//server_page->SetNext(account_page);
		account_page->SetPrev(account_page);
	}

	/* Only show if in the initial setup or a server setup. */
	if (type == SERURO_SETUP_SERVER) {
		//this->server_page   = new ServerPage(this);
		this->account_page  = new AccountPage(this);
		//this->initial_page  = this->server_page;
		this->initial_page  = this->account_page;

		//initial_page->SetNext(account_page);
		//account_page->SetPrev(initial_page);
	}

	if (type == SERURO_SETUP_ACCOUNT) {
		this->account_page  = new AccountPage(this);
		this->initial_page  = this->account_page;
	}

	/* Downloading an identity is automatic if this is the initial setup. */
	bool force_identity_download = (type == SERURO_SETUP_INITIAL);
	this->identity_page = new IdentityPage(this, force_identity_download);

	if (type != SERURO_SETUP_IDENTITY) {
		account_page->SetNext(identity_page);
		identity_page->SetPrev(account_page);
	} else {
		initial_page = identity_page;
	}

	/* Applications page will always exist. */
	this->applications_page = new ApplicationsPage(this);
	identity_page->SetNext(applications_page);
	applications_page->SetPrev(identity_page);

    this->GetPageAreaSizer()->Add(this->initial_page);
}

wxJSONValue SeruroSetup::GetServerInfo()
{
    if (setup_type == SERURO_SETUP_SERVER || setup_type == SERURO_SETUP_INITIAL) {
        return ((ServerPage*) this->GetAccountPage())->GetValues();
    }
    return wxGetApp().config->GetServer(this->server_name);
}

wxString SeruroSetup::GetAccount()
{
    if (setup_type != SERURO_SETUP_IDENTITY) {
        wxJSONValue account_values;
        account_values = ((AddAccountForm*)this->GetAccountPage())->GetValues();
        return account_values["address"].AsString();
    }
    return this->account;
}

/* Implemented in wxWizard at: wxwidgets\src\generic. 
 * This override for a button click allows the page to go backward even if there is
 * a form with invalid input (such as no input). */
void SeruroSetup::GoPrev(wxCommandEvent &event)
{
	/* Allow the page to react to a backward event (OnChanging). */
    wxWizardEvent eventPreChanged(wxEVT_WIZARD_BEFORE_PAGE_CHANGED, GetId(), false, m_page);
    (void)m_page->GetEventHandler()->ProcessEvent(eventPreChanged);
    
    if (!eventPreChanged.IsAllowed())
        return;
    
	/* Without validating (page->Validate()), show the previous */
    wxWizardPage *page;
    page = m_page->GetPrev();

    (void)ShowPage(page, false);
}

void SeruroSetup::ForceNext()
{
	/* If a callback is trying to force the forward action, the wizard must 
	 * process the event. */
	//wxCommandEvent event(wxEVT_COMMAND_BUTTON_CLICKED); 
	//event.SetId(wxID_FORWARD); 
	//this->ProcessEvent(event); 
	if (! this->HasNextPage(this->GetCurrentPage())) return;

	(void)ShowPage(this->GetCurrentPage()->GetNext(), true);
}

void SeruroSetup::OnChanging(wxWizardEvent &event)
{
	/* Don't worry if the page is going backward (for now). */
	if (! event.GetDirection() ) return;
	wxLogMessage(_("SeruroSetup> (OnChanging) the page is trying to move forward."));

	/* The PAGE_CHANGING event will have taken care of the generic (back/forward) 
	 * events which include checking the page for validation errors.
	 */

	/* This page may not allow us to proceed. */
	if (! ((SetupPage*) event.GetPage())->GoNext()) {
		wxLogMessage(_("SeruroSetup> (OnChanging) this page prevented the wizard form moving next."));
		event.Veto();
	}
	/* Continue normally... */
}

/* Catch the wizard when a new page is displayed (to update UI elements). */
void SeruroSetup::OnChanged(wxWizardEvent &event)
{
	wxLogMessage(_("SeruroSetup> (OnChanged) the page has changed."));
	SetupPage *shown_page = (SetupPage*) event.GetPage();

	/* Decorate the buttons */
	//if (shown_page->enable_back) 
	this->EnablePrev(shown_page->enable_prev);
    this->EnableNext(shown_page->enable_next);
	this->RequireAuth(shown_page->require_auth);

	/* Let us take care of the text. */
	this->SetButtonText(shown_page->prev_button, shown_page->next_button);

	/* Finally, each page may update it's content with 'DoFocus'. */
	shown_page->DoFocus();
}

/* Allow each page to set the wizards prev/next button text. */
void SeruroSetup::SetButtonText(wxString prev, wxString next)
{
	if (next.compare(wxEmptyString) == 0) {
		if (! this->HasNextPage(this->GetCurrentPage())) {
			this->m_btnNext->SetLabel(_("&Finish"));
		} else {
			this->m_btnNext->SetLabel(next_button_orig);
		}
	} else {
		this->m_btnNext->SetLabel(next);
	}

	if (prev.compare(wxEmptyString) == 0) {
		this->m_btnPrev->SetLabel(prev_button_orig);
	} else {
		this->m_btnPrev->SetLabel(prev);
	}
}
