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
	wxString GetToken(const wxString &server, const wxString &address);
	bool WriteToken(const wxString &server, const wxString &address, 
		const wxString &token);

	wxJSONValue GetServers();
	wxJSONValue GetServer(const wxString &server);
	wxArrayString GetAddressList(const wxString &server);
	wxArrayString GetServerList();

	/* Does not have to be part of the config. */
	wxString GetServerString(wxString server);

protected:
	/* Used in previous 'testing builds'. */
	wxArrayString GetMemberArray(const wxString &member);

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
};

#endif
