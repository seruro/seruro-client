#pragma once

#if defined(__WXOSX__) || defined(__WXMAC__)

#ifndef H_AppOSX_Mail
#define H_AppOSX_Mail

#include "SeruroApps.h"

class AppOSX_Mail : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppOSX_Mail() : AppHelper() {}
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    bool IsIdentityInstalled(wxString account_name);
    
private:
    bool GetInfo();
    wxJSONValue info;
};

#endif

/* OS detection. */
#endif