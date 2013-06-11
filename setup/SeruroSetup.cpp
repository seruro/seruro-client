
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

BEGIN_EVENT_TABLE(SeruroSetup, wxWizard)
    EVT_BUTTON(wxID_BACKWARD, SeruroSetup::GoBack)
END_EVENT_TABLE()

InitialPage::InitialPage(SeruroSetup *parent) : SetupPage(parent)
{
    /* Show welcome message, and overview of the workflow to follow. */
    wxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
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

SeruroSetup::SeruroSetup(wxFrame *parent, bool add_server, bool add_address) : 
  server_setup(add_server), address_setup(add_address),
  /* Set the default values of the navigation buttons */
  next_button_orig(m_btnNext), prev_button_orig(m_btnPrev)
{
	wxString setup_title = wxString(_(SERURO_APP_NAME)) + _(" Setup");
	if (server_setup && ! address_setup) setup_title = _("Add Server Setup"); 
	if (! server_setup && address_setup) setup_title = _("Add Address Setup");

    this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);
    this->Create(parent, wxID_ANY, setup_title,
        /* Todo: replace icon */
        wxIcon(icon_good), wxDefaultPosition,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    
    /* Page creation, a welcome page for the initial setup. */
	if (! server_setup && ! address_setup) {
		this->initial_page  = new InitialPage(this);
	}

	/* Only show if in the initial setup or a server setup. */
	if (server_setup && ! address_setup) {
		this->server_page   = new ServerPage(this);
		/* Set the initial page for dumb member functions. */
		this->initial_page  = this->server_page;
	}

	this->account_page  = new AccountPage(this);
	/* Set the initial page for dumb member functions. */
	if (address_setup) this->initial_page = this->account_page;
    
    if (! server_setup && ! address_setup) {
		initial_page->SetNext(server_page);
		server_page->SetPrev(initial_page);
	}

	if (server_setup && ! address_setup) {
		server_page->SetNext(account_page);
		account_page->SetPrev(server_page);
	}

    this->GetPageAreaSizer()->Add(this->initial_page);
}

/* Implemented in wxWizard at: wxwidgets\src\generic. 
 * This override for a button click allows the page to go backward even if there is
 * a form with invalid input (such as no input). */
void SeruroSetup::GoBack(wxCommandEvent &event)
{
    wxWizardEvent eventPreChanged(wxEVT_WIZARD_BEFORE_PAGE_CHANGED, GetId(), false, m_page);
    (void)m_page->GetEventHandler()->ProcessEvent(eventPreChanged);
    
    if (!eventPreChanged.IsAllowed())
        return;
    
    wxWizardPage *page;
    page = m_page->GetPrev();

    (void)ShowPage(page, false);
}

