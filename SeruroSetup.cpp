
#include "SeruroSetup.h"

SeruroSetup::SeruroSetup(SeruroFrameMain *parent)
{
	this->SetExtraStyle(wxWIZARD_EX_HELPBUTTON);

	this->Create(parent, wxID_ANY, wxT("Seruro Client Setup"),
		/* Todo: replace icon */
		wxIcon(icon_good), wxDefaultPosition,
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

	this->page1 = new SeruroSetupAlert(this);

}

SeruroSetupAlert::SeruroSetupAlert(wxWizard *parent) : wxWizardPageSimple(parent)
{
	checkBox = new wxCheckBox(this, wxID_ANY, wxT("I would like to configure the Seruro Client manually?"));
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	
	/* Todo: the entire text block is not displaying. */
	mainSizer->Add(new wxStaticText(this, wxID_ANY,
		wxT("Warning: Seruro Client could not find valid configuration settings. It does not know how to connect to a Seruro Server. Perhaps you installed the client manually, and did not download a pre-configured client from the server. It is recommended to download the client from your server web interface.")
		wxT("Would you like to continue and manually configure the client?")),
		0, wxALL, 5);
	mainSizer->Add(checkBox, 0, wxALL, 5); /* 0 = no stretching, 5 = border */
	SetSizerAndFit(mainSizer); /* builtin? */
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
