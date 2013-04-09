
#ifndef H_SeruroServerAPI
#define H_SeruroServerAPI

#include <wx/socket.h>
#include "../wxJSON/wx/jsonval.h"

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
	SERURO_API_LOGIN
};

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
	SeruroRequest(api_name_t name, wxJSONValue params,wxEvtHandler *parent);
	virtual ~SeruroRequest();

	virtual void *Entry(); /* thread execution starts */
	/* Todo: consider an OnExit() is the thread can be terminated by user action. */

private:
	api_name_t name;
	wxJSONValue params;
	wxEvtHandler *evtHandler;
	/* Todo: consider naming the frame that spawns the request.
	 * to provide a progress update? */
};

class SeruroServerAPI
{
public:
	SeruroServerAPI(wxEvtHandler *caller) : evtHandler(caller) {}

	SeruroRequest *CreateRequest(api_name_t name, wxJSONValue params);
private:
	//SeruroRequest *thread;
	wxEvtHandler *evtHandler;
};


#endif
