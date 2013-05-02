
#include "SeruroClient.h"
#include "SeruroConfig.h"
#include "wxJSON/wx/jsonreader.h"

#include <wx/stdpaths.h>

/* Set the UserAppData location of the expected config file. */
SeruroConfig::SeruroConfig()
{
    wxStandardPaths *paths = new wxStandardPaths();
	wxString configPath = paths->GetUserDataDir() + wxString(wxT("/")) + wxString(wxT(SERURO_CONFIG_NAME));

	VLDDisable();
    this->configFile = new wxTextFile(configPath);
	VLDEnable();
    delete [] paths;

    wxLogStatus(wxT("Config file: " + this->configFile->GetName()));
    if (! this->configFile->Exists()) {
		/* A config will be written using the SeruroSetup wizard. */
        wxLogMessage(wxT("Config does not exist"));
	}
    
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
	configFile->Close();

    wxLogStatus(configString);
    
    /* Parse the file into JSON. */
    wxJSONReader configReader;
	wxJSONValue tmpConfigData;
	VLDDisable();
    int numErrors = configReader.Parse(configString, &tmpConfigData);
	VLDEnable();
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        wxLogStatus(wxT("Error: could not parse config file."));
        return;
    }

	this->configData = &tmpConfigData;
    
	/* Config must have an array of "servers". */
	if (! configData->HasMember("servers") || ! (*configData)["servers"].IsArray()) {
		wxLogStatus(wxT("Config: could not find a 'servers' array."));
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
		for (int i = 0; i < (*configData)[member].Size(); i++) {
			values.Add((*configData)[member][i].AsString());
		}
	}
	return values;
}

bool SeruroConfig::HasConfig()
{
	/* Make a decision to run the SeruroSetup wizard. */
    return (this->configFile->Exists() && this->configValid);
}

/* Token management methods. */
wxJSONValue SeruroConfig::GetToken(wxString &server, wxString &address)
{
	wxJSONValue token_data;

	return token_data;
}

bool SeruroConfig::WriteToken(wxString &server, wxString &address, wxString &token)
{
	bool success = false;

	return success;
}
