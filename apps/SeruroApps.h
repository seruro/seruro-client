#pragma once

#ifndef H_SeruroApps
#define H_SeruroApps

#include <wx/wx.h>
#include "../wxJSON/wx/jsonval.h"
//#include <map>

#if defined(__WXOSX__) || defined(__WXMAC__)
wxString AsString(CFStringRef string);
//wxString AsStringFromCollection(const void *value)
wxString AsString(const void *value);
//wxString AsStringIfExists(CFDictionaryRef dict, const void* key);
#endif

#if defined(__WXMSW__)
#include <wx/msw/registry.h>

/* Base key on both 32 and 64bit. */
#define HKLM_BIT_ROOT "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"
/* Key within install key to query version and name. */
#define HKLM_KEY_PROPERTIES L"InstallProperties"

/* Registry accessor (abstraction to get 32-bit and 64-bit). */
wxRegKey *GetInstallKey(wxString key_install,
	wxRegKey::StdKey hive = wxRegKey::HKLM, wxString base = HKLM_BIT_ROOT);


#endif

/* Each account has a current state or status. */
enum account_status_t
{
    APP_ASSIGNED = 1,
    /* Certificates are in use. */
    APP_ALTERNATE_ASSIGNED = 2,
    /* Certificates are in use, but not an identity managed by Seruro. */
    APP_UNASSIGNED = 3
    /* No Certificates are un use. */
};

wxString UUIDFromFingerprint(const wxString &fingerprint);

class AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppHelper() { 
		is_installed = false; 
		is_detected = false;
		this->info = wxJSONValue(wxJSONTYPE_OBJECT);
        
        /* Helpers for actions. */
        can_assign = true;
        can_unassign = true;
	}
    
    virtual bool IsInstalled() { return false; }
    virtual wxString GetVersion() { return _("0"); }
    
    virtual wxArrayString GetAccountList() {
        wxArrayString empty_list;
        return empty_list;
    }
    /* Resolve the address to a canonical account "name". */
    virtual wxString GetAccountName(wxString address) {
        return address;
    }
    
    virtual account_status_t IdentityStatus(wxString address,
        wxString &server_uuid) {
        /* If the identity is APP_ASSIGNED, then fill in the server_uuid. */
        return APP_UNASSIGNED;
    }
	virtual bool AssignIdentity(wxString server_uuid, wxString address) {
		return false;
	}

    /* Action helpers. */
    bool can_assign;
    bool can_unassign;
    
public:
    /* A secondary boolean indicating a success/failure
	 * while detecting the application status. */
    bool is_detected;
    bool is_installed;

	wxJSONValue info;
};

class SeruroApps
{
public:
    SeruroApps();
    ~SeruroApps() {
        while (app_count > 0) {
            delete app_helpers[--app_count];
        }
    }
    
    /* Returns a list of all initialized application helpers. */
    wxArrayString GetAppList();
    wxArrayString GetAppList(wxArrayString whitelist);
    
    /* Searches for the application and returns status: 
	 * (installed, not, n/a), and version. */
    wxJSONValue GetApp(wxString app_name);
    
    /* A list of all the configured accounts for the given application. */
    wxArrayString GetAccountList(wxString app_name);
    wxArrayString GetAccountList(wxString app_name, wxArrayString whitelist);
    /* Get the canonical name for the account (from the address). */
    wxString GetAccountName(wxString app_name, wxString address);
    
    /* Check if a 'Seruro' identity is configured/installed 
	 * the given app/account pair. */
    account_status_t IdentityStatus(wxString app_name,
        wxString address, wxString &server_uuid);
    
	bool AssignIdentity(wxString app_name,
		wxString server_uuid, wxString address);

    bool CanAssign(wxString app_name);
    bool CanUnassign(wxString app_name);

private:
    AppHelper* GetHelper(wxString app_name);
    
    /* A cheap way to implement a map, since the API/doc 
	 * is not available in offline OSX. */
    wxArrayString app_names;
    AppHelper *app_helpers[32];
    size_t app_count;

    /* OS app initializers (add app helpers). */
    void InitOSX();
    void InitMSW();
    void InitGTK();
    
    /* Add a string/application pointer. */
    void AddAppHelper(wxString app_name, AppHelper* app_helper);
};

#endif