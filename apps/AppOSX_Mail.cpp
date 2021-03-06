
#if defined(__WXOSX__) || defined(__WXMAC__)

#include <wx/filename.h>
#include <wx/string.h>
#include <wx/log.h>

#include "AppOSX_Mail.h"
#include "ProcessManager.h"
#include "helpers/AppOSX_Utils.h"

#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../crypto/SeruroCrypto.h"
#include "../logging/SeruroLogger.h"

/* When looking up OSX applications ( from PLIST ):
 * <key>CFBundleIdentifier</key>
 * <string>com.apple.mail</string>
 
 * Version: CFBundleShortVersionString
 * or /Applications/<AppName>.app/version.plist
 * Attributes: AccountName, EmailAddresses, FullUserName, Hostname, Username
 */

#include <ApplicationServices/ApplicationServices.h>

#define BUNDLE_ID "com.apple.mail"
#define MAILDATA_PLIST "/Library/Mail/V2/MailData/Accounts.plist"

DECLARE_APP(SeruroClient);

bool AppOSX_Mail::IsRunning()
{
    return ProcessManager::IsProcessRunning(BUNDLE_ID);
}

bool AppOSX_Mail::StopApp()
{
    return ProcessManager::StopProcess(BUNDLE_ID);
}

bool AppOSX_Mail::StartApp()
{
    return ProcessManager::StartProcess(BUNDLE_ID);
}

bool AppOSX_Mail::IsInstalled()
{
    if (! GetInfo()) return false;
    
    return this->is_installed;
}

wxString AppOSX_Mail::GetVersion()
{
    if (! GetInfo()) return _("N/A");
    
    if (this->info.HasMember("version")) {
        return info["version"].AsString();
    }
    
    this->info["version"] = AppVersion(this->info["location"].AsString());
    return this->info["version"].AsString();
}

wxArrayString AppOSX_Mail::GetAccountList()
{
    wxArrayString accounts;
    
    if (! GetInfo()) return accounts;
    
    CFMutableDictionaryRef properties;
    //CFStringRef key;
    if (! ReadPList(_(MAILDATA_PLIST), properties)) {
        DEBUG_LOG(_("AppOSX_Mail> (GetAccountsList) could not read plist."));
        
        return accounts;
    }

    /* dict: MailAccounts, array: [0](dict): AccountType: LocalAccount (ignore), */
    
    /* Key must be a CFStringRef. */
    if (! CFDictionaryContainsKey(properties, CFSTR("MailAccounts"))) {
        DEBUG_LOG(_("AppleOSX_Mail> (GetAccountList) 'MailAccounts' not in dictionary."));
        
        CFRelease(properties);
        return accounts;
    }
    
    /* MailAccounts should be an array of all the accounts configured. */
    const void *accounts_value;
    CFArrayRef accounts_list;
    accounts_value = CFDictionaryGetValue(properties, CFSTR("MailAccounts"));
    
    if (CFGetTypeID(accounts_value) != CFArrayGetTypeID()) {
        wxLogMessage(_("AppleOSX_Mail> (GetAccountList) 'MailAccounts' is not an array."));
        return accounts;
    }
    
    accounts_list = (CFArrayRef) accounts_value;
    CFIndex num_accounts = CFArrayGetCount(accounts_list);
    /* Each account is a dictionary (within the account array). */
    const void *account_value;
    CFDictionaryRef account_dict;
    
    /* Within a valid account is a list of email addresses. */
    const void *addresses_value;
    CFArrayRef addresses_list;
    /* We are interested in multiple dictionary string values. */
    wxString account_type, account_name, address;
    
    for (int i = 0; i < num_accounts; i++) {
        account_value = CFArrayGetValueAtIndex(accounts_list, i);
        /* Accounts must be dictionary representations. */
        if (CFGetTypeID(account_value) != CFDictionaryGetTypeID()) {
            continue;
        }
        account_dict = (CFDictionaryRef) account_value;
        /* They must contain an AccountType and AccountName. */
        if (! CFDictionaryContainsKey(account_dict, CFSTR("AccountType"))) {
            continue;
        }
        account_type = AsString(CFDictionaryGetValue(account_dict, CFSTR("AccountType")));
        /* We are NOT interested in local accounts. */
        if (account_type.compare(_("LocalAccount")) == 0) {
            continue;
        }
        /* The account SHOULD have a name. */
        if (! CFDictionaryContainsKey(account_dict, CFSTR("AccountName")) ||
            ! CFDictionaryContainsKey(account_dict, CFSTR("EmailAddresses"))) {
            continue;
        }
        account_name = AsString(CFDictionaryGetValue(account_dict, CFSTR("AccountName")));
        
        addresses_value = CFDictionaryGetValue(account_dict, CFSTR("EmailAddresses"));
        /* Email addresses must be a list. */
        if (CFGetTypeID(addresses_value) != CFArrayGetTypeID()) {
            continue;
        }
        addresses_list = (CFArrayRef) addresses_value;
        for (int j = 0; j < CFArrayGetCount(addresses_list); j++) {
            address = AsString(CFArrayGetValueAtIndex(addresses_list, j));
            
            wxLogMessage(_("AppOSX_Mail> (GetAccountList) found address (%s), account name (%s)."), address, account_name);
            accounts.Add(address);
        }
    }
    
    /* All other object are still owned by properties. */
    CFRelease(properties);
    
    return accounts;
}

account_status_t AppOSX_Mail::IdentityStatus(wxString account_name, wxString &server_uuid)
{
    wxArrayString server_list, account_list;
    SeruroCrypto crypto;
    
    server_uuid.Clear();
    if (! GetInfo()) return APP_UNASSIGNED;

    /* OSX will use a matching identity automatically. */
    /* Check for any account with an installed identity, matching the account name. */
    server_list = theSeruroConfig::Get().GetServerList();
    
    for (size_t i = 0; i < server_list.size(); i++) {
        account_list = theSeruroConfig::Get().GetAddressList(server_list[i]);
        
        for (size_t j = 0; j < account_list.size(); j++) {
            if (account_name.compare(account_list[j]) != 0) continue;
            if (crypto.HaveIdentity(server_list[i], account_list[j])) {
                
                /* Fill in the appropriate server. */
                server_uuid.Append(server_list[i]);
                return APP_ASSIGNED;
            }
        }
    }
    
    return APP_UNASSIGNED;
}

bool AppOSX_Mail::AssignIdentity(wxString server_uuid, wxString address)
{
    SeruroCrypto crypto;
    
    /* OSX Mail will auto-detect the certificates. */
    return crypto.HaveIdentity(server_uuid, address);
}

bool AppOSX_Mail::GetInfo()
{
    OSStatus success;
    
    /* GetInfo should only occur once. */
    if (this->is_detected) {
        return true;
    }
    
    CFStringRef bundle_id = CFStringCreateWithCString(kCFAllocatorDefault, BUNDLE_ID, kCFStringEncodingASCII);
    CFURLRef app_url;
    //FSRef app_fs;
    
    success = LSFindApplicationForInfo(kLSUnknownCreator, bundle_id, NULL, NULL, &app_url);
    
    if (success != 0) {
        if (success != kLSApplicationNotFoundErr) {
            /* There was a problem detecting the application. */
            wxLogMessage(_("AppOSX_Outlook> error (%d) detection application."), success);
            this->is_detected = false;
        }
        CFRelease(bundle_id);
        
        this->is_detected = true;
        return false;
    }
    
    /* For debugging. */
    CFStringRef app_string = CFURLCopyFileSystemPath(app_url, kCFURLPOSIXPathStyle);
    wxString path_string = AsString(app_string);
    
    wxLogMessage(_("AppOSX_Outlook> url: (%s)."), path_string);
    
    this->is_detected = true;
    this->is_installed = true;
    this->info["location"] = path_string;
    
    CFRelease(app_string);
    CFRelease(bundle_id);
    CFRelease(app_url);
    
    return (success == 0);
}

/* OS detection. */
#endif
