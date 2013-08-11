#pragma once

#if defined(__WXMSW__)

#ifndef H_AppMSW_LiveMail
#define H_AppMSW_LiveMail

#include "SeruroApps.h"

class AppMSW_LiveMail : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppMSW_LiveMail() : AppHelper() {}
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    bool IsIdentityInstalled(wxString account_name);
    
private:
    bool GetInfo();
};

#endif

/* OS detection. */
#endif