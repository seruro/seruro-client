
#if defined(__WXMSW__)

#include "AppMSW_Outlook.h"
#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../crypto/SeruroCrypto.h"

#include <wx/msw/registry.h>

/* Add all keys which may contain Outlook. */
#define KEY_OFFICE_2010_PLUS L"\\UserData\\S-1-5-18\\Products\\00004109110000000000000000F01FEC"

#define HKCU_SUBSYSTEM_BASE L"Software\\Microsoft\\Windows NT\\CurrentVersion"
#define HKCU_SUBSYSTEM L"Windows Messaging Subsystem\\Profiles\\Outlook"

wxDECLARE_APP(SeruroClient);

bool AppMSW_Outlook::IsInstalled()
{
	if (! GetInfo()) return false;

	//return this->is_installed;
	return false;
}

wxString AppMSW_Outlook::GetVersion()
{
	if (! GetInfo()) return _("N/A");

	if (this->info.HasMember("version")) {
		return info["version"].AsString();
	}

	return _("0");
}

wxArrayString AppMSW_Outlook::GetAccountList()
{
	wxArrayString accounts;

	/* Fail if no application info is detected. */
	if (! this->GetInfo()) {
		return accounts;
	}

	/* Search for accounts within the Messaging Subsystem. */
	//wxRegKey *message_subsystem = new wxRegKey(wxRegKey::HKCU, HKCU_SUBSYSTEM_BASE L"\\" HKCU_SUBSYSTEM);

	/* There are no account profiles? */
	//if (! message_subsystem->Exists() || ! message_subsystem->HasSubkeys()) {
	//	delete message_subsystem;
	//	return accounts;
	//}

	/* Iterate over subkeys. */
	//wxString profile_name, profile_index;
	//wxString account_name, account_address;

	/* Use the MAPI to enumerate profiles, possibly retreiving account name/email from each. */

	return accounts;
}

bool AppMSW_Outlook::AssignIdentity(wxString server_uuid, wxString address)
{
	return false;
}

account_status_t AppMSW_Outlook::IdentityStatus(wxString address, wxString &server_uuid)
{
	return APP_UNASSIGNED;
}

bool AppMSW_Outlook::GetInfo()
{
	if (is_installed || is_detected) {
		return true;
	}

	/* Check each possible key. */
	wxRegKey *install_key;
	wxArrayString versions, products;
	wxString version;

	versions.Add(_(KEY_OFFICE_2010_PLUS));

	for (size_t i = 0; i < versions.size(); i++) {
		install_key = GetInstallKey(versions[i]);
		if (install_key->Exists() && install_key->HasSubKey(_(HKLM_KEY_PROPERTIES))) {
			/* The user may have multiple versions of Office installed. */
			products.Add(versions[i]);
		}
		delete install_key;
	}

	/* Just use the first version (most recent install by year) for now. */
	if (products.size() == 0) {
		is_detected = false;
		is_installed = false;
		return false;
	}

	install_key = GetInstallKey(products[0]);
	wxRegKey properties_key(*install_key, _(HKLM_KEY_PROPERTIES));
	delete install_key;

	if (! properties_key.QueryValue(_("DisplayVersion"), version, false)) {
		is_detected = false;
		return false;
	}

	/* Querying was successful. */
	is_detected = true;
	is_installed = true;

	this->info["version"] = version;

	return true;
}

#endif