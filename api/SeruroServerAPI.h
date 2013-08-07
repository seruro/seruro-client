
#ifndef H_SeruroServerAPI
#define H_SeruroServerAPI

#include <wx/window.h>

#include "../Defs.h"
#include "SeruroRequest.h"

#include "../wxJSON/wx/jsonval.h"

/* API commands are identified by enum macro, and the API params are manually
 * parsed and assembled by the call handler.
 */
enum api_name_t
{
	SERURO_API_SEARCH,
	SERURO_API_CERTS,
	SERURO_API_CA,
	SERURO_API_CRL,
	SERURO_API_P12S,
	/* Special API call. */
	SERURO_API_PING
};

/* Used by Create Request to bind result to callback method. */
enum seruro_api_callbacks_t
{
	SERURO_API_CALLBACK_CA,
	SERURO_API_CALLBACK_P12S,
	SERURO_API_CALLBACK_CERTS,
	SERURO_API_CALLBACK_CRL,
	SERURO_API_CALLBACK_SEARCH,
	/* Special API call. */
	SERURO_API_CALLBACK_PING
};

//extern enum api_name api_name_t;
//extern enum seruro_api_callbacks seruro_api_callbacks_t;

/* The SeruroServerAPI is a socket-based RESTful client for the SeruroServer.
 * When the SeruroClient must make an API call a thread is spawned (or run)
 * which takes the call command and optional parameters and data.
 */

class SeruroServerAPI
{
public:
	SeruroServerAPI(wxEvtHandler *caller) : 
	  evtHandler(caller) {}
	SeruroServerAPI(wxWindow *caller) :
	  evtHandler(caller->GetEventHandler()) {}

	/* Helper functions for those who do not want to DECLARE_APP 
	 * for access to the config methods. 
	 */
	wxJSONValue GetServer(const wxString &server_uuid);
    
	/* Must provide the API name, params, and callback event ID */
	SeruroRequest *CreateRequest(api_name_t name, 
		wxJSONValue params, int evtId);

	/* Special API calls. */
	SeruroRequest *Ping(wxJSONValue params) {
		return CreateRequest(SERURO_API_PING, params,
			SERURO_API_CALLBACK_PING);
	}

	/* API callbacks */
	//bool InstallP12(wxJSONValue response, wxJSONValue unlock_codes, bool force_install = false);
    bool InstallP12(wxString server_uuid, wxString address, wxString cert_type,
        wxString encoded_p12, wxString unlock_code, bool force_install = false);
	bool InstallCA(wxJSONValue response);
	bool InstallCertificate(wxJSONValue response);
    
    /* Todo: maybe this isn't the best place for removals. */
    bool UninstallIdentity(wxString server_uuid, wxString address);
    bool UninstallCertificates(wxString server_uuid, wxString address);
    bool UninstallCA(wxString server_uuid);

protected:
	/* Return an auth object {"auth": {"token": "", "have_token": bool}}.
	 * If have_token is false, the call will block to retreive a username/password.
	 * Finally auth["data"] will be filled in appropriately.
	 * Otherwise token will be assembled and passed as a query variable.
	 */
	//wxJSONValue GetAuth(const wxString &server,
	//	const wxString &address = wxEmptyString);
    wxJSONValue GetAuth(wxJSONValue params);

	/* Given an API identifier, this function will return a JSON propery with
	 * the required REQUEST parameters (including verb, object, servername.
	 * Params will be embedded into the request URL or as headers. */
	wxJSONValue GetRequest(api_name_t name, wxJSONValue params);

private:
	//SeruroRequest *thread;
	wxEvtHandler *evtHandler;
};


#endif
