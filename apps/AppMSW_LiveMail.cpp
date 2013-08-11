
#if defined(__WXMSW__)

#include "AppMSW_LiveMail.h"
#include "../SeruroClient.h"
#include "../crypto/SeruroCrypto.h"

#include <wx/msw/registry.h>

#include <wx/stdpaths.h>

#define MAILDATA_PATH "/AppData/Local/Microsoft/Windows Live Mail/"

/* The file will exist if the folder corresponds to an account, root = MessageAccount. */
#define MAILDATA_SUBACCOUNTS "*/account*.oeaccount"

/* Both fields have a "type" attribute as "BINARY" */
/* Both fields may not exist, and should be the "thumbprint" or hash of the certificate. */
#define XML_FIELD_AUTH "SMTP_Certificate" 
#define XML_FIELD_ENC  "SMTP_Encryption_Certificate"

/* Check if Windows Live Mail is installed. */
/*
 * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Installer\
 * UserData\S-1-5-18\Products\D0E0789820698F14E938416F4839644A
 *
 * HKEY_CLASSES_ROOT\Installer\Products\D0E0789820698F14E938416F4839644A
 *
 * HKEY_LOCAL_MACHINE\SOFTWARE\Classes\Installer\Products\D0E0789820698F14E938416F4839644A
 */

#define HKLM_KEY "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer\\\
UserData\\S-1-5-18\\Products\D0E0789820698F14E938416F4839644A"
#define HKLM_KEY_PROPERTIES "InstallProperties"

#define XML_FIELD_NAME "Account_Name" /* type="SZ" */
#define XML_FIELD_ADDRESS "SMTP_Email_Address" /* type="SZ" */
// #define XML_FIELD_DISPLAY_NAME "SMTP_DisplayName"

DECLARE_APP(SeruroClient);

bool AppMSW_LiveMail::IsInstalled()
{
	if (! GetInfo()) return false;

	return this->is_installed;
}

wxString AppMSW_LiveMail::GetVersion()
{
	if (! GetInfo()) return _("N/A");

	if (this->info.HasMember("version")) {
		return info["version"].AsString();
	}

	return _("0");
}

wxArrayString AppMSW_LiveMail::GetAccountList()
{
	wxArrayString accounts;

	wxStandardPaths paths = wxStandardPaths::Get();
	DEBUG_LOG(_("local app dir: %s"), paths.GetLocalDataDir());

	return accounts;
}

bool AppMSW_LiveMail::IsIdentityInstalled(wxString account_name)
{
	return false;
}

bool AppMSW_LiveMail::GetInfo()
{
    wxRegKey check_key(wxRegKey::HKLM, _(HKLM_KEY));
    
    if (! check_key.Exists() || ! check_key.HasSubKey(_(HKLM_KEY_PROPERTIES))) {
        is_detected = false;
        is_installed = false;
        return false;
    }
    
    wxString version;
    wxRegKey properties_key(wxRegKey::HKLM, _(HKLM_KEY) + _(HKLM_KEY_PROPERTIES));
    if (! properties_key.QueryValue(_("DisplayVersion"), version, false)) {
        is_detected = false;
        return false;
    }
    
    this->info["version"] = version;
    
    
	return false;
}

#endif