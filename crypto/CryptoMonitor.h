
#ifndef H_SeruroCryptoMonitor
#define H_SeruroCryptoMonitor

#include "../SeruroMonitor.h"

class CryptoMonitor : public MonitorHelper
{
    CryptoMonitor() {}
    
    bool Monitor() { return true; }
};

#endif