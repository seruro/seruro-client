
#include "SeruroSetup.h"

SeruroSetup::SeruroSetup(SeruroFrameMain *parent)
{
	this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);

	this->Create(parent, wxID_ANY, wxT("Seruro Client Setup"),
		/* Todo: replace icon */
		wxIcon(icon_good), wxDefaultPosition,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	this->page1 = new SeruroSetupAlert(this);
    /* This will auto size the window vertically. */
    this->GetPageAreaSizer()->Add(this->page1);
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
