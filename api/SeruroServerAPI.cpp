
#include "SeruroServerAPI.h"
#include "../SeruroClient.h"

DECLARE_APP(SeruroClient);
DEFINE_EVENT_TYPE(SERURO_API_RESULT);

SeruroRequest::SeruroRequest(wxJSONValue api_params, wxEvtHandler *parent, int parentEvtId) 
	: wxThread(), params(api_params), evtHandler(parent), evtId(parentEvtId) {}

SeruroRequest::~SeruroRequest()
{
	/* Start client (all) threads accessor, to delete this, with a critial section. */
	wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);

	wxArrayThread& threads = wxGetApp().seruro_threads;
	threads.Remove(this);

	/* Todo: (check for an empty threads array) might want to signal to main thread if
	 * this was the last item, meaning it is OK to shutdown if it were waiting. */
	
}

wxThread::ExitCode SeruroRequest::Entry()
{
	wxLogMessage("Seruro Thread started...");

	/* Do some stuff */

	wxLogMessage("Seruro Thread finished...");

	//wxJSONValue *response = new wxJSONValue(wxT("data"));
	wxJSONValue response;
	response[wxT("data")] = wxT("data");
	//response[wxT("data")] = wxT("data");
	wxCommandEvent evt(SERURO_API_RESULT, this->evtId);

	/* The controller might be responsible for remove this memory, the wxWidgets API does not
	 * explicitly call out who owns this object. 
	 */
	/* Alternatively, this could be passed as a string (both in an out actually), 
	 * and reassembled as a JSON object by the caller. 
	 */
	evt.SetClientData((void *) new wxJSONValue(response));

	/* Not a critical section? */
	this->evtHandler->AddPendingEvent(evt);

	return (ExitCode)0;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth */
	params[wxT("auth")] = GetAuth(name);
	params[wxT("request")] = GetRequest(name, params);

	SeruroRequest *thread = new SeruroRequest(params, evtHandler, evtId);

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI> Could not create thread."));
	}

	/* Add to datastructure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}

wxJSONValue SeruroServerAPI::GetAuth(api_name_t name)
{
	wxJSONValue auth;
	
	if (name == SERURO_API_SETUP) {
		auth[wxT("auth")] = wxT("username\nsha1ofmypasswordanddata");
		auth[wxT("data")] = wxT("today");
	} else if (false) {
		/* Todo: replace false with check for client-ssl-certificate (sync cert). */
	}

	return auth;
}

wxJSONValue SeruroServerAPI::GetRequest(api_name_t name, wxJSONValue params)
{
	wxJSONValue request;

	/* All calls to the server are currently GETs. */
	request[wxT("type")] = wxT("GET");
	/* Switch over each API call and set it's URL */
	switch (name) {
	case SERURO_API_SETUP:
		request[wxT("url")] = wxT("setup");
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember(wxT("search_string"))) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
		request[wxT("url")] = wxString(wxT("search/")) + params[wxT("search_string")].AsString();
		break;
	case SERURO_API_GET_CERT:
		if (! params.HasMember(wxT("request_address"))) {
			/* Error and stop!. */
		}
		request[wxT("url")] = wxString(wxT("cert/")) + params[wxT("request_address")].AsString();
	}

	/* Add prefix of "/api/". */
	request[wxT("url")] = wxString(wxT("/api/")) + request[wxT("url")].AsString();
	return request;
}
