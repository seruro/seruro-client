
#include "SeruroClient.h"
#include "SeruroConfig.h"
#include "wxJSON/wx/jsonreader.h"

#include <wx/stdpaths.h>

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    wxStandardPaths *paths = new wxStandardPaths();
    this->configFile = new wxTextFile(paths->GetUserDataDir() + wxT("/") + wxT(SERURO_CONFIG_NAME));
    
    wxLogStatus(wxT("Config file: " + this->configFile->GetName()));
    if (! this->configFile->Exists())
        wxLogMessage(wxT("Config does not exist"));
    
    this->configValid = false;
    this->LoadConfig();

    /* May be a good idea to remove loadConfig, and put the code here. */

}

void SeruroConfig::LoadConfig()
{
    if (! this->configFile->Exists())
        /* Cannot load a non-existing config file. */
        return;
    
    /* Read entire file into string. */
    wxString configString;
    configFile->Open();
    while (! configFile->Eof()) {
        configString += configFile->GetNextLine() + wxT("\n");
    }
    wxLogStatus(configString);
    
    /* Parse the file into JSON. */
    wxJSONReader configReader;
    int numErrors = configReader.Parse(configString, &configData);
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        wxLogStatus(wxT("Error: could not parse config file."));
        return;
    }
    
    /* Indicate that the config is valid. */
    this->configValid = true;
}

bool SeruroConfig::HasConfig()
{
    return (this->configFile->Exists() && this->configValid);
}


