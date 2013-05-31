
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include "wx/wizard.h"
#include "wx/textctrl.h"

#include "../wxJSON/wx/jsonval.h"

#include "SeruroClient.h"

/* The dialogs include the form mixin classes. */
#include "../frames/dialogs/AddServerDialog.h"
//#include "frames/SeruroFrameMain.h"

enum seruro_setup_ids
{
	//seruroID_SETUP_ALERT,
	//seruroID_SETUP_DOWNLOAD,
	//seruroID_SETUP_DECRYPT
    SETUP_SERVER_ID,
    SETUP_ACCOUNT_ID,
    SETUP_APPLICATIONS_ID,
    SETUP_SETTINGS_ID
};

class SetupPage : public wxWizardPageSimple
{
public:
    SetupPage (wxWizard *parent) : wxWizardPageSimple(parent) {}
    //virtual wxJSONValue GetValues();
};

class SeruroSetup : public wxWizard
{
public:
	/* Todo: disable "Exit" while wizard is running. */
	SeruroSetup(wxFrame *parent);

	//wxWizardPage *GetManualConfig() const { return manualConfigPage; }
	//wxWizardPage *GetDownload() const { return downloadP12Page; }
	//wxWizardPage *GetDecrypt() const { return decryptP12Page; }
    wxWizardPage *GetInitialPage() const { return initial_page; }
    
    /* Over write without validation for backbutton */
    void GoBack(wxCommandEvent& event);

private:
	//wxWizardPageSimple *manualConfigPage;
	//wxWizardPageSimple *downloadP12Page;
	//wxWizardPageSimple *decryptP12Page;
    SetupPage *initial_page;
    
    SetupPage *server_page;
    SetupPage *account_page;
    SetupPage *applications_page;
    SetupPage *settings_page;
    
    DECLARE_EVENT_TABLE()
};

class InitialPage : public SetupPage
{
public:
    InitialPage (SeruroSetup *parent);
};

/* Should be very similar to /frames/dialogs/AddServerDialog */
class ServerPage : public SetupPage, public AddServerForm
{
public:
    ServerPage(SeruroSetup *parent);
};

/* Should be very similar to /frames/dialogs/AddAccountDialog */
class AccountPage : public SetupPage
{
public:
    AccountPage (SeruroSetup *parent);
};

#endif
