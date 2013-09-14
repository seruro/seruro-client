
#ifndef H_SeruroAppMonitor
#define H_SeruroAppMonitor

#include "../SeruroMonitor.h"
#include "../wxJSON/wx/jsonval.h"


class AppMonitor : public MonitorHelper
{
public:
    AppMonitor();
    
    /* The AppMonitor must check for running applications quickly. */
    bool Monitor() { return true; }
    bool FastMonitor();
    
private:
    /* Create an event for the main thread alerting that a pending application was closed. */
    void CreateCloseEvent(wxString app_name);

    wxJSONValue pending_closed;
};

#endif