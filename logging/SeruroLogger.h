
#ifndef H_SeruroLogger
#define H_SeruroLogger

#include <wx/log.h>
#include <wx/textfile.h>

#define SERURO_LOG_FILE_NAME "seruro.log"
#define SERURO_USE_LOGGER       1
#define SERURO_USE_SETTINGSLOG  1

/***
 * Production logs: wxLogMessage
 * Debug log:       wxLogStatus
 ***/

#define ERROR_LOG wxLogError
#define DEBUG_LOG wxLogStatus
#define LOG       wxLogMessage

class SeruroLoggerTarget
/* Any class may inherit from SeruroLoggerTarget, making it available as a 
 * target to the client. 
 */
{
public:
    SeruroLoggerTarget() {}
    
    virtual void ProxyLog(wxLogLevel level, const wxString &msg) {}
};

class SeruroLogger : public wxLog
{
public:
    SeruroLogger() { log_opened = false; }
    /* For some reason the ctor is not always called. */
    bool InitLogger();

    /* Allow a premptive logger to store results for a lazy-initiated logger. */
    void ToggleBuffer(bool enable=true) { this->buffer_logs = enable; }
    wxString GetBuffer() {
        /* Retreive and remove buffer. */
        wxString buffer = this->log_buffer;
        this->log_buffer.Clear();
        return buffer;
    }
    
    void SetProxyTarget(SeruroLoggerTarget *new_target) {
        this->log_target = new_target;
        if (new_target != 0) {
            this->has_target = true;
        }
    }
    void RemoveProxyTarget() {
        this->has_target = false;
        this->log_target = 0;
    }
    
protected:
    void DoLogTextAtLevel(wxLogLevel level, const wxString &msg);
    void DoLogRecord(wxLogLevel level, const wxString &msg, const wxLogRecordInfo &info);
    //virtual void ProxyLog(wxLogLevel level, const wxString &msg) {}
    //virtual wxString GetLog() { return wxEmptyString; }
    
private:
    /* Write log to text file and proxy to other inheritors. */
    void WriteLog(wxLogLevel level, const wxString &msg);
	bool CreateLog();
    
    /* The container for the logs. */
    wxTextFile *log_file;
    bool log_opened;
	/* Determine if the log is attempting initialization. */
    bool log_initializing;


    bool buffer_logs;
    wxString log_buffer;
    
    SeruroLoggerTarget *log_target;
    bool has_target;
};

#endif