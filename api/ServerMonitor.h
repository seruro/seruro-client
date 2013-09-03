
#ifndef H_SeruroServerMonitor
#define H_SeruroServerMonitor

#include "../SeruroMonitor.h"
#include "SeruroRequest.h"

#include "../wxJSON/wx/jsonval.h"

//#include <wx/dynarray.h>
//WX_DEFINE_ARRAY_SIZE_T(size_t, wxArrayRequests);

class ServerMonitor : public MonitorHelper
{
public:
    ServerMonitor();
    
    bool Monitor();
    
private:
    /* Request callbacks. */
    void OnUpdateResponse(SeruroRequestEvent &event);
    
    /* The server will reduce the info to a single
     * API call, the results will trigger various installs. */
    bool ProcessContacts(wxString server_uuid, wxJSONValue contacts);
    //bool RemoveContacts(wxJSONValue contacts) { return true; }
    
    bool ProcessCertificates(wxString server_uuid, wxJSONValue certificates);
    //bool RevokeCertificate(wxJSONValue certificates) { return true; }
    
    /* Check the CA/CRL status. */
    bool NotifyAuthority(wxJSONValue authority) { return true; }
    
    /* Check your own status. */
    bool RevokeAccount(wxJSONValue account) { return true; }
    /* Set some status icon saying new certs are available. */
    bool NotifyAccount(wxJSONValue account) { return true; }
    
    /* Keep track of running requests, do not send multiple. */
    wxJSONValue requests;
};

#endif