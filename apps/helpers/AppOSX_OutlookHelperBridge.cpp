
#if defined(__WXOSX__) || defined(__WXMAC__)

#import "AppOSX_OutlookHelper.h"
#include "AppOSX_OutlookHelperBridge.h"

#include <wx/osx/core/cfstring.h>

namespace CocoaBridge {
    bool addContact(wxString address, wxString first_name, wxString last_name) {
        
        BOOL result;
        
        //CFStringRef cf_address = wxCFStringRef(address);
        NSString *ns_address = wxCFStringRef(address).AsNSString();
        NSString *ns_first_name = wxCFStringRef(first_name).AsNSString();
        NSString *ns_last_name = wxCFStringRef(last_name).AsNSString();
        
        result = [AppOSX_OutlookHelper addContactWithEmail:ns_address firstName:ns_first_name lastName:ns_last_name];
        
        [ns_address release];
        [ns_first_name release];
        [ns_last_name release];
        
        return (result == true);
    }
    
    
}

#endif /* OS Check */