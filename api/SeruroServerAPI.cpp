
#include "SeruroServerAPI.h"
#include "../SeruroClient.h"

DECLARE_APP(SeruroClient);
DEFINE_EVENT_TYPE(SERURO_API_RESULT);

SeruroRequest::SeruroRequest(api_name_t api_name, wxJSONValue api_params, wxEvtHandler *parent) 
	: wxThread(), name(api_name), params(api_params), evtHandler(parent) {}

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
	wxCommandEvent evt(SERURO_API_RESULT, wxID_ANY);

	/* The controller might be responsible for remove this memory, the wxWidgets API does not
	 * explicitly call out who owns this object. 
	 */
	/* Alternatively, this could be passed as a string (both in an out actually), 
	 * and reassembled as a JSON object by the caller. 
	 */
	evt.SetClientData((void *) new wxJSONValue(response));

	/* Not a critical section? */
	evtHandler->AddPendingEvent(evt);

	return (ExitCode)0;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params)
{
	SeruroRequest *thread = new SeruroRequest(name, params, evtHandler);

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI> Could not create thread."));
	}

	/* Add to datastructure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}