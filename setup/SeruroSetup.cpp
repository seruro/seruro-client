
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

SeruroSetup::SeruroSetup(wxFrame *parent)
{
    this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);
    this->Create(parent, wxID_ANY, wxString(wxT(SERURO_APP_NAME)) + wxT(" Setup"),
        /* Todo: replace icon */
        wxIcon(icon_good), wxDefaultPosition,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    
    /* Page creation. */
    this->initial_page  = new InitialPage(this);
    this->server_page   = new ServerPage(this);
	this->account_page  = new AccountPage(this);
    
    //initial_page->Chain(server_page);
    initial_page->SetNext(server_page);
    server_page->SetPrev(initial_page);

	server_page->SetNext(account_page);
	account_page->SetPrev(server_page);
    
    this->GetPageAreaSizer()->Add(this->initial_page);
}

/* Implemented in wxWizard at: wxwidgets\src\generic */
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

