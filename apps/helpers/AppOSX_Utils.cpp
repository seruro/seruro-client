
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_Utils.h"
#include "../../logging/SeruroLogger.h"
#include "../../api/Utils.h"

#include <wx/filename.h>

#define VERSION_KEY "CFBundleShortVersionString"
#define VERSION_PLIST "/Contents/version.plist"

/* Is application installed, use the application bundle ID. */
bool AppInstalled(wxString bundle_string, wxString &app_location)
{
    OSStatus success;
    CFURLRef app_url;
    
    
    CFStringRef bundle_id;
    
    bundle_id = CFStringCreateWithCString(kCFAllocatorDefault, AsChar(bundle_string), kCFStringEncodingASCII);
    success = LSFindApplicationForInfo(kLSUnknownCreator, bundle_id, NULL, NULL, &app_url);

    if (success != 0) {
        if (success != kLSApplicationNotFoundErr) {
            /* There was a problem detecting the application. */
            DEBUG_LOG(_("AppOSX_Utils> error (%d) detection application."), success);
        }
        CFRelease(bundle_id);
        return false;
    }
    
    CFStringRef app_string;
    wxString app_path_string;
    
    app_string = CFURLCopyFileSystemPath(app_url, kCFURLPOSIXPathStyle);
    app_path_string = AsString(app_string);
    
    CFRelease(app_string);
    CFRelease(bundle_id);
    CFRelease(app_url);
    
    app_location.Append(app_path_string);
    
    return (success == 0);
}

/* Read the MailData PList (with the option of reading relative to the user's dir). */
bool ReadPList(wxString relative_location, CFMutableDictionaryRef &results_dict, bool relative)
{
    CFURLRef plist_url; /* File location to accounts plist. */
    CFPropertyListRef properties; /* list data structure. */
    CFDataRef resource_data; /* File contents. */
    /* Error/state handleing. */
    SInt32 error_code = 0;
    CFErrorRef error_string;
    bool success;
    
    /* Get url to account data. */
    wxString plist_url_path;
    CFStringRef plist_url_cfpath;
    
    if (relative) {
        plist_url_path = wxString(wxFileName::GetHomeDir() + relative_location);
    } else {
        /* This is actually an absolute path. */
        plist_url_path = wxString(relative_location);
    }
    
    plist_url_cfpath = CFStringCreateWithCString(kCFAllocatorDefault, plist_url_path.mb_str(wxConvUTF8), kCFStringEncodingMacRoman);
    plist_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, plist_url_cfpath, kCFURLPOSIXPathStyle, false);
    /* Load file contents. */
    success = CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, plist_url, &resource_data, NULL, NULL, &error_code);
    CFRelease(plist_url);
    
    if (!success || error_code != 0) {
        //DEBUG_LOG(_("AppOSX_Utils> (ReadPList) failed to read (error= %d)."), error_code);
        return false;
    }
    
    /* This is deprecated. */
    properties = CFPropertyListCreateWithData(kCFAllocatorDefault, resource_data, kCFPropertyListImmutable, NULL, &error_string);
    CFRelease(resource_data);
    
    /* Make sure the property list can be represented as a dictionary. */
    if (CFGetTypeID(properties) != CFDictionaryGetTypeID()) {
        CFRelease(properties);
        
        DEBUG_LOG(_("AppOSX_Utils> (ReadPList) plist data is not a dictionary."));
        return false;
    }
    
    results_dict = (CFMutableDictionaryRef) properties;
    return true;
}

wxString AppVersion(wxString app_location)
{
    wxString app_version;
    wxString version_location;
    CFMutableDictionaryRef properties;
    
    version_location = app_location + _(VERSION_PLIST);
    if (! ReadPList(version_location, properties, false)) {
        DEBUG_LOG(_("AppOSX_Utils> (GetVersion) could not read verson plist."));
        return _("unknown");
    }
    
    /* Key must be a CFStringRef. */
    if (! CFDictionaryContainsKey(properties, CFSTR(VERSION_KEY))) {
        DEBUG_LOG(_("AppOSX_Utils> (GetVersion) 'CFBundleVersion' not in dictionary."));
        
        CFRelease(properties);
        return _("unknown");
    }
    
    const void *version_value;
    CFStringRef version_cfstring;
    
    version_value = CFDictionaryGetValue(properties, CFSTR(VERSION_KEY));
    
    if (CFGetTypeID(version_value) != CFStringGetTypeID()) {
        DEBUG_LOG(_("AppOSX_Utils> (GetVersion) 'CFBundleVersion' is not a string type."));
        
        CFRelease(properties);
        return _("unknown");
    }
    
    version_cfstring = (CFStringRef) version_value;
    app_version = AsString(version_cfstring);
    
    //CFRelease(version_cfstring);
    CFRelease(properties);
    
    return app_version;
}

/* Converts a CF string into a wx string. */
wxString AsString(CFStringRef string)
{
    wxString path_string;
    char *path;
    size_t length = 0;
    
    /* CFString lengths are in UTF 16 pairs. */
    length = CFStringGetLength(string) * 2 * sizeof(char);
    
    /* Allocate and convert. */
    path = (char *) malloc(length);
    CFStringGetCString(string, path, length, kCFStringEncodingASCII);
    path_string = _(path);
    delete path;
    
    return path_string;
}

/* Converts an expected CF string from CF collection into a wx string. */
wxString AsString(const void *value)
{
    if (CFGetTypeID(value) != CFStringGetTypeID()) {
        /* If the actual type is not a string, return an empty representation. */
        return wxEmptyString;
    }
    
    return AsString((CFStringRef) value);
}


#endif /* OSX Check */