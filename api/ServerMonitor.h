
#ifndef H_SeruroServerMonitor
#define H_SeruroServerMonitor

#include "../SeruroMonitor.h"

#include "../wxJSON/wx/jsonval.h"

class ServerMonitor : public MonitorHelper
{
    ServerMonitor() {}
    
    bool Monitor();
    
private:
    /* The server will reduce the info to a single
     * API call, the results will trigger various installs. */
    bool AddContacts(wxJSONValue contacts);
    bool RevokeCertificates(wxJSONValue certificates);
    bool RemoveContacts(wxJSONValue contacts) { return true; }
    
    /* Check the CA/CRL status. */
    
    /* Check your own status. */
    bool RevokeAccount(wxJSONValue account);
    /* Set some status icon saying new certs are available. */
    bool NotifyAccount(wxJSONValue account);
};

#endif