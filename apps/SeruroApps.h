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

class AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppHelper() { is_installed = false; is_detected = false; }
    
    virtual bool IsInstalled() { return false; }
    virtual wxString GetVersion() { return _("0"); }
    
    virtual wxArrayString GetAccountList() {
        wxArrayString empty_list;
        return empty_list;
    }
    virtual bool IsIdentityInstalled(wxString account_name) { return false; }
    
public:
    /* A secondary boolean indicating a success/failure while detecting the application status. */
    bool is_detected;
    bool is_installed;
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
    wxArrayString GetAppList() {
        return this->app_names;
    }
    
    /* Searches for the application and returns status: (installed, not, n/a), and version. */
    wxJSONValue GetApp(wxString app_name);
    
    /* A list of all the configured accounts for the given application. */
    wxArrayString GetAccountList(wxString app_name);
    /* Check if a 'Seruro' identity is configured/installed for the given app/account pair. */
    bool IsIdentityInstalled(wxString app_name, wxString account);

private:
    /* A cheap way to implement a map, since the API/doc is not available in offline OSX. */
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