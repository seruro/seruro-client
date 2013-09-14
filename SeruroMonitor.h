
#ifndef H_SeruroMonitorThread
#define H_SeruroMonitorThread

#include <wx/thread.h>

class SeruroClient;

/* Monitors should inherit from the MonitorHelper. */
class MonitorHelper
{
public:
    MonitorHelper() {}
    
    /* This mix-in makes life easy for the thread. */
    virtual bool Monitor() { return false; }
    /* This happens very quickly and should not include any type of blocking. */
    virtual bool FastMonitor() { return false; }
};

#include <wx/dynarray.h>
WX_DEFINE_ARRAY_PTR(MonitorHelper *, ArrayMonitorHelper);

class SeruroMonitor : public wxThread
{
public:
    SeruroMonitor(SeruroClient *client, size_t poll_milli_delay);
    virtual ~SeruroMonitor();
    
protected:
    virtual ExitCode Entry();
    void Monitor();
    void FastMonitor();
    
private:
    size_t poll_milli_delay;
    /* Will need to delete client's pointer. */
    SeruroClient *client;
    
    /* Helpers. */
    ArrayMonitorHelper monitor_helpers;
};

#endif
