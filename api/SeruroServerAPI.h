
#ifndef H_SeruroServerAPI
#define H_SeruroServerAPI

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
	wxJSONValue GetServer(const wxString &server);
	/* Must provide the API name, params, and callback event ID */
	SeruroRequest *CreateRequest(api_name_t name, 
		wxJSONValue params, int evtId);

	/* Special API calls. */
	SeruroRequest *Ping(wxJSONValue params) {
		return CreateRequest(SERURO_API_PING, params,
			SERURO_API_CALLBACK_PING);
	}

	/* API callbacks */
	bool InstallP12(wxJSONValue response);
	bool InstallCA(wxJSONValue response);
	bool InstallCert(wxJSONValue response);

protected:
	/* Return an auth object {"auth": {"token": "", "have_token": bool}}.
	 * If have_token is false, the call will block to retreive a username/password.
	 * Finally auth["data"] will be filled in appropriately.
	 * Otherwise token will be assembled and passed as a query variable.
	 */
	wxJSONValue GetAuth(const wxString &server,
		const wxString &address = wxEmptyString);

	/* Given an API identifier, this function will return a JSON propery with
	 * the required REQUEST parameters (including verb, object, servername.
	 * Params will be embedded into the request URL or as headers. */
	wxJSONValue GetRequest(api_name_t name, wxJSONValue params);

private:
	//SeruroRequest *thread;
	wxEvtHandler *evtHandler;
};


#endif
