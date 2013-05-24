
#ifndef H_SeruroSetup
#define H_SeruroSetup

#include "wx/wizard.h"

#include "SeruroClient.h"
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

private:
	//wxWizardPageSimple *manualConfigPage;
	//wxWizardPageSimple *downloadP12Page;
	//wxWizardPageSimple *decryptP12Page;
    SetupPage *initial_page;
    
    SetupPage *server_page;
    SetupPage *account_page;
    SetupPage *applications_page;
    SetupPage *settings_page;
};

class InitialPage : public SetupPage
{
public:
    InitialPage (SeruroSetup *parent);
};

class ServerPage : public SetupPage
{
public:
    ServerPage(SeruroSetup *parent);
};

/**** DEPRECATED ******/

/* The following classes/methods definitions use a model expecting the server to generate
 * a client with a valid config. This is not possible if the setup is expected to include
 * a signature (aka, this binary cannot be generated on the fly.)
 */

#if 0
/* Manual configuration alert */
class SeruroSetupAlert : public wxWizardPageSimple
{
public:
	SeruroSetupAlert(wxWizard *parent);
	virtual bool TransferDataFromWindow();
private:
	wxCheckBox *checkBox;
};

/* Download P12 (server, username, password) */
class SeruroSetupDownload : public wxWizardPageSimple
{
/* General note: this view takes two main forms, depending on whether the 
 * client was 'bundled' with the SeruroServer address and certificate.
 * (Manual) means the Client was installed without this bundle and is 
 * determined based on the existance of a "SeruroConfig.config" file.
 * (Auto) means the Client was bundled with a config file listing the 
 * address of the SeruroServer, and *should* have the server's TLS
 * certificate installed. 
 *
 * Problem(1): the manual install may try to connect to a server that uses
 * a CA-signed TLS cert, so the user does NOT have to provide the cert.
 * Problem(2): we bypass the 'manual' check if a config exists, it is 
 * very possible and likely that a user will write their own, but this
 * does not mean a TLS cert is provied (resulting in problem 1). However
 * the 'auto' install may fail, or the TLS cert may be removed by some
 * other means, turning an 'auto' install into something terrible for the
 * unknowning-user. A config without TLS cert error should be handled
 * gently to say the least.
 *
 * Handling: Knowning the odd-ities of having a TLS cert, this view
 * will initially NOT ask for a TLS cert for the manual case. If an
 * SSL/TLS handshake fails due to an unknown-server-cert, then the view
 * will change and ask the user for the cert. For the non-manual case
 * the view will display an ERROR, but allow the user to continue.
 */
public:
	SeruroSetupDownload(wxWizard *parent);
	virtual bool TransferDataFromWindow();
private:
	wxTextCtrl *usernameText;
	wxTextCtrl *passwordText;

	/* They choose a server to authenticate */
	/* The sync cert is 'grey-ed' out initially for a manual setup.
	 * this becomes interactive if the user chooses to enter or a 
	 * initial-try of a SSL/TLS handshake fails.
	 */
	wxFileDialog *syncCertFile;
	wxCheckBox *useCert;
	wxTextCtrl *serverName;

	/* ComboBox if servers already configured. */
	wxComboBox *server;
};

/* Decrypt P12 */
class SeruroSetupDecrypt : public wxWizardPageSimple
{
public:
	SeruroSetupDecrypt(wxWizard *parent);
	virtual bool TransferDataFromWindow();
private:
	wxTextCtrl *keyText;
	wxComboBox *p12;
};
#endif


#endif
