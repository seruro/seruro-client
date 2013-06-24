
#include "SeruroApps.h"

//#define MAX_PATH_SIZE 1024

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

void SeruroApps::InitMSW()
{
    
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

wxArrayString SeruroApps::GetAccountList(wxString app_name)
{
    wxArrayString accounts;
    AppHelper *helper = 0;
    
    for (size_t i = 0; i < app_names.size(); i++) {
        if (app_name.compare(app_names[i]) == 0) {
            helper = app_helpers[i];
        }
    }
    
    if (helper == 0) return accounts;
    return helper->GetAccountList();
}

wxJSONValue SeruroApps::GetApp(wxString app_name)
{
    wxJSONValue app_info;
    AppHelper *helper = 0;
    
    /* Search the list of supported apps for the app given by name. */
    for (size_t i = 0; i < app_names.size(); i++) {
        if (app_name.compare(app_names[i]) == 0) {
            helper = app_helpers[i];
        }
    }
    
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

void SeruroApps::AddAppHelper(wxString app_name, AppHelper *app_helper)
{
    this->app_names.Add(app_name);
    this->app_helpers[app_count++] = app_helper;
}
