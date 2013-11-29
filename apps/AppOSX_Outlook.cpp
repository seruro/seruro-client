
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_Outlook.h"

#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/string.h>
#include <wx/dir.h>

#include "ProcessManager.h"
#include "../logging/SeruroLogger.h"
#include "helpers/AppOSX_Utils.h"

#include "../crypto/SeruroCrypto.h"
#include "../SeruroConfig.h"

/* For application location/info. */
#include <ApplicationServices/ApplicationServices.h>

#define BUNDLE_ID "com.microsoft.Outlook"
#define BUNDLE_ID_DAEMON "com.microsoft.outlook.database_daemon"

/* Location relative to user's home directory for outlook data. */
#define MAILDATA_RECORDS    "/Documents/Microsoft User Data/Office 2011 Identities/Main Identity/Data Records"
/* Locations relative to records directory for identities/contacts. */
#define MAILDATA_IDENTITIES "/Mail Accounts/0T/0B/0M/0K"
#define MAILDATA_CONTACTS   "/Contacts/0T/0B/0M/0K"

/* Tries to parse an identity file, should also try to verify using AppleScript. */
void AppOSX_Outlook::ParseIdentity(wxString identity_path)
{
    wxString full_path;
    wxJSONValue account;
    
    full_path = wxFileName::GetHomeDir() + _(MAILDATA_RECORDS) + _(MAILDATA_IDENTITIES);
    full_path += _("/") + identity_path;
    
    AppOSX_OutlookIdentity identity;
    
    identity.SetPath(full_path);
    if (! identity.ReadMarc()) {
        /* Could not open identity file, or identity file is too large. */
        return;
    }
    
    if (! identity.ParseMarc()) {
        /* Nothing left to do. */
        return;
    }
    
    /* Perform the parsed data formatting. */
    account["path"] = full_path;
    account["address"] = identity.GetAddress();
    
    /* Retrieve auth/enc certificate serial/subject from identity, check if serial/subject exists. */
    if (identity.HasAuthCertificate()) {
        account["auth_data"] = identity.GetAuthCertificate();
    }

    if (identity.HasEncCertificate()) {
        account["enc_data"] = identity.GetEncCertificate();
    }
    
    /* Add this account instance to the list of accounts. */
    this->info["accounts"].Append(account);
}

wxArrayString AppOSX_Outlook::GetAccountList()
{
    wxArrayString accounts;

    /* Reset previous account info. */
	this->info["accounts"] = wxJSONValue(wxJSONTYPE_ARRAY);
    
	if (! this->GetInfo()) {
		return accounts;
	}
    
    /* Iterate through the files within MAILDATA_RECORDS + MAILDATA_IDENTITIES, try to open each file and look for magic. */
    wxDir search_directory(wxString(wxFileName::GetHomeDir() + _(MAILDATA_RECORDS) + _(MAILDATA_IDENTITIES)));
    
	wxString sub_filename;
	wxString account_filename;
	bool continue_search;
    
	/* Search for each file within. */
	continue_search = search_directory.GetFirst(&sub_filename, wxEmptyString, wxDIR_FILES);
	while (continue_search) {
		/* Will return true if there was a match, even if it is the last match. */
        ParseIdentity(sub_filename);
		continue_search = search_directory.GetNext(&sub_filename);
	}
    
    /* Iterate the accounts list, create an array of the addresses. */
    for (size_t i = 0; i < info["accounts"].Size(); ++i) {
        if (info["accounts"][i].HasMember("address")) {
            accounts.Add(info["accounts"][i]["address"].AsString());
        }
    }
    
    return accounts;
}

account_status_t AppOSX_Outlook::IdentityStatus(wxString address, wxString &server_uuid)
{
    //wxJSONValue account;
    wxString serial, subject, subject_key;
    int account_index = 0;
    
	if (! this->GetInfo()) {
		return APP_UNASSIGNED;
	}
    
    /* Check that the address exists as a configured account for Outlook. */
    for (size_t i = 0; i < info["accounts"].Size(); ++i) {
        if (info["accounts"][i].HasMember("address") && info["accounts"][i]["address"].AsString() == address) {
            //account = info["accounts"][i];
            account_index = i;
            break;
        }
    }
    
    /* Check that the account has both an auth an enc certificate configured. */
    if (! info["accounts"][account_index].HasMember("auth_data") ||
        ! info["accounts"][account_index].HasMember("enc_data")) {
        return APP_UNASSIGNED;
    }
    
    SeruroCrypto crypto;
    serial = info["accounts"][account_index]["auth_data"]["serial"].AsString();
    subject = info["accounts"][account_index]["auth_data"]["subject"].AsString();
    
    if (serial == wxEmptyString || subject == wxEmptyString) {
        return APP_UNASSIGNED;
    }
    
    /* Verify the certificate exists in the keychain. */
    if (! crypto.HasIdentityIssuer(subject, serial)) {
        return APP_ALTERNATE_ASSIGNED;
    }
    
    serial = info["accounts"][account_index]["enc_data"]["serial"].AsString();
    subject = info["accounts"][account_index]["enc_data"]["subject"].AsString();
    
    if (serial == wxEmptyString || subject == wxEmptyString) {
        return APP_UNASSIGNED;
    }
    
    if (! crypto.HasIdentityIssuer(subject, serial)) {
        return APP_ALTERNATE_ASSIGNED;
    }
    
    subject_key = crypto.GetIdentityIssuerSKID(subject, serial);
    server_uuid.Append(UUIDFromFingerprint(subject_key));
    
    return APP_ASSIGNED;
}

bool AppOSX_Outlook::AssignIdentity(wxString server_uuid, wxString address)
{
    wxString identity_path;
    wxString auth_skid, enc_skid;
    wxString auth_subject, auth_serial, enc_subject, enc_serial;
    int account_index = 0;
    
	if (! this->GetInfo()) {
		return false;
	}
    
    /* Get account/identity from address. */
    for (size_t i = 0; i < info["accounts"].Size(); ++i) {
        if (info["accounts"][i].HasMember("address") && info["accounts"][i]["address"].AsString() == address) {
            identity_path = info["accounts"][i]["path"].AsString();
            account_index = i;
            break;
        }
    }
    
    AppOSX_OutlookIdentity identity;
    
    identity.SetPath(identity_path);
    if (! identity.ReadMarc()) {
        /* Could not open identity file, or identity file is too large. */
        return false;
    }
    
    /* Could not parse identity MaRC file. */
    if (! identity.ParseMarc()) {
        return false;
    }
    
    /* Get subject serial from address/server_uuid. */
    SeruroCrypto crypto;
    
    /* Fillin required certificate information. */
    auth_skid = theSeruroConfig::Get().GetIdentity(server_uuid, address, ID_AUTHENTICATION);
    crypto.GetSKIDIdentityIssuer(auth_skid, auth_subject, auth_serial);
    enc_skid = theSeruroConfig::Get().GetIdentity(server_uuid, address, ID_ENCIPHERMENT);
    crypto.GetSKIDIdentityIssuer(enc_skid, enc_subject, enc_serial);
    
    if (! identity.AssignCerts(auth_subject, auth_serial, enc_subject, enc_serial)) {
        /* Big problem, recover by applying backup. */
        return false;
    }
    
    /* Cache the changes. */
    info["accounts"][account_index]["auth_data"]["serial"] = auth_serial;
    info["accounts"][account_index]["auth_data"]["subject"] = auth_subject;
    info["accounts"][account_index]["enc_data"]["serial"] = enc_serial;
    info["accounts"][account_index]["enc_data"]["subject"] = enc_subject;
    
    return true;
}

bool AppOSX_Outlook::UnassignIdentity(wxString address)
{
    wxString identity_path;
    int account_index = 0;

	if (! this->GetInfo()) {
		return false;
	}
    
    /* Get account/identity from address. */
    for (size_t i = 0; i < info["accounts"].Size(); ++i) {
        if (info["accounts"][i].HasMember("address") && info["accounts"][i]["address"].AsString() == address) {
            identity_path = info["accounts"][i]["path"].AsString();
            account_index = 0;
            break;
        }
    }
    
    AppOSX_OutlookIdentity identity;
    
    identity.SetPath(identity_path);
    if (! identity.ReadMarc()) {
        /* Could not open identity file, or identity file is too large. */
        return false;
    }
    
    /* Could not parse identity MaRC file. */
    if (! identity.ParseMarc()) {
        return false;
    }
    
    if (! identity.ClearCerts()) {
        /* Big problem, recover by applying backup. */
        return false;
    }
    
    /* Cache the changes. */
    info["accounts"][account_index].Remove("auth_data");
    info["accounts"][account_index].Remove("enc_data");
    
    return true;
}

bool AppOSX_Outlook::IsRunning()
{
    return ProcessManager::IsProcessRunning(BUNDLE_ID);
}

bool AppOSX_Outlook::StopApp()
{
    if (! ProcessManager::StopProcess(BUNDLE_ID)) {
        return false;
    }
    
    /* Stopping the database daemon is optional. */
    //ProcessManager::StopProcess(BUNDLE_ID_DAEMON);
    return true;
}

bool AppOSX_Outlook::StartApp()
{
    return ProcessManager::StartProcess(BUNDLE_ID);
}

bool AppOSX_Outlook::IsInstalled()
{
    if (! GetInfo()) return false;
    
    return this->is_installed;
}

wxString AppOSX_Outlook::GetVersion()
{
    if (! GetInfo()) return _("N/A");
    
    if (this->info.HasMember("version")) {
        return info["version"].AsString();
    }
    
    this->info["version"] = AppVersion(this->info["location"].AsString());
    return this->info["version"].AsString();
}

bool AppOSX_Outlook::GetInfo()
{
    /* GetInfo should only occur once. */
    if (this->is_detected) {
        return true;
    }
    
    wxString app_location;
    if (! AppInstalled(_(BUNDLE_ID), app_location)) {
        this->is_installed = false;
        this->is_detected = false;
        return false;
    }
    
    /* Store the location, then find the version from the relative PLIST. */
    this->info["location"] = app_location;
    this->is_detected = true;
    this->is_installed = true;

    return true;
}

#endif /* OS Check */