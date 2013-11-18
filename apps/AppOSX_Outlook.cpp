
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_Outlook.h"

#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/string.h>
#include <wx/dir.h>

#include "ProcessManager.h"
#include "../logging/SeruroLogger.h"
#include "helpers/AppOSX_Utils.h"

/* For application location/info. */
#include <ApplicationServices/ApplicationServices.h>

#define BUNDLE_ID "com.microsoft.Outlook"
#define BUNDLE_ID_DAEMON "com.microsoft.outlook.database_daemon"

/* Location relative to user's home directory for outlook data. */
#define MAILDATA_RECORDS    "/Documents/Microsoft User Data/Office 2011 Identities/Main Identity/Data Records"
/* Locations relative to records directory for identities/contacts. */
#define MAILDATA_IDENTITIES "/Mail Accounts/0T/0B/0M/0K"
#define MAILDATA_CONTACTS   "/Contacts/0T/0B/0M/0K"

#define IDENTITY_MAGIC "MaRC"
#define IDENTITY_MAX_LENGTH 10000

//DECLARE_APP(SeruroClient);

bool AppOSX_Outlook::GetIdentity(wxString full_path, AppOSX_OutlookIdentity &identity)
{
    /* A raw file wrapper */
    wxFFile identity_file;
    
    void *read_buffer;
    size_t read_size;
    
    /* Try to open the file and assure we can read (default open parameter is readonly). */
    if (! identity_file.Open(full_path) || ! identity_file.IsOpened()) {
        DEBUG_LOG(_("AppOSX_Outlook> (ParseIdentity) could not open file (%s)."), full_path);
        return false;
    }
    
    read_buffer = (void*) malloc(5);
    memset(read_buffer, 0, 5);
    
    read_size = identity_file.Read(read_buffer, 4);
    if (read_size != 4 || wxString((char*) read_buffer) != _(IDENTITY_MAGIC)) {
        /* Could not read 4 bytes, this is a bad file. */
        
        identity_file.Close();
        return false;
    }
    free(read_buffer);
    
    /* Store the length, then read the entire file contents, if it less than a threshold. */
    wxFileOffset length;
    length = identity_file.Length();
    
    if (length < 4 || length > IDENTITY_MAX_LENGTH) {
        DEBUG_LOG(_("AppOSX_Outlook> (ParseIdentity) the length (%d) of the identity file is too large."), length);
        
        identity_file.Close();
        return false;
    }
    
    read_buffer = (void*) malloc(length);
    memset(read_buffer, 0, length);
    
    identity_file.Seek(0);
    read_size = identity_file.Read(read_buffer, length);
    
    if (read_size != (length)) {
        DEBUG_LOG(_("AppOSX_Outlook> (ParseIdentity) read (%d) bytes does not match size (%d)."), (int) read_size, ((int) length));
        
        identity_file.Close();
        return false;
    }
    
    /* Fill in identity information (without acting on it). */
    identity.SetPath(full_path);
    identity.SetData(read_buffer, length);
    
    free(read_buffer);
    identity_file.Close();
    
    return true;
}

/* Tries to parse an identity file, should also try to verify using AppleScript. */
void AppOSX_Outlook::ParseIdentity(wxString identity_path)
{
    wxString full_path;
    
    full_path = wxFileName::GetHomeDir() + _(MAILDATA_RECORDS) + _(MAILDATA_IDENTITIES);
    full_path += _("/") + identity_path;
    
    AppOSX_OutlookIdentity identity;
    
    if (! this->GetIdentity(full_path, identity)) {
        /* Could not option/identity/read identity file. */
        return;
    }
    
    if (! identity.ParseMarc()) {
        /* Nothing left to do. */
        return;
    }
    
    /* Perform the parsed data formatting. */
    identity.ParseAccount();
    
    /* Add this account instance to the list of accounts. */
    this->info["accounts"].Append(identity.GetAccount());
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

bool AppOSX_Outlook::IsRunning()
{
    return ProcessManager::IsProcessRunning(BUNDLE_ID);
}

bool AppOSX_Outlook::StopApp()
{
    return ProcessManager::StopProcess(BUNDLE_ID);
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