
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include "wx/wizard.h"
#include "wx/textctrl.h"

#include "../wxJSON/wx/jsonval.h"

#include "../SeruroClient.h"
#include "SeruroSetup.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
#include "../frames/dialogs/AddAccountDialog.h"

class AddServerWizard : public wxWizard
{
public:
	AddServerWizard(wxFrame *parent);

	wxWizardPage *GetInitialPage() const { return server_page; }

	void GoBack(wxCommandEvent &event);

private:
	SetupPage *server_page;
	SetupPage *address_page;
	SetupPage *identity_page;

	DECLARE_EVENT_TABLE()
};

#endif