
#ifndef H_AppOSX_OutlookIdentity
#define H_AppOSX_OutlookIdentity

#if defined(__WXOSX__) || defined(__WXMAC__)

/* This class provides a utility into Microsoft's MaRC identity file (Mail Account).
 * The MaRC parsing is an example of horrible interfaces to controlling S/MIME characteristics.
 */

#include <wx/string.h>
#include <wx/memory.h>

#include "../../wxJSON/wx/jsonval.h"

class AppOSX_OutlookIdentity
{
public:
    AppOSX_OutlookIdentity() {}
    ~AppOSX_OutlookIdentity() {}
    
    void SetPath(wxString path);
    void SetData(void *data, size_t length);
    bool ParseMarc();
    
    wxJSONValue GetAccount();
    
private:
    wxMemoryBuffer raw_data;
    wxJSONValue account;
    
};

#endif /* OS Check */

#endif