
#ifndef H_SeruroServerAPI
#define H_SeruroServerAPI

#include <wx/socket.h>
#include "../wxJSON/wx/jsonval.h"

#include "../Defs.h"

/* API commands are identified by enum macro, and the API params are manually
 * parsed and assembled by the call handler.
 */
enum api_name_t
{
	SERURO_API_SEARCH,
	SERURO_API_GET_CERT,
	SERURO_API_GET_CA,
	SERURO_API_GET_CRL,
	SERURO_API_GET_SYNC_CERT,
	SERURO_API_GET_P12
};

/* Define the API routes (all prefixed with /api) */

/* When an API command finished it will add a SERURO_API_RESULT event.
 * This event should be caught by a single function and the event object
 * should be inspected to determine which API call returned, and the resultant
 * response parameters. 
 */
DECLARE_EVENT_TYPE(SERURO_API_RESULT, -1);

/* The SeruroServerAPI is a socket-based RESTful client for the SeruroServer.
 * When the SeruroClient must make an API call a thread is spawned (or run)
 * which takes the call command and optional parameters and data.
 */

class SeruroRequest : public wxThread
{
public:
	SeruroRequest(wxJSONValue params, wxEvtHandler *parent, int parentEvtId);
	virtual ~SeruroRequest();

	virtual void *Entry(); /* thread execution starts */
	/* Todo: consider an OnExit() is the thread can be terminated by user action. */

protected:
    /* Creates a TLS Request (as an attempt) to create an authentication token. */
	wxString GetAuthToken();
    /* Performs the TLS Request (all of which require an authentication token. */
	wxJSONValue DoRequest();

private:
	wxJSONValue params;

	wxEvtHandler *evtHandler;
	int evtId;
	/* Todo: consider naming the frame that spawns the request.
	 * to provide a progress update? */
};

class SeruroServerAPI
{
public:
	SeruroServerAPI(wxEvtHandler *caller) : evtHandler(caller) {}

	/* Must provide the API name, params, and callback event ID */
	SeruroRequest *CreateRequest(api_name_t name, wxJSONValue params, int evtId);

protected:
	/* Return an auth object {"auth": {"token": "", "have_token": bool}}.
	 * If have_token is false, the call will block to retreive a username/password.
	 * Finally auth["data"] will be filled in appropriately.
	 * Otherwise token will be assembled and passed as a query variable.
	 */
	wxJSONValue GetAuth(wxString &server);

	/* Given an API identifier, this function will return a JSON propery with
	 * the required REQUEST parameters (including verb, object, servername.
	 * Params will be embedded into the request URL or as headers. */
	wxJSONValue GetRequest(api_name_t name, wxJSONValue params);

private:
	//SeruroRequest *thread;
	wxEvtHandler *evtHandler;
};

/* The auth dialog is a beefed up password dialog with an added selection for
 * username (or email address). Since a single server may have multiple addresses
 * configured. 
 */
class AuthDialog : public wxDialog
{
public:
    AuthDialog(wxString &server);

	/* Return the address/password pair. */
	wxJSONValue GetValues();

	/* Note: there are no events registered, since the auth dialog should
	 * only be used as a modal. 
	 */

private:
	wxTextCtrl *address_control;
	wxTextCtrl *password_control;
};

/* DecryptDialog will accept a "method", meaning the method the decryption key 
 * should have been communicated to the user.
 *
 * See Defs.h for TEXT_DECRYPT_METHOD_... for various textual messages.
 */
class DecryptDialog : public wxDialog
{
public:
	DecryptDialog(wxString &method);

	/* Return the entered password. */
	wxString GetValue();
private:
	wxTextCtrl *password_control;
};


#endif
