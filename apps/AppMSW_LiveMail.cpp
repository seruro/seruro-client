
#if defined(__WXMSW__)

#include "AppMSW_LiveMail.h"
#include "../SeruroClient.h"
#include "../crypto/SeruroCrypto.h"

#include <wx/msw/registry.h>

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/xml/xml.h>
#include <wx/base64.h>

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

#define HKLM_BIT_ROOT "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"
#define HKLM_INSTALL_KEY "\\UserData\\S-1-5-18\\Products\\D0E0789820698F14E938416F4839644A"
#define HKLM_KEY_PROPERTIES "InstallProperties"

#define XML_FIELD_NAME "Account_Name" /* type="SZ" */
#define XML_FIELD_ADDRESS "SMTP_Email_Address" /* type="SZ" */
#define XML_FIELD_AUTHENTICATION "SMTP_Certificate"
#define XML_FIELD_ENCIPHERMENT "SMTP_Encryption_Certificate"
// #define XML_FIELD_DISPLAY_NAME "SMTP_DisplayName"

DECLARE_APP(SeruroClient);

wxMemoryBuffer AsBinary(wxString hex_string)
{
	wxMemoryBuffer buffer;
	int byte;
	char hex;

	size_t length = hex_string.Length();
	
	/* Expect the string to be hex byte encoded. */
	for (size_t i = 0; i < length/2; i++) {
		byte = 0;
		hex = hex_string.GetChar(i*2);
		if (hex >= '0' && hex <= '9') { byte = hex-'0'; } 
		else { byte = hex-'a'+10; }
		
		byte *= 16;
		hex = hex_string.GetChar(i*2 + 1);
		if (hex >= '0' && hex <= '9') { byte += hex-'0'; } 
		else { byte += hex-'a'+10; }
		buffer.AppendByte(byte);
	}

	return buffer;
}

wxString AsHex(wxMemoryBuffer binary_string)
{
	wxString hex_encoded;
	int byte = 0;

	void *binary = (void *) binary_string.GetData();
	size_t length = binary_string.GetDataLen();

	for (size_t i = 0; i < length; i++) {
		byte = ((unsigned char *) binary)[i];
		hex_encoded.Append(wxString::Format(_("%x"), byte));
	}

	return hex_encoded;
}

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

	if (! this->GetInfo()) {
		return accounts;
	}

	/* Loop through folders looking for the account file. */
	// isFileReadable, isFileWritable
	wxDir search_directory(this->info["location"].AsString()); /* The Windows Live Mail directory */
	wxString sub_filename;
	wxString account_filename;
	bool continue_search;

	/* Search for each sub directory. */
	continue_search = search_directory.GetFirst(&sub_filename, wxEmptyString, wxDIR_DIRS);
	while (continue_search) {
		/* Will return true if there was a match, even if it is the last match. */
		if (GetAccountFile(sub_filename, account_filename)) {
			/* Otherwise add the account. */
			accounts.Add(this->GetAccountInfo(sub_filename, account_filename));
		}
		continue_search = search_directory.GetNext(&sub_filename);
	}

	return accounts;
}

/* Search mail home and sub_folder for an account file. */
bool AppMSW_LiveMail::GetAccountFile(const wxString &sub_folder, wxString &account_filename)
{
	wxFileName account_folder;
	account_folder.AssignDir(this->info["location"].AsString());
	account_folder.AppendDir(sub_folder);

	/* Create a directory from the two appended paths. */
	wxDir search_directory(account_folder.GetFullPath());
	wxString search_file;
	wxFileName sub_filename;
	bool continue_search;

	/* Clear the pass-by-reference result parameter. */
	account_filename.Clear();
	if (! search_directory.HasFiles()) {
		return false;
	}

	/* Search only for files. */
	continue_search = search_directory.GetFirst(&search_file, wxEmptyString, wxDIR_FILES);
	while (continue_search) {
		/* Check the file extension, it must match OEACCOUNT. */
		sub_filename.Assign(search_file);
		if (sub_filename.IsOk() && sub_filename.GetExt().compare("oeaccount") == 0) {
			account_filename.Append(search_file);
			return true;
		}

		continue_search = search_directory.GetNext(&search_file);
	}

	return false;
}

wxString AppMSW_LiveMail::GetAccountInfo(const wxString &account_folder, const wxString &account_filename)
{
	wxJSONValue account_info;
	wxFileName account_file;

	account_file.AssignDir(this->info["location"].AsString());
	account_file.AppendDir(account_folder);
	account_file.SetFullName(account_filename);

	account_info["filename"] = account_file.GetFullPath();

	wxXmlDocument xml_doc;
	if (! xml_doc.Load(account_file.GetFullPath())) {
		DEBUG_LOG(_("AppMSW_LiveMail> (GetAccountInfo) Cannot read XML (%s)."), account_file.GetFullPath());
		return _("<None>");
	}

	if (xml_doc.GetRoot()->GetName() != "MessageAccount") {
		DEBUG_LOG(_("AppMSW_LiveMail> (GetAccountInfo) XML does not contain the correct root."));
		return _("<None>");
	}

	wxXmlNode *child = xml_doc.GetRoot()->GetChildren();
	while (child) {
		if (child->GetName() == XML_FIELD_NAME) {
			account_info["name"] = child->GetNodeContent();
		} else if (child->GetName() == XML_FIELD_ADDRESS) {
			account_info["address"] = child->GetNodeContent();
		} else if (child->GetName() == XML_FIELD_AUTHENTICATION) {
			account_info["authentication"] = child->GetNodeContent();
		} else if (child->GetName() == XML_FIELD_ENCIPHERMENT) {
			account_info["encipherment"] = child->GetNodeContent();
		}

		child = child->GetNext();
	}

	if (! account_info.HasMember("address")) {
		DEBUG_LOG(_("AppMSW_LiveMail> (GetAccountInfo) XML does not contain a proper address node."));
		return _("<None>");
	}

	/* Set or replace the account info for this folder (account). */
	this->info["accounts"][account_filename] = account_info;

	return account_info["address"].AsString();
}

bool AppMSW_LiveMail::IsIdentityInstalled(wxString address)
{
	wxString account_name;
	wxArrayString account_names;
	
	/* Check each info["accounts"][i][address] for authentication and encipherment. */
	account_names = this->info["accounts"].GetMemberNames();
	for (size_t i = 0; i < account_names.size(); i++) {
		if (this->info["accounts"][account_names[i]]["address"].AsString() == address) {
			account_name = account_names[i];
			break;
		}
	}

	if (account_name.compare(wxEmptyString) == 0) {
		DEBUG_LOG(_("AppMSW_LiveMail> (IsIdentityInstalled) Cannot find (%s) within account info."), address);
		return false;
	}

	/* Test if certificate values exist. */
	if (! this->info["accounts"][account_name].HasMember("encipherment") || 
		! this->info["accounts"][account_name].HasMember("authentication")) {
			return false;
	}

	/* If those values are found then search then turn them into their binary representations. */
	SeruroCrypto crypto_helper;
	wxMemoryBuffer search_hash;
	
	search_hash = AsBinary(this->info["accounts"][account_name]["encipherment"].AsString());
	/* Crypto expects all certificate as base64 encoded buffers. */
	if (! crypto_helper.HaveIdentityByHash(wxBase64Encode(search_hash))) {
		return false;
	}
	search_hash = AsBinary(this->info["accounts"][account_name]["encipherment"].AsString());
	if (! crypto_helper.HaveIdentityByHash(wxBase64Encode(search_hash))) {
		return false;
	}

	/* Search the certificate store for that HASH value, and retreive the SKID, then check config. */
	/* Todo: get SKID and check the application config. */

	return false;
}

bool AppMSW_LiveMail::GetInfo()
{
	if (is_installed || is_detected) {
		return true;
	}

    wxRegKey *check_key = new wxRegKey(wxRegKey::HKLM, HKLM_BIT_ROOT);
    
	/* If this is a 64-bit system then the registry will need to be reopened. */
	wxString installer_sub;
	long key_index = 0;
	if (check_key->Exists() && check_key->GetFirstKey(installer_sub, key_index)) {
		if (installer_sub.compare("ResolveIOD") == 0) {
			delete check_key;
			check_key = new wxRegKey(wxRegKey::HKLM, HKLM_BIT_ROOT, wxRegKey::WOW64ViewMode_64);
		}
	}

	wxRegKey install_key(*check_key, HKLM_INSTALL_KEY);
	delete check_key;

    if (! install_key.Exists() || ! install_key.HasSubKey(_(HKLM_KEY_PROPERTIES))) {
        is_detected = false;
        is_installed = false;
        return false;
    }

    wxString version;
	wxRegKey properties_key(install_key, HKLM_KEY_PROPERTIES);
    if (! properties_key.QueryValue(_("DisplayVersion"), version, false)) {
        is_detected = false;
        return false;
    }
    
	is_detected = true;
	is_installed = true;
    this->info["version"] = version;

	wxFileName mail_location;
	
	/* Get the local data dir by accessing this apps local, and moving up a directory. */
	wxStandardPaths paths = wxStandardPaths::Get();
	mail_location.AssignDir(paths.GetUserLocalDataDir());
	mail_location.RemoveLastDir();

	/* The mail accounts should be in Microsoft/Windows Live Mail. */
	mail_location.AppendDir("Microsoft");
	mail_location.AppendDir("Windows Live Mail");

	if (! mail_location.Exists() || ! mail_location.IsDir()) {
		is_detected = false;
		return false;
	}

	this->info["location"] = mail_location.GetFullPath();
    
	return true;
}

#endif