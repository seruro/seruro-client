
#include "SeruroLogger.h"

#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/datetime.h>
    
bool SeruroLogger::InitLogger()
{
    buffer_logs = false;
    
    wxStandardPaths paths = wxStandardPaths::Get();
    wxFileName log_path(paths.GetUserDataDir(), _(SERURO_LOG_FILE_NAME));
    
    /* Add the config file name to the path. */
    log_opened = false;
    if (! wxFileName::DirExists(paths.GetUserDataDir())) {
        /* Ignore the return. */
        if (! wxFileName::Mkdir(paths.GetUserDataDir())) {
            return false;
        }
    }
    
    log_file = new wxTextFile(log_path.GetFullPath());
    if (! log_file->Exists()) {
        if (! log_file->Create()) {
            return false;
        }
    }
    
    log_opened = true;
    return true;
}

void SeruroLogger::WriteLog(wxLogLevel level, const wxString &msg)
{
    wxString text_line, date_time_string;
    
    /* Use a space separator. */
    date_time_string = wxDefaultDateTime.Now().FormatISOCombined(' ');
    
    /* Do not try to write to log if it is not valid. */
    if (! log_opened || ! log_file->Exists()) {
        if (! this->InitLogger()) {
            return;
        }
    }
    
    /* Add the type of error to the text output. */
    if (level <= wxLOG_Error) {
        text_line = "ERROR";
    } else if (level == wxLOG_Warning) {
        text_line = "WARN";
    } else if (level == wxLOG_Message) {
        text_line = "INFO";
    } else {
        text_line = "DEBUG";
    }
    /* Append it all together. */
    text_line = wxString::Format("[%s %s] %s", date_time_string, text_line, msg);
    
    /* Send the message to the virtual proxy. */
    ProxyLog(level, text_line);
    if (this->buffer_logs) {
        this->log_buffer.Append(wxString::Format(_("%s\n"), text_line));
    }
    
    /* Hold on to your butts. */
    log_file->Open();
	log_file->AddLine(text_line);
	log_file->Write();
	log_file->Close();
}

void SeruroLogger::DoLogTextAtLevel(wxLogLevel level, const wxString &msg)
{
    /* Capture debug messages too. */
    if (level == wxLOG_Debug) {
        this->WriteLog(level, msg);
    }
}

void SeruroLogger::DoLogRecord(wxLogLevel level, const wxString &msg, const wxLogRecordInfo &info)
{
    this->WriteLog(level, msg);
}
