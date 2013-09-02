
#ifndef H_SeruroAppMonitor
#define H_SeruroAppMonitor

#include "../SeruroMonitor.h"

class AppMonitor : public MonitorHelper
{
    AppMonitor() {}
    
    bool Monitor() { return true; }
};

#endif