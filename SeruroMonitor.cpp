
#include "SeruroMonitor.h"
#include "SeruroClient.h"
#include "logging/SeruroLogger.h"

#include "api/ServerMonitor.h"

#include <wx/time.h>

SeruroMonitor::SeruroMonitor(SeruroClient *client, size_t poll_milli_delay) : wxThread(wxTHREAD_DETACHED)
{
    this->client = client;
    this->poll_milli_delay = poll_milli_delay;
}

SeruroMonitor::~SeruroMonitor()
{
    /* Remove all helpers. */
    for (size_t i = 0; i < this->monitor_helpers.size(); ++i) {
        delete this->monitor_helpers[i];
    }
    
    /* Reach into the client's DS and remove the thread pointer. */
    wxCriticalSectionLocker enter(this->client->seruro_critsection_monitor);
    this->client->DeleteMonitor();
}

wxThread::ExitCode SeruroMonitor::Entry()
{
    /* Create helpers. */
    MonitorHelper *helper;
    size_t delay_counter;
    
    helper = new ServerMonitor();
    this->monitor_helpers.Add(helper);
    
    DEBUG_LOG(_("SeruroMonitor> Monitor thread started..."));
    
    delay_counter = 0;
    while (! this->TestDestroy()) {        
        if (delay_counter <= 0) {
            //DEBUG_LOG(_("SeruroMonitor> polling..."));
            this->Monitor();
            delay_counter = this->poll_milli_delay/100;
        }
        
        /* This quick-wait allows TestDestroy to run often. */
        this->Sleep(100);
        ++delay_counter;
    }
    
    return (wxThread::ExitCode)0;
}

void SeruroMonitor::Monitor()
{
    for (size_t i = 0; i < this->monitor_helpers.size(); ++i) {
        this->monitor_helpers[i]->Monitor();
    }
}
