#pragma once

#ifndef H_SeruroConfig
#define H_SeruroConfig

#include <wx/textfile.h>
#include "wxJSON/wx/jsonval.h"

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

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
};

#endif
