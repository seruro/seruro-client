#pragma once

#if defined(__WXOSX__) || defined(__WXMAC__)

#ifndef H_AppOSX_Mail
#define H_AppOSX_Mail

#include "SeruroApps.h"

class AppOSX_Mail : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppOSX_Mail() : AppHelper() {
        can_assign = false;
        can_unassign = false;
    }
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    account_status_t IdentityStatus(wxString account_name, wxString &server_uuid);
    bool AssignIdentity(wxString server_uuid, wxString address);
    
    bool IsRunning();
    bool StopApp();
    bool StartApp();
    
private:
    bool GetInfo();
};

#endif

/* OS detection. */
#endif