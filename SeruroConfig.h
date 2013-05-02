#pragma once

#ifndef H_SeruroConfig
#define H_SeruroConfig

#include "Defs.h"

#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

class SeruroConfig
{
public:
	SeruroConfig();
	~SeruroConfig() {
		delete configFile;
		//delete configData;
	}

    /* OS locations:
       MSW(XP): <UserDir>/AppData/Roaming/Seruro/SeruroClient.config
       MSW(6+): <UserDir>/Application Data/Seruro/SeruroClient.config
       OSX: <UserDir>/Library/Seruro/SeruroClient.config
       LNX: <UserDir>/.seruro/SeruroClient.config
     */
	void LoadConfig();
	void WriteConfig();
    bool HasConfig();

	/* Token management, this file can exist before a config. */
	wxJSONValue GetToken(wxString &server, wxString &address);
	bool WriteToken(wxString &server, wxString &address, wxString &token);

	wxArrayString GetServers() { return GetMemberArray("servers"); }

protected:
	wxArrayString GetMemberArray(const wxString &member);

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue *configData;
};

#endif
