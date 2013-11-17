
#ifndef H_AppOSX_Utils
#define H_AppOSX_Utils

#if defined(__WXOSX__) || defined(__WXMAC__)

#include <wx/string.h>

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CFStream.h>

bool ReadPList(wxString relative_location, CFMutableDictionaryRef &results_dict, bool relative = true);
bool AppInstalled(wxString bundle_string, wxString &app_location);
wxString AppVersion(wxString app_location);

/* From SeruroApps. */
wxString AsString(CFStringRef string);
wxString AsString(const void *value);

#endif /* OS Check */

#endif