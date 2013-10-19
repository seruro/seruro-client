
#include "SeruroSetup.h"
#include "../logging/SeruroLogger.h"
//#include "../frames/UIDefs.h"

#include <wx/event.h>
#include <wx/clipbrd.h>
#include <wx/dataobj.h>

#include <wx/statbmp.h>

/* Initial side image, this will change on each step. */
#include "../resources/images/setup_full_step_0_flat.png.h"

/* Individual step images. */
#include "../resources/images/setup_connect_24.png.h"
#include "../resources/images/setup_unlock_24.png.h"
#include "../resources/images/setup_applications_24.png.h"
#include "../resources/images/setup_settings_24.png.h"


#if defined(__WXMSW__)
#define SETUP_TEXT_WRAP 400
#define SETUP_TAB_SIZE  20
#define SETUP_TEXT_PADDING 5
#define SETUP_TEXT_PADDING_OPTIONS wxLEFT | wxTOP
#elif defined(__WXOSX__) || defined(__WXMAC__)
#define SETUP_TEXT_WRAP 500
#define SETUP_TAB_SIZE 20
#define SETUP_TEXT_PADDING 5
#define SETUP_TEXT_PADDING_OPTIONS wxLEFT | wxTOP
#endif

#define TEXT_SETUP_WELCOME_PREAMBLE "Welcome to Seruro! \
Let's take a moment and configure your computer.\
\n\n\
If this is your first time installing Seruro, \
this setup wizard will guide you through the following steps:"

#define TEXT_SETUP_WELCOME_POSTAMBLE "This setup may be canceled and restarted at a later time. \
After completing the setup you may add additional Seruro servers and accounts."

#define TEXT_SETUP_WELCOME_STEP1 "Connect to a Seruro server."
#define TEXT_SETUP_WELCOME_STEP2 "Install your Seruro certificates."
#define TEXT_SETUP_WELCOME_STEP3 "Configure your email application(s)."
#define TEXT_SETUP_WELCOME_STEP4 "Download Seruro contacts and adjust settings."

BEGIN_EVENT_TABLE(SeruroSetup, wxWizard)
    EVT_BUTTON(wxID_BACKWARD, SeruroSetup::GoPrev)
	EVT_WIZARD_PAGE_CHANGED(SERURO_SETUP_ID, SeruroSetup::OnChanged)
	EVT_WIZARD_BEFORE_PAGE_CHANGED(SERURO_SETUP_ID, SeruroSetup::OnChanging)

    //EVT_WIZARD_CANCEL(SERURO_SETUP_ID,   SeruroSetup::OnCanceled)
    //EVT_WIZARD_FINISHED(SERURO_SETUP_ID, SeruroSetup::OnFinished)
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

void AddIconText(wxWindow *parent, wxSizer* sizer, const wxBitmap& icon, wxWindow *text)
{
    wxSizer *horz_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticBitmap *static_icon = new wxStaticBitmap(parent, wxID_ANY, icon);
    
    horz_sizer->Add(static_icon, DEFAULT_BOXSIZER_OPTIONS.Border(wxRIGHT, 10));
    horz_sizer->Add(text, wxALIGN_CENTER_VERTICAL|wxALL, 1);
    sizer->Add(horz_sizer, DEFAULT_BOXSIZER_OPTIONS);
}

InitialPage::InitialPage(SeruroSetup *parent) : SetupPage(parent)
{
    /* Show welcome message, and overview of the workflow to follow. */
    wxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    this->enable_prev = false;
    this->enable_next = true;
    
	/* Preamble welcome text. */
	Text *preamble_text = new Text(this, _(TEXT_SETUP_WELCOME_PREAMBLE), false);
	preamble_text->Wrap(SETUP_TEXT_WRAP);
    vert_sizer->Add(preamble_text, DEFAULT_BOXSIZER_OPTIONS);
    vert_sizer->AddSpacer(SETUP_TAB_SIZE);
    
    AddIconText(this, vert_sizer, 
		wxGetBitmapFromMemory(setup_connect_24), new Text(this, _(TEXT_SETUP_WELCOME_STEP1)));
    AddIconText(this, vert_sizer, 
		wxGetBitmapFromMemory(setup_unlock_24), new Text(this, _(TEXT_SETUP_WELCOME_STEP2)));
    AddIconText(this, vert_sizer, 
		wxGetBitmapFromMemory(setup_applications_24), new Text(this, _(TEXT_SETUP_WELCOME_STEP3)));
    AddIconText(this, vert_sizer, 
		wxGetBitmapFromMemory(setup_settings_24), new Text(this, _(TEXT_SETUP_WELCOME_STEP4)));
    
    vert_sizer->AddSpacer(SETUP_TAB_SIZE);
	Text *postamble_text = new Text(this, _(TEXT_SETUP_WELCOME_POSTAMBLE), false);
	postamble_text->Wrap(SETUP_TEXT_WRAP);
	vert_sizer->Add(postamble_text, DEFAULT_BOXSIZER_OPTIONS);

    this->SetSizer(vert_sizer);
}

void InitialPage::DoFocus()
{
    /* Replace the setup bitmap. */
    wizard->SetBitmap(wxGetBitmapFromMemory(setup_full_step_0_flat));
}

//SeruroSetup::SeruroSetup(wxFrame *parent, bool add_server, bool add_address) : 
SeruroSetup::SeruroSetup(wxFrame *parent, setup_type_t type,
    wxString server_uuid, wxString account)
  : setup_type(type), server_uuid(server_uuid), account(account)
	//server_setup(type), address_setup(type)
{
	//wxGetApp().SetSetup(this);

	/* Pre-check for sanity. */
	if (type == SERURO_SETUP_ACCOUNT || type == SERURO_SETUP_IDENTITY) {
		if (theSeruroConfig::Get().GetServerList().size() == 0) {
			this->server_uuid = wxEmptyString;
			this->account = wxEmptyString;
			this->setup_type = SERURO_SETUP_SERVER;
		}
	}

	/* Set title based on type of setup. */
	wxString setup_title = wxString(_(SERURO_APP_NAME)) + _(" Setup");
	if (setup_type == SERURO_SETUP_SERVER) setup_title = _("Add Server Setup");
	if (setup_type == SERURO_SETUP_ACCOUNT) setup_title = _("Add Account Setup");
	if (setup_type == SERURO_SETUP_IDENTITY) setup_title = _("Install Identity");

	/* Create ICON. */
    //wxIcon setup_icon;
    //setup_icon.CopyFromBitmap(wxGetBitmapFromMemory(setup_full_step_0));

    this->Create(parent, SERURO_SETUP_ID, setup_title,
        wxGetBitmapFromMemory(setup_full_step_0_flat), wxDefaultPosition,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	//this->SetIcon(setup_icon);
    
    /* OSX uses a larger font, the identity page warning text may fall off. */
#if defined(__WXOSX__) || defined(__WXMAC__)
    this->SetPageSize(wxSize(SERURO_APP_DEFAULT_WIDTH - 150, -1));
#endif

	/* Set the default values of the navigation buttons. */
	this->next_button_orig = this->m_btnNext->GetLabelText();
	this->prev_button_orig = this->m_btnPrev->GetLabelText();

    /* Page creation, a welcome page for the initial setup. */
	if (setup_type == SERURO_SETUP_INITIAL) {
		this->initial_page  = new InitialPage(this);
        this->account_page  = new AccountPage(this);

		initial_page->SetNext(account_page);
		account_page->SetPrev(initial_page);
        account_page->enable_prev = true;
	}

	/* Only show if in the initial setup or a server/account setup. */
	if (setup_type == SERURO_SETUP_SERVER || setup_type == SERURO_SETUP_ACCOUNT) {
		this->account_page  = new AccountPage(this);
		this->initial_page  = this->account_page;
	}

	/* Downloading an identity is automatic if this is the initial setup. */
	bool force_identity_download = (setup_type == SERURO_SETUP_INITIAL);
	this->identity_page = new IdentityPage(this, force_identity_download);

	if (setup_type != SERURO_SETUP_IDENTITY) {
		account_page->SetNext(identity_page);
		identity_page->SetPrev(account_page);
	} else {
        wxLogMessage(_("SeruroSetup> Created with server UUID (%s)."), this->server_uuid);
		initial_page = identity_page;
	}

	/* Applications page will always exist. */
	this->applications_page = new ApplicationsPage(this);
	identity_page->SetNext(applications_page);
	applications_page->SetPrev(identity_page);

	/* Settings page will always exist. */
	this->settings_page = new SettingsPage(this);
	applications_page->SetNext(settings_page);
	settings_page->SetPrev(applications_page);

    this->GetPageAreaSizer()->Add(this->initial_page);
}

void SeruroSetup::OnFinished(wxWizardEvent &event)
{
	//wxGetApp().RemoveSetup();
}

void SeruroSetup::OnCanceled(wxWizardEvent &event)
{
	//wxGetApp().RemoveSetup();
}

bool SeruroSetup::IsNewServer()
{
	/* Is there a server_uuid provided? */
	return (this->setup_type == SERURO_SETUP_INITIAL || this->setup_type == SERURO_SETUP_SERVER);
}

wxJSONValue SeruroSetup::GetServerInfo()
{
    wxString current_server_uuid;
    
    if (setup_type == SERURO_SETUP_SERVER || setup_type == SERURO_SETUP_INITIAL ||
        setup_type == SERURO_SETUP_ACCOUNT) {
        /* Check for "SERURO_SETUP_ACCOUNT"'s selected server name/uuid response. */
        current_server_uuid = ((AccountPage*) this->GetAccountPage())->GetServerUUID();
    } else {
        current_server_uuid = this->server_uuid;
    }
    
    return theSeruroConfig::Get().GetServer(current_server_uuid);
}

wxString SeruroSetup::GetAccount()
{
    if (setup_type != SERURO_SETUP_IDENTITY) {
        wxJSONValue account_values;
        return ((AccountPage*) this->GetAccountPage())->GetAccount();
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
    wxLogMessage(_("SeruroSetup> (OnChanging) the page is trying to move."));
    
	/* Don't worry if the page is going backward (for now). */
	if (! event.GetDirection() ) {
        event.Skip();
        return;
    }
    
	//wxLogMessage(_("SeruroSetup> (OnChanging) the page is trying to move forward."));

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
