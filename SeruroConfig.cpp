
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
    
	/* Config must have "servers" and "addresses", and they must be arrays. */
	if (! configData.HasMember("servers") || ! configData.HasMember("addresses") ||
		! configData["servers"].IsArray() || ! configData["addresses"].IsArray()) {
		wxLogStatus(wxT("Config: could not find a required object array (servers, addresses)."));
		return;
	}

    /* Indicate that the config is valid. */
    this->configValid = true;
}

wxArrayString SeruroConfig::GetMemberArray(const wxString &member)
{
	/* Semi pointless check. */
	wxArrayString values;
	if (HasConfig()) {
		for (int i = 0; i < configData[member].Size(); i++) {
			values.Add(configData[member][i].AsString());
		}
	}
	return values;
}

bool SeruroConfig::HasConfig()
{
    return (this->configFile->Exists() && this->configValid);
}


