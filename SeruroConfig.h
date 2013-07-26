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
		delete config_file;
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

	/***********************************************************/
	/************** TOKEN MANIPULATOR/ACCESSORS ****************/
	/***********************************************************/

	/* Token management, this file can exist before a config. */
	wxString GetToken(const wxString &server, const wxString &address);
	bool WriteToken(const wxString &server, const wxString &address, 
		const wxString &token);
	bool RemoveToken(const wxString &server_uuid,
		const wxString &address);
	bool RemoveTokens(const wxString &server_uuid);

	/* Set/get the last-known-good token. */
	bool SetActiveToken(const wxString &server_uuid, const wxString &account);
	wxString GetActiveToken(const wxString &server_uuid);

	/***********************************************************/
	/************** SERVER/ACCOUNT MANIPULATORS ****************/
	/***********************************************************/

	bool AddServer(wxJSONValue server_info);
	bool AddAddress(const wxString &server_uuid,
		const wxString &address);

	bool RemoveServer(wxString server_uuid);
	bool RemoveAddress(wxString server_uuid, wxString address);

	/***********************************************************/
	/************** SERVER/ACCOUNT ACCESSORS *******************/
	/***********************************************************/

	wxJSONValue   GetServers();
	wxJSONValue   GetServer(const wxString &server_uuid);
    wxString      GetServerUUID(const wxString &server_name); /* should be server_name, translation. */
    wxString      GetServerName(const wxString &server_uuid); /* reverse translation. */
	wxArrayString GetServerList(); /* deprecated */ 
    wxArrayString GetServerNames();
    
    wxArrayString GetAddressList(const wxString &server_uuid);
    
	bool ServerExists(wxJSONValue server_info);
	bool AddressExists(wxString server_uuid, wxString address);
	/* Helper to always convert the port to a long. */
	long GetPort(wxString server_uuid);
	long GetPortFromServer(wxJSONValue server_info);

	/* Reports the name (hostname:port) for the server. */
	wxString GetServerString(wxString server_uuid);

	/***********************************************************/
	/************** CERTIFICATE / FINGERPRINTING ***************/
	/***********************************************************/

	/* Identity related set/gets. */
	bool SetCAFingerprint(wxString server_uuid,
		wxString fingerprint);
	//wxString GetCAFingerprint();
	bool RemoveCACertificate(wxString server_uuid,
		bool write_config = false);
	bool AddIdentity(wxString server_uuid, wxString addresss,
		wxString fingerprint);
	bool RemoveIdentity(wxString server_uuid, wxString address,
		bool write_config = false);
	/* Each address under each server should be a list. */
	bool AddCertificate(wxString server_uuid, wxString address,
		wxString fingerprint);
	bool RemoveCertificates(wxString server_uuid, wxString address,
		bool write_config = false);

	/* May search the OS certificate store for the fingerprint. */
	bool HaveCertificates(wxString server_uuid, wxString address);
	bool HaveIdentity(wxString server_uuid, wxString address);
	bool HaveCA(wxString server_uuid);

	/* Fingerprint/thumbprint/hash retreival. */
    wxArrayString GetIdentityList(wxString server_uuid);
    wxArrayString GetCertificatesList(wxString server_uuid);
    
	wxArrayString GetCertificates(wxString server_uuid, wxString address);
	wxArrayString GetIdentity(wxString server_uuid, wxString address);
	wxString GetCA(wxString server_uuid);

protected:
	/* Used in previous 'testing builds'. */
	wxArrayString GetMemberArray(const wxString &member);

	bool AddFingerprint(wxString location, wxString server_uuid,
		wxString fingerprint, wxString address = wxEmptyString);
	bool RemoveFingerprints(wxString location, wxString server_uuid,
		wxString address = wxEmptyString);

private:
    bool InitConfig();
    
    bool config_valid;
    wxTextFile *config_file;
	wxJSONValue config;
};

#endif
