
#include "SeruroServerAPI.h"
#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "../crypto/SeruroCrypto.h"

#include "../wxJSON/wx/jsonreader.h"

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
	wxLogMessage("SeruroRequest> Thread started...");

	/* Do some stuff */
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
    
	wxLogMessage(wxT("SeruroRequest (TLS)> %s,%s,%s"),
		params["server"].AsString(), params["request"]["verb"].AsString(), params["request"]["object"].AsString());
	wxString response = cryptoHelper->TLSRequest(params["server"].AsString(), params["request"]["flags"].AsInt(),
		params["request"]["verb"].AsString(), params["request"]["object"].AsString());

	delete [] cryptoHelper;

	wxCommandEvent evt(SERURO_API_RESULT, this->evtId);

	/* The controller might be responsible for remove this memory, the wxWidgets API does not
	 * explicitly call out who owns this object. 
	 */
	/* Alternatively, this could be passed as a string (both in an out actually), 
	 * and reassembled as a JSON object by the caller. 
	 */
    /* Parse the file into JSON. */
    wxJSONReader responseReader;
	wxJSONValue responseData;
    int numErrors = responseReader.Parse(response, &responseData);
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        wxLogStatus(wxT("SeruroRequest> could not parse response data."));
    }
	evt.SetClientData((void *) new wxJSONValue(responseData));

	/* Not a critical section? */
	this->evtHandler->AddPendingEvent(evt);

	wxLogMessage("SeruroRequest> Thread finished...");

	return (ExitCode)0;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth */
	if (name == SERURO_API_SETUP) {
		/* The API TLS request WILL require client authentication for API calls other than SETUP.
		 * Otherwise the server server will allow unauthenticated client requests over TLS, but 
		 * WILL require an auth header. If the client makes a client authenticated TLS request 
		 * the crypto helper will attempt to find a valid client cert and will fail implicitly.
		 */
		params[wxT("auth")] = GetAuth(name);
	}
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
	request[wxT("verb")] = wxT("GET");
	request[wxT("flags")] = SERURO_SECURITY_OPTIONS_NONE;

	/* Switch over each API call and set it's URL */
	switch (name) {
	case SERURO_API_SETUP:
		request[wxT("object")] = wxT("setup");
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember(wxT("search_string"))) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
		request[wxT("object")] = wxString(wxT("search/")) + params[wxT("search_string")].AsString();
		break;
	case SERURO_API_GET_CERT:
		if (! params.HasMember(wxT("request_address"))) {
			/* Error and stop!. */
		}
		request[wxT("object")] = wxString(wxT("cert/")) + params[wxT("request_address")].AsString();
	}

	/* Check to make sure server is in the params. */
	request[wxT("server")] = params[wxT("server")].AsString();

	/* Add prefix of "/api/". */
	request[wxT("object")] = wxString(wxT("/api/")) + request[wxT("object")].AsString();
	return request;
}
