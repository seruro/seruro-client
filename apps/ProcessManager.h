
#ifndef H_SeruroProcessManager
#define H_SeruroProcessManager

#include <wx/string.h>

#define APPS_RESTART_DELAY 500
#define APPS_RESTART_MAX_COUNT 10

#if defined(__WXMSW__)

class ProcessManagerMSW
{
public:
    static bool IsProcessRunning(wxString process_name);
    static bool StartProcess(wxString process_path);
    static bool StopProcess(wxString process_name);
    
    ProcessManagerMSW () { }
};

#endif

#if defined(__WXMAC__) || defined(__WXOSX__)

class ProcessManagerOSX
{
public:
    static bool IsProcessRunning(wxString process_name);
    static bool StartProcess(wxString process_path);
    static bool StopProcess(wxString process_name);
    
    ProcessManagerOSX () { }
};

#endif

#if defined(__WXMSW__)
class ProcessManager : public ProcessManagerMSW
#endif
#if defined(__WXMAC__)
class ProcessManager : public ProcessManagerOSX
#endif
{
public:
    //static bool IsProcessRunning(wxString process_name);
    //static bool StopProcess(wxString process_name);
    //static bool StartProcess(wxString process_name);
    
private:
    ProcessManager() { }
};

#endif
