
#include "wx/wizard.h"

#include "SeruroClient.h"

enum
{
	seruroID_SETUP_ALERT = wxID_HIGHEST + 20,
};

class SeruroSetup : public wxWizard
{
public:
	SeruroSetup(wxFrame *parent);

	wxWizardPage *GetFirstPage() const { return page1; }

private:
	wxWizardPageSimple *page1;
}

class SeruroSetupAlert : public wxWizardPageSimple
{
public:
	SeruroSetupAlert(wxWizard *parent) : wxWizardPageSimple(parent)
	{
		checkBox = new wxCheckBox(this, wxID_ANY, wxT("I would like to configure the Seruro Client manually?"));
		wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
		
		mainSizer->Add(new wxStaticText(this, wxID_ANY,
			wxT("Warning: Seruro Client could not find valid configuration settings. It does not know how to connect to a Seruro Server. Perhaps you installed the client manually, and did not download a pre-configured client from the server. It is recommended to download the client from your server web interface.")
			wxT("Would you like to continue and manually configure the client?")),
			0, wxALL, 5);
		mainSizer->Add(checkBox, 0, wxALL, 5); /* 0 = no stretching, 5 = border */
		SetSizerAndFit(mainSizer); /* builtin? */
	}

	virtual bool TransferDataFromWindow() 
	{
		if (! this->checkBox->GetValue()) {
			wxMessageBox(wxT("Please verify that you would like to configure the Seruro Client manually.", wxT("No Way"), wxICON_WARNING | wxOK, this);
			return false;
		}
		return true;
	}

private:
	wxCheckBox *checkBox;
}

