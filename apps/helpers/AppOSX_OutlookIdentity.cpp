
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_OutlookIdentity.h"

void AppOSX_OutlookIdentity::SetPath(wxString path)
{
    /* A simple wrapper to set the path. */
    this->account["path"] = path;
}

void AppOSX_OutlookIdentity::SetData(void *data, size_t length)
{
    /* Copy memory to internal storage while the identity is created. */
    this->raw_data.AppendData(data, length);
}

bool AppOSX_OutlookIdentity::ParseMarc()
{
    /* The magic string is missing (it was read earlier). */
    
    
    return true;
}

wxJSONValue AppOSX_OutlookIdentity::GetAccount()
{
    /* Return the private account data. */
    return this->account;
}

#endif /* OS Check */