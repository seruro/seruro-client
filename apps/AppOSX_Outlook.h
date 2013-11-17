
#ifndef H_AppOSX_Outlook
#define H_AppOSX_Outlook

#if defined(__WXOSX__) || defined(__WXMAC__)

#include "helpers/AppOSX_OutlookIdentity.h"
#include "SeruroApps.h"

class AppOSX_Outlook : public AppHelper
{
public:
    AppOSX_Outlook() : AppHelper() {
        can_assign = true;
        can_unassign = false;
        
        needs_restart = true;
    }
    
    bool IsInstalled();
    wxString GetVersion();
    
    bool IsRunning();
    bool StopApp();
    bool StartApp();
    
    wxArrayString GetAccountList();
    
private:
    /* For each sub file within IDENTITIES, try to parse. */
    void ParseIdentity(wxString identity_path);
    bool GetIdentity(wxString full_path, AppOSX_OutlookIdentity &identity);
    
    bool GetInfo();
};

#endif /* OS Check */

#endif