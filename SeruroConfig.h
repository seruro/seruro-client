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
	bool WriteConfig();
    bool HasConfig();

	/* Token management, this file can exist before a config. */
	wxString GetToken(const wxString &server, const wxString &address);
	bool WriteToken(const wxString &server, const wxString &address, 
		const wxString &token);
	bool RemoveToken(const wxString &server_name, 
		const wxString &address);
	bool RemoveTokens(const wxString &server_name);

	bool AddServer(wxJSONValue server_info);
	bool AddAddress(const wxString &server_name, 
		const wxString &address);

	bool RemoveServer(wxString server_name);
	bool RemoveAddress(wxString server_name, wxString address);

	/* Identity related set/gets. */
	bool SetCAFingerprint(wxString server_name,
		wxString fingerprint);
	//wxString GetCAFingerprint();
	bool RemoveCACertificate(wxString server_name,
		bool write_config = false);
	bool AddIdentity(wxString server_name, wxString addresss,
		wxString fingerprint);
	bool RemoveIdentity(wxString server_name, wxString address,
		bool write_config = false);
	/* Each address under each server should be a list. */
	bool AddCertificate(wxString server_name, wxString address,
		wxString fingerprint);
	bool RemoveCertificates(wxString server_name, wxString address,
		bool write_config = false);
	/* May search the OS certificate store for the fingerprint. */
	bool HaveCertificates(wxString server_name, wxString address);
	bool HaveIdentity(wxString server_name, wxString address);

	wxJSONValue GetServers();
	wxJSONValue GetServer(const wxString &server);
	wxArrayString GetAddressList(const wxString &server);
	wxArrayString GetServerList();

	bool ServerExists(wxJSONValue server_info);
	bool AddressExists(wxString server_name, wxString address);
	/* Helper to always convert the port to a long. */
	long GetPort(wxString server_name);
	long GetPortFromServer(wxJSONValue server_info);

	/* Reports the name (hostname:port) for the server. */
	wxString GetServerString(wxString server);

protected:
	/* Used in previous 'testing builds'. */
	wxArrayString GetMemberArray(const wxString &member);

	bool AddFingerprint(wxString location, wxString server_name,
		wxString fingerprint, 
		wxString address = wxEmptyString);
	bool RemoveFingerprints(wxString location, wxString server_name,
		wxString address = wxEmptyString);

private:
    bool configValid;
    wxTextFile *configFile;
	wxJSONValue configData;
};

#endif
