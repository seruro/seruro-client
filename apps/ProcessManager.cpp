
#include "ProcessManager.h"
#include "SeruroApps.h"

/* Include the AsChar function */
#include "../crypto/SeruroCryptoMAC.h"

#if defined(__WXMAC__) || defined(__WXOSX__)

//#include <sys/sysctl.h>
#include <Carbon/Carbon.h>

/* Quiting the application. */

pid_t GetPIDFromBundleID(wxString bundle_id)
{
    ProcessSerialNumber psn;
    CFDictionaryRef process_info;
    CFStringRef bundle_string;
    
    wxString current_bundle_id;
    pid_t pid;

    psn.lowLongOfPSN = kNoProcess;
    psn.highLongOfPSN = 0;
    pid = 0;
    
    //kCFBundleIdentifierKey
    while (GetNextProcess(&psn) == noErr) {
        process_info = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
        bundle_string = (CFStringRef) CFDictionaryGetValue(process_info, kCFBundleIdentifierKey);
        CFRelease(process_info);
        
        /* There might not have been a value for bundle_id in the dict. */
        if (bundle_string != NULL) {
            current_bundle_id = AsString(bundle_string);
            CFRelease(bundle_string);
        } else {
            continue;
        }
            
        if (current_bundle_id == bundle_id) {
            GetProcessPID(&psn, &pid);
            break;
        }
    }
    
    return pid;
}

bool ProcessManagerOSX::IsProcessRunning(wxString process_name)
{
    return (GetPIDFromBundleID(process_name) != 0);
}

bool ProcessManagerOSX::StopProcess(wxString process_name)
{
    pid_t process_pid = GetPIDFromBundleID(process_name);
    
    /* The given process name was not running. */
    if (process_pid == 0) {
        return false;
    }
    
    /* Ask the application to "quit" using SIGTERM. */
    killpg(process_pid, SIGTERM);
    return true;
}

bool ProcessManagerOSX::StartProcess(wxString process_path)
{ 
    CFURLRef app_reference;
    CFStringRef bundle_string;
    FSRef fs_reference;
    OSErr error_num;
    
    /* Do not start another instance of the application. */
    if (ProcessManagerOSX::IsProcessRunning(process_path)) {
        return true;
    }
    
    bundle_string = CFStringCreateWithCString(kCFAllocatorDefault, AsChar(process_path), kCFStringEncodingASCII);
    error_num = LSFindApplicationForInfo(kLSUnknownCreator, bundle_string, NULL, NULL, &app_reference);
    CFRelease(bundle_string);
    
    if (error_num != noErr) {
        /* There's a problem, send to debug log. */
        return false;
    }
    
    /* This should be using LSApplicationParameters and LSOpenApplication */
    if (! CFURLGetFSRef(app_reference, &fs_reference)) {
        /* Problem getting FS reference? */
        return false;
    }
    
    LSApplicationParameters launch_parameters = {0, kLSLaunchDefaults};
    launch_parameters.application = &fs_reference;
    
    /* No need to get PSN. */
    error_num = LSOpenApplication(&launch_parameters, NULL);
    //error_num = LSOpenCFURLRef(app_reference, NULL);
    //CFRelease(app_reference);
    
    if (error_num != noErr) {
        /* There's a problem opening the application. */
        return false;
    }
    
    return true;
}

#endif