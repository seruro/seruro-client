
#ifndef H_SeruroAppMonitorThread
#define H_SeruroAppMonitorThread

class AppMonitorThread : public wxThread
{
public:
    AppMonitorThread(size_t poll_delay);
    
    virtual ExitCode Entry();
    
};

#endif
