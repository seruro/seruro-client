#pragma once

#ifndef H_SeruroConfig
#define H_SeruroConfig

#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

#define SERURO_DEFAULT_PORT 8080
#define SERURO_DEFAULT_USER_AGENT "SeruroClient/1.0"

#define SERURO_SECURITY_OPTIONS_TLS12	0x0001
#define SERURO_SECURITY_OPTIONS_STRONG	0x0010
#define SERURO_SECURITY_OPTIONS_CLIENT  0x0100
#define SERURO_SECURITY_OPTIONS_DATA	0x1000

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

protected:
	wxArrayString GetMemberArray(const wxString &member);

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
};

#endif
