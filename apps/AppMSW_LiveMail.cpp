
#if defined(__WXMSW__)

#include "AppMSW_LiveMail.h"
#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../crypto/SeruroCrypto.h"
#include "../api/Utils.h"

#include "ProcessManager.h"

#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/base64.h>
#include <wx/mstream.h>
#include <wx/stream.h>

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

#define HKLM_INSTALL_KEY "\\UserData\\S-1-5-18\\Products\\D0E0789820698F14E938416F4839644A"


#define XML_FIELD_NAME "Account_Name" /* type="SZ" */
#define XML_FIELD_ADDRESS "SMTP_Email_Address" /* type="SZ" */
#define XML_FIELD_AUTHENTICATION "SMTP_Certificate"
#define XML_FIELD_ENCIPHERMENT "SMTP_Encryption_Certificate"
// #define XML_FIELD_DISPLAY_NAME "SMTP_DisplayName"

#define LIVEMAIL_NAME "wlmail.exe"
#define LIVEMAIL_DESCRIPTION "Windows Live Mail"
#define LIVEMAIL_PATH ""

wxDECLARE_APP(SeruroClient);

bool AppMSW_LiveMail::IsRunning()
{
	return ProcessManager::IsProcessRunning(LIVEMAIL_NAME);
}

bool AppMSW_LiveMail::StopApp()
{
	return ProcessManager::StopProcess(LIVEMAIL_NAME);
}

bool AppMSW_LiveMail::StartApp()
{
	return ProcessManager::StartProcessByPath(LIVEMAIL_PATH);
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

	/* Reset previous account info. */
	this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);

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

bool LoadAccountFile(wxString filename, wxXmlDocument &document)
{
	if (! document.Load(filename, "UTF-16")) { 
		/* "UTF-16", wxXMLDOC_KEEP_WHITESPACE_NODES */
		DEBUG_LOG(_("AppMSW_LiveMail> (LoadAccountFile) Cannot read XML (%s)."), filename);
		return false;
	}

	if (document.GetRoot()->GetName() != "MessageAccount") {
		DEBUG_LOG(_("AppMSW_LiveMail> (LoadAccountFile) XML does not contain the correct root."));
		return false;
	}

	return true;
}

bool SaveAccountFile(wxString filename, wxXmlDocument &document)
{
	wxMemoryOutputStream memory_stream;
	size_t xml_size;

	wxMemoryBuffer memory_buffer, output_buffer;
	wxFile document_file;
	
	/* Write the XML document into a memory stream. */
	if (! document.Save(memory_stream, 4)) {
		/* Could not write to memory. */
		return false;
	}

	unsigned char byte = 0;
	void *internal_buffer = 0;

	/* Header has a \xff \xfe ? */
	output_buffer.AppendByte((unsigned char) 255);
	output_buffer.AppendByte((unsigned char) 254);

	/* Convert the stream into a buffer on which we can operate on. */
	xml_size = memory_stream.GetLength();
	memory_buffer.AppendData(memory_stream.GetOutputStreamBuffer()->GetBufferStart(), xml_size);
	internal_buffer = memory_buffer.GetData();

	/* Loop through replaceing 'LF' with 'CRLF'. */
	for (size_t i = 0; i < xml_size; i++) {
		byte = ((unsigned char *) internal_buffer)[i];
		if (byte == '\n') {
			output_buffer.AppendByte('\r');
			output_buffer.AppendByte((unsigned char) 0);
		}
		output_buffer.AppendByte(byte);
	}

	document_file.Open(filename, wxFile::write);
	if (! document_file.IsOpened()) {
		/* Cannot open the file for writing. */
		return false;
	}

	/* The length has changed with the additions included. */
	xml_size = output_buffer.GetDataLen();
	if (document_file.Write(output_buffer.GetData(), xml_size) != xml_size) {
		/* Cannot write to the file. */
		return false;
	}

	document_file.Close();

	return true;
	/* , wxXML_NO_INDENTATION */
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
	if (! LoadAccountFile(account_info["filename"].AsString(), xml_doc)) {
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
	this->info["accounts"][account_filename]["account_name"] = account_filename;

	return account_info["address"].AsString();
}

void SetNodeValue(wxXmlDocument &document, wxString key, wxString value)
{
	wxXmlNode *child;
	wxXmlNode *prev_child;
	wxXmlNode *inner_text;
	
	child = document.GetRoot()->GetChildren();
	while (child) {
		/* Search for an existing element name matching key. */
		if (child->GetName() == key) {
			//child->SetContent(value);
			//return;
			document.GetRoot()->RemoveChild(child);
		} else {
			prev_child = child;
		}
		child = child->GetNext();
	}

	/* Create the element and the attribute, binary. */
	wxXmlAttribute *type_attribute = new wxXmlAttribute(_("type"), _("BINARY"));

	child      = new wxXmlNode(wxXML_ELEMENT_NODE, key);
	inner_text = new wxXmlNode(child, wxXML_TEXT_NODE, wxEmptyString, value);
	child->SetAttributes(type_attribute);
	document.GetRoot()->InsertChildAfter(child, prev_child);

	/* The values we're assigned to the xmlNode structure? */
	/* Do NOT delete! */
}

bool AppMSW_LiveMail::AssignIdentity(wxString server_uuid, wxString address)
{
	wxString auth_skid, enc_skid;
	wxString auth_hash, enc_hash;
	wxString account_file;
	SeruroCrypto crypto;

	account_file = this->GetAccountFile(address);
	if (account_file.compare(wxEmptyString) == 0) {
		DEBUG_LOG(_("AppMSW_LiveMail> (InstallIdentity) Cannot find (%s) within account info."), address);
		return false;
	}

	/* Get both certificate SKIDs from config. */
	auth_skid = wxGetApp().config->GetIdentity(server_uuid, address, ID_AUTHENTICATION);
	enc_skid  = wxGetApp().config->GetIdentity(server_uuid, address, ID_ENCIPHERMENT);

	/* For each, GetIdentityHashBySKID */
	auth_hash = crypto.GetIdentityHashBySKID(auth_skid);
	enc_hash  = crypto.GetIdentityHashBySKID(enc_skid);

	if (auth_hash == wxEmptyString || enc_hash == wxEmptyString) {
		return false;
	}

	/* Open XML and save XML_FIELD_AUTHENTICATION, XML_FIELD_ENCIPHERMENT */
	wxXmlDocument xml_doc;
	if (! LoadAccountFile(info["accounts"][account_file]["filename"].AsString(), xml_doc)) {
		return false;
	}

	SetNodeValue(xml_doc, XML_FIELD_ENCIPHERMENT, AsHex(wxBase64Decode(enc_hash)));
	SetNodeValue(xml_doc, XML_FIELD_AUTHENTICATION, AsHex(wxBase64Decode(auth_hash)));

	/* This step is not working. */
	if (! SaveAccountFile(info["accounts"][account_file]["filename"].AsString(), xml_doc)) {
		return false;
	}

	return true;
}

wxString AppMSW_LiveMail::GetAccountFile(wxString address)
{
	/* Get the address (account) file for a given address (there may be duplicates, find the first. */
	/* Todo: alert user if there are duplicates. */
	wxArrayString account_names;
	
	/* Check each info["accounts"][i][address] for authentication and encipherment. */
	account_names = this->info["accounts"].GetMemberNames();
	for (size_t i = 0; i < account_names.size(); i++) {
		if (this->info["accounts"][account_names[i]]["address"].AsString() == address) {
			//account_name = account_names[i];
			//break;
			return this->info["accounts"][account_names[i]]["account_name"].AsString();
		}
	}

	return wxEmptyString;
}


account_status_t AppMSW_LiveMail::IdentityStatus(wxString address, wxString &server_uuid)
{
	wxString account_name;
	wxJSONValue profile;

    server_uuid.Clear();
	account_name = this->GetAccountFile(address);
	if (account_name.compare(wxEmptyString) == 0) {
		DEBUG_LOG(_("AppMSW_LiveMail> (IsIdentityInstalled) Cannot find (%s) within account info."), address);
		return APP_UNASSIGNED;
	}

	profile =  this->info["accounts"][account_name];

	/* Test if certificate values exist. */
	if (! profile.HasMember("encipherment") || ! profile.HasMember("authentication")) {
			/* STATE: missing certificates. */
			return APP_UNASSIGNED;
	}

	/* If those values are found, search the cert store, then turn them into their binary representations. */
	wxMemoryBuffer search_hash;
	
	search_hash = AsBinary(profile["authentication"].AsString());
	if (! IsHashInstalledAndSet(address, search_hash)) {
		DEBUG_LOG(_("AppMSW_LiveMail> (IsIdentityInstalled) Authentication hash does not exists for (%s)."), address);
		/* STATE: no seruro configured certificates. */
        server_uuid.Append(_("-1"));
		return APP_ALTERNATE_ASSIGNED;
	}

	search_hash = AsBinary(profile["encipherment"].AsString());
	if (! IsHashInstalledAndSet(address, search_hash)) {
		DEBUG_LOG(_("AppMSW_LiveMail> (IsIdentityInstalled) Encipherment hash does not exist for (%s)."), address);
		/* STATE: no seruro configured certificates. */
        server_uuid.Append(_("-1"));
		return APP_ALTERNATE_ASSIGNED;
	}

	SeruroCrypto crypto;
	wxString fingerprint;

	fingerprint = crypto.GetIdentitySKIDByHash(
		wxBase64Encode(AsBinary(profile["authentication"].AsString()))
	);

	/* STATE: seruro configured certificate installed. */
	server_uuid.Append(UUIDFromFingerprint(fingerprint));
	return APP_ASSIGNED;
}

bool AppMSW_LiveMail::GetInfo()
{
	if (is_installed || is_detected) {
		return true;
	}

	wxRegKey *install_key = GetInstallKey(HKLM_INSTALL_KEY);

    if (! install_key->Exists() || ! install_key->HasSubKey(_(HKLM_KEY_PROPERTIES))) {
		delete install_key;
        is_detected = false;
        is_installed = false;
        return false;
    }

    wxString version;
	wxRegKey properties_key(*install_key, HKLM_KEY_PROPERTIES);
	delete install_key;

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