#pragma once

#ifndef H_SeruroConfig
#define H_SeruroConfig

#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

#define SERURO_DEFAULT_PORT 4443
#define SERURO_DEFAULT_USER_AGENT "Client/1.0"

#define SERURO_SECURITY_OPTIONS_NONE    0x00
#define SERURO_SECURITY_OPTIONS_TLS12	0x01
#define SERURO_SECURITY_OPTIONS_STRONG	0x02
#define SERURO_SECURITY_OPTIONS_CLIENT  0x04
#define SERURO_SECURITY_OPTIONS_DATA	0x08

class SeruroConfig
{
public:
	SeruroConfig();

    /* OS locations:
       MSW(XP): <UserDir>/AppData/Roaming/Seruro/SeruroClient.config
       MSW(6+): <UserDir>/Application Data/Seruro/SeruroClient.config
       OSX: <UserDir>/Library/Seruro/SeruroClient.config
       LNX: <UserDir>/.seruro/SeruroClient.config
     */
	void LoadConfig();
	void WriteConfig();
    bool HasConfig();

	bool HasSyncCert();

	wxArrayString GetServers() { return GetMemberArray("servers"); }
	wxArrayString GetAddresses() { return GetMemberArray("addresses"); }
	wxString GetSyncSubjectFromServer(wxString &p_serverAddress) {
		/* Use the input server hostname to lookup the user's address, then append ' sync'. */
		wxString address(wxT("/CN=teddy.reed@gmail.com sync"));
		return address;
	}

protected:
	wxArrayString GetMemberArray(const wxString &member);

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
};

#endif
