
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
};

class SeruroMonitor : public wxThread
{
public:
    SeruroMonitor(SeruroClient *client, size_t poll_milli_delay);
    virtual ~SeruroMonitor();
    
protected:
    virtual ExitCode Entry();
    
private:
    size_t poll_milli_delay;
    /* Will need to delete client's pointer. */
    SeruroClient *client;
};

#endif
