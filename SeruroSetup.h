
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include "wx/wizard.h"

#include "SeruroClient.h"
#include "frames/SeruroFrameMain.h"

enum
{
	seruroID_SETUP_ALERT = wxID_HIGHEST + 20,
};

class SeruroSetup : public wxWizard
{
public:
	/* Todo: disable "Exit" while wizard is running. */
	SeruroSetup(SeruroFrameMain *parent);

	wxWizardPage *GetFirstPage() const { return page1; }

private:
	wxWizardPageSimple *page1;
};

class SeruroSetupAlert : public wxWizardPageSimple
{
public:
	SeruroSetupAlert(wxWizard *parent);

	virtual bool TransferDataFromWindow();

private:
	wxCheckBox *checkBox;
};

#endif
