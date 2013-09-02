
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
    
    helper = new ServerMonitor();
    this->monitor_helpers.Add(helper);
    
    while (! this->TestDestroy()) {
        this->Sleep(this->poll_milli_delay);
        //DEBUG_LOG(_("SeruroMonitor> polling..."));
        this->Monitor();
    }
    
    return (wxThread::ExitCode)0;
}

void SeruroMonitor::Monitor()
{
    for (size_t i = 0; i < this->monitor_helpers.size(); ++i) {
        this->monitor_helpers[i]->Monitor();
    }
}