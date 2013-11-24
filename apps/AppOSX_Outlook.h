
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
    
    /* Requires parsing. */
    wxArrayString GetAccountList();
    account_status_t IdentityStatus(wxString address, wxString &server_uuid);
    
    /* Requires assembling. */
    bool AssignIdentity(wxString server_uuid, wxString address);
	bool UnassignIdentity(wxString address);
    
private:
    /* For each sub file within IDENTITIES, try to parse. */
    void ParseIdentity(wxString identity_path);
    bool GetIdentity(wxString full_path, AppOSX_OutlookIdentity &identity);
    
    /* Given a serial and subject, determine if CA exists. */
    //bool HasCert(wxString serial, wxString subject);
    
    bool GetInfo();
};

#endif /* OS Check */

#endif