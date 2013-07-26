
#ifndef H_SeruroRequest
#define H_SeruroRequest

#include "../wxJSON/wx/jsonval.h"

#include <wx/thread.h>
#include <wx/event.h>

enum seruro_request_callbacks_t
{
    /* The token auth was invalid, prompt for credentials. */
    SERURO_REQUEST_CALLBACK_AUTH,
    SERURO_REQUEST_CALLBACK_NONE
};
    
/* When an API command finished it will add a SERURO_REQUEST_RESPONSE event.
 * This event should be caught by a single function and the event object
 * should be inspected to determine which API call returned, and the resultant
 * response parameters.
 */
//DECLARE_EVENT_TYPE(SERURO_REQUEST_RESULT, -1);

/* Create a new event type which holds JSON data, to prevent reading and 
 * writing JSON/string data for every request-response.
 * http://wiki.wxwidgets.org/Custom_Events
 */
class SeruroRequestEvent : public wxCommandEvent
{
public:
    /* The ID specifies the callback, the developer should not need to modify the command_type. */
	SeruroRequestEvent(int id = 0);
    
	SeruroRequestEvent(const SeruroRequestEvent &event) 
		: wxCommandEvent(event) { this->SetResponse(event.GetResponse()); }
	wxEvent* Clone() const { return new SeruroRequestEvent(*this); }

	wxJSONValue GetResponse() const { return response_data; }
	void SetResponse(wxJSONValue response) { response_data = response; }
    
    /* Event handler getter/setter. */
    void SetOriginalEvent(wxEvtHandler *event_handler, int event_id) {
        this->og_handler = event_handler;
        this->og_event_id = event_id;
    }
    wxEvtHandler *GetEventHandler() const { return this->og_handler; }
    int GetEventID() const { return this->og_event_id; }

private:
	wxJSONValue response_data;
    
    /* The response may create a susequent request, using the original event handler/id. */
    wxEvtHandler *og_handler;
    int og_event_id;
};

wxDECLARE_EVENT(SERURO_REQUEST_RESPONSE, SeruroRequestEvent);

/* Define a event type for API/Request responses. */
typedef void (wxEvtHandler::*SeruroRequestEventFunction) (SeruroRequestEvent &);

#define SeruroRequestEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
    wxStaticCastEvent(SeruroRequestEventFunction, &func)

#define EVT_SERURO_API_RESPONSE(type, fn) \
	DECLARE_EVENT_TABLE_ENTRY(SERURO_REQUEST_RESPONSE, type, -1, \
	(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
	wxStaticCastEvent(SeruroRequestEventFunction, &fn), (wxObject*) NULL),

#define EVT_SERURO_REQUEST(type, fn) EVT_SERURO_API_RESPONSE(type, fn)
#define EVT_SERURO_RESPONSE(type, fn) EVT_SERURO_API_RESPONSE(type, fn)

/* If a request required auth, perform the UI interaction and resubmit the request. */
void PerformRequestAuth(SeruroRequestEvent &event);

class SeruroRequest : public wxThread
{
public:
	SeruroRequest(wxJSONValue params, wxEvtHandler *parent, int parentEvtId);
	virtual ~SeruroRequest();
    
	virtual ExitCode Entry(); /* thread execution starts */
	/* Todo: consider an OnExit() is the thread can be terminated by user action. */
    
	/* Create the response event type and add it to the provided event handler. */
	void Reply(wxJSONValue response);
    /* The request has determined a parameter/auth failure, create a response. */
    void ReplyWithFailure();

protected:
    /* Creates a TLS Request (as an attempt) to create an authentication token. */
	//wxString GetAuthToken();
    bool DoAuth();
    /* Stop thread, and request a UI prompt to change auth credentials. */
    void RequestAuth();
    
    /* Performs the TLS Request (all of which require an authentication token. */
	wxJSONValue DoRequest();
    
private:
	wxJSONValue params;
    
	wxEvtHandler *evtHandler;
	int evtId;
	/* Todo: consider naming the frame that spawns the request.
	 * to provide a progress update? */
};

#endif