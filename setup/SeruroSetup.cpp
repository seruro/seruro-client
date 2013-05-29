
#include "SeruroSetup.h"
#include "../frames/UIDefs.h"

BEGIN_EVENT_TABLE(SeruroSetup, wxWizard)
    EVT_BUTTON(wxID_BACKWARD, SeruroSetup::GoBack)
END_EVENT_TABLE()

InitialPage::InitialPage(SeruroSetup *parent) : SetupPage(parent)
{
    /* Show welcome message, and overview of the workflow to follow. */
    wxSizer *vert_sizer = new wxBoxSizer(wxVERTICAL);
    
    Text *msg = new Text(this, wxT("Welcome to Seruro! Let's take a moment and configure your client.\n"
        "\n"
        "If this is your first time installing the Seruro Client, please pay attention as some settings may"
        "affect your privacy settings. This initial setup wizard will guide you through:\n"
        "\n"
            "\t 1. Connecting to your Seruro Server.\n"
            "\t 2. Configuring your account and downloading your identity.\n"
            "\t 3. Automatic setup of your email applications.\n"
            "\t 4. Retreival and installation of contact identities.\n"
        "\n"
        "This setup may be canceled and restarted at a later time. After completing the setup you may"
        "add additional servers and accounts as well as change any setting options."));
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
    
    //initial_page->Chain(server_page);
    initial_page->SetNext(server_page);
    server_page->SetPrev(initial_page);
    
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

#if 0
SeruroSetup::SeruroSetup(SeruroFrameMain *parent)
{
	this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);

	this->Create(parent, wxID_ANY, wxT("Seruro Client Setup"),
		/* Todo: replace icon */
		wxIcon(icon_good), wxDefaultPosition,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	this->manualConfigPage = new SeruroSetupAlert(this);
    
	downloadP12Page = new SeruroSetupDownload(this);
	/* Todo: does going backward make sense? */
	manualConfigPage->SetNext(downloadP12Page);
	downloadP12Page->SetPrev(manualConfigPage);
	decryptP12Page = new SeruroSetupDecrypt(this);
	downloadP12Page->SetNext(decryptP12Page);
	decryptP12Page->SetPrev(downloadP12Page);
    /* This will auto size the window vertically. */
    this->GetPageAreaSizer()->Add(this->manualConfigPage);
}

SeruroSetupAlert::SeruroSetupAlert(wxWizard *parent) : wxWizardPageSimple(parent)
{
	this->checkBox = new wxCheckBox(this, wxID_ANY, wxT("I would like to configure the Seruro Client manually?"));
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

	/* Todo: the entire text block is not displaying. */
	mainSizer->Add(new wxStaticText(this, wxID_ANY,
		wxT("Warning: Seruro Client could not find valid configuration settings.\n\n")
        wxT("It does not know how to connect to a Seruro Server. Perhaps you installed\n")
        wxT("the client manually, and did not download a pre-configured client from the server.\n")
        wxT("It is recommended to download the client from your server web interface.\n")
		wxT("Would you like to continue and manually configure the client?")),
		0, wxALL, 5);
	mainSizer->Add(this->checkBox, 0, wxALL, 5); /* 0 = no stretching, 5 = border */
	SetSizer(mainSizer); /* builtin? */
    mainSizer->Fit(this);
}

bool SeruroSetupAlert::TransferDataFromWindow()
{
	if (! this->checkBox->GetValue()) {
		wxMessageBox(wxT("Please verify that you would like to configure the Seruro Client manually."),
			wxT("No Way"), wxICON_WARNING | wxOK, this);
		return false;
	}
	return true;
}

SeruroSetupDownload::SeruroSetupDownload(wxWizard *parent) : wxWizardPageSimple(parent)
{
}

bool SeruroSetupDownload::TransferDataFromWindow()
{
	return true;
}

SeruroSetupDecrypt::SeruroSetupDecrypt(wxWizard *parent) : wxWizardPageSimple(parent)
{
}

bool SeruroSetupDecrypt::TransferDataFromWindow()
{
	return true;
}
#endif

