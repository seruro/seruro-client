
#include "SeruroApps.h"
#include "../SeruroClient.h"

//#define MAX_PATH_SIZE 1024

DECLARE_APP(SeruroClient);

wxString UUIDFromFingerprint(const wxString &fingerprint)
{
    wxArrayString server_list;
    wxArrayString account_list;
    wxArrayString identity_list;
    
    /* Find the first server which contains an account that has the given fingerprint. */
    server_list = wxGetApp().config->GetServerList();
    for (size_t i = 0; i < server_list.size(); i++) {
        account_list = wxGetApp().config->GetAddressList(server_list[i]);
        for (size_t j = 0; j < account_list.size(); j++) {
            identity_list = wxGetApp().config->GetIdentity(server_list[i], account_list[j]);
            for (size_t k = 0; k < identity_list.size(); k++) {
                if (identity_list[k] == fingerprint) {
                    return server_list[i];
                }
            }
        }
    }
    return wxEmptyString;
}

#if defined(__WXOSX__) || defined(__WXMAC__)
/* OSX includes. */
#include "AppOSX_Mail.h"
#include <CoreFoundation/CFStream.h>

/* Converts a CF string into a wx string. */
wxString AsString(CFStringRef string)
{
    char *path;
    /* CFString lengths are in UTF 16 pairs. */
    size_t length = CFStringGetLength(string) * 2 * sizeof(char);
    
    /* Allocate and convert. */
    path = (char *) malloc(length);
    CFStringGetCString(string, path, length, kCFStringEncodingASCII);
    wxString path_string = _(path);
    delete path;
    
    return path_string;
}

/* Converts an expected CF string from CF collection into a wx string. */
wxString AsString(const void *value)
{
    //CFStringRef cf_string;
    //wxString encoded = wxEmptyString;
    
    if (CFGetTypeID(value) != CFStringGetTypeID()) {
        /* If the actual type is not a string, return an empty representation. */
        //return encoded;
        wxEmptyString;
    }
    
    //cf_string = (CFString)
    //return encoded;
    return AsString((CFStringRef) value);
}

void SeruroApps::InitOSX()
{
    AppHelper *helper;
    
    /* Example, create the helper, add the string association. */
    helper = (AppHelper *) new AppOSX_Mail();
    AddAppHelper(_("OSX Mail"), helper);
}

#endif

#if defined(__WXMSW__)
/* MS windows includes. */

#include "AppMSW_LiveMail.h"
#include "AppMSW_Outlook.h"

void SeruroApps::InitMSW()
{
    AppHelper *helper;

	helper = (AppHelper *) new AppMSW_LiveMail();
	AddAppHelper(_("Windows Live Mail"), helper);
}

#endif

#if defined(__WXGTK__)
/* GTK includes. */

void SeruroApps::InitGTK()
{
    
}

#endif

SeruroApps::SeruroApps()
{
    this->app_count = 0;
    
    /* Start each app helper. */
#if defined(__WXMSW__)
    this->InitMSW();
#elif defined(__WXOSX__) || defined(__WXMAC__)
    this->InitOSX();
#elif defined(__WXGTK__)
    this->InitGTK();
#else
    wxLogMessage(_("SeruroApps> (ctor) cannot determine OS."));
    return;
#endif
}

AppHelper* SeruroApps::GetHelper(wxString app_name)
{
    AppHelper *helper = 0;
    
    for (size_t i = 0; i < app_names.size(); i++) {
        if (app_name.compare(app_names[i]) == 0) {
            helper = app_helpers[i];
        }
    }
    return helper;
}

wxArrayString SeruroApps::GetAccountList(wxString app_name)
{
    wxArrayString accounts;
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    
    if (helper == 0) return accounts;
    return helper->GetAccountList();
}

wxJSONValue SeruroApps::GetApp(wxString app_name)
{
    wxJSONValue app_info;
    AppHelper *helper;
    
    /* Search the list of supported apps for the app given by name. */
    helper = this->GetHelper(app_name);
    
    /* Did not find the app, this should not happen. */
    if (helper == 0) return app_info;
    
    app_info["version"] = helper->GetVersion();
    //bool is_installed =
    /* Allow status as a tri-state. */
    if (helper->IsInstalled()) {
        app_info["status"] = _("Installed");
    } else if (! helper->is_detected) {
        app_info["status"] = _("N/A");
    } else {
        app_info["status"] = _("Not Installed");
    }
    
    return app_info;
}

account_status_t SeruroApps::IdentityStatus(wxString app_name, wxString account_name, wxString &server_uuid)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return APP_UNASSIGNED;
    
    return helper->IdentityStatus(account_name, server_uuid);
}

bool SeruroApps::AssignIdentity(wxString app_name, wxString server_uuid, wxString address)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->AssignIdentity(server_uuid, address);
}

bool SeruroApps::CanAssign(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->can_assign;
}

bool SeruroApps::CanUnassign(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->can_unassign;
}

void SeruroApps::AddAppHelper(wxString app_name, AppHelper *app_helper)
{
    this->app_names.Add(app_name);
    this->app_helpers[app_count++] = app_helper;
}
