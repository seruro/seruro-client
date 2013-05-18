
#ifndef H_SeruroRequest
#define H_SeruroRequest

#include "../wxJSON/wx/jsonval.h"

#include <wx/thread.h>
#include <wx/event.h>

/* When an API command finished it will add a SERURO_API_RESULT event.
 * This event should be caught by a single function and the event object
 * should be inspected to determine which API call returned, and the resultant
 * response parameters.
 */
DECLARE_EVENT_TYPE(SERURO_API_RESULT, -1);

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

#endif