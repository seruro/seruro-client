
#include "SeruroMonitor.h"
#include "SeruroClient.h"
#include "logging/SeruroLogger.h"

#include <wx/time.h>

SeruroMonitor::SeruroMonitor(SeruroClient *client, size_t poll_milli_delay) : wxThread(wxTHREAD_DETACHED)
{
    this->client = client;
    this->poll_milli_delay = poll_milli_delay;
}

SeruroMonitor::~SeruroMonitor()
{
    /* Reach into the client's DS and remove the thread pointer. */
    wxCriticalSectionLocker enter(this->client->seruro_critsection_monitor);
    this->client->DeleteMonitor();
}

wxThread::ExitCode SeruroMonitor::Entry()
{
    while (! this->TestDestroy()) {
        this->Sleep(this->poll_milli_delay);
        //DEBUG_LOG(_("SeruroMonitor> polling..."));
    }
    
    return (wxThread::ExitCode)0;
}