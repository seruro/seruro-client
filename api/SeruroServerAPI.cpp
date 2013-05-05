
#include "SeruroServerAPI.h"
#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "../crypto/SeruroCrypto.h"

#include "../wxJSON/wx/jsonreader.h"
#include "../wxJSON/wx/jsonwriter.h"

DECLARE_APP(SeruroClient);
DEFINE_EVENT_TYPE(SERURO_API_RESULT);

const char hexa[] = {"0123456789ABCDEFabcdef"};
const char nonencode[] = {"abcdefghijklmnopqrstuvwqyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_~"};
void URLEncode(char* dest, const char* source, int length)
{
	/* Used to encode POST data. */
	int destc = 0, i, a, clear = 0;

	for (i = 0; (destc + 1) < length && source[i]; i++) {
		clear=0;
        for (a = 0; nonencode[a]; a++) {
            if (nonencode[a] == source[i]) {
                clear=1;
                break;
            }
        }
		if (! clear) {
            if((destc + 3) >= length) break;

			dest[destc] = '%';
            dest[destc+1] = hexa[source[i]/16];
            dest[destc+2] = hexa[source[i]%16];
            destc += 3;
		} else {
			dest[destc] = source[i];
            destc++;
		}
	}

	dest[destc] = '\0';
}

wxJSONValue parseResponse(wxString raw_response)
{
    wxJSONValue response;
    wxJSONReader response_reader;
    int num_errors;

    /* Parse the response (raw content) string into JSON. */
    num_errors = response_reader.Parse(raw_response, &response);
    if (num_errors > 0) {
        wxLogStatus(wxT("SeruroRequest> could not parse response data as JSON."));
    }
	if (! response.HasMember("success")) {
		wxLogMessage(wxT("SeruroRequest> response does not contain a 'success' key."));
		/* Fill in JSON data with TLS/connection error. */
		response["success"] = false;
        if (! response.HasMember("error")) {
            response["error"] = wxT("Connection failed.");
        }
	}
    
    /* Error may be a string, dict, or list? */
    if (! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroRequest> failed: (%s)."), response["error"].AsString());
	}
    
    /* Todo: consider checking for error string that indicated invalid token. */
    
    return response;
}

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

/* Async-Requesting:
 * All API calls are over TLS, when the request thread begins, create a Crypto object.
 * When the TLS request returns, create a SERURO_API_RESULT event.
 * The TLS response will be either (1) a JSON value, or (2) error state from the connection.
 * This response is attached to the new event and the event is queued for the caller to catch.
 */
wxThread::ExitCode SeruroRequest::Entry()
{
    wxJSONWriter writer;
    wxJSONValue response;
    wxString event_string;
    
    
	wxLogMessage("SeruroRequest> Thread started...");

    /* Check for params["auth"]["have_token"], if false perform GetAuthToken, fill in params["auth"]["token"]. */
    
    /* Perform Request, receive JSON response. */
    response = this->DoRequest();
       
    /* Match error of "Invalid authentication token." if true, perform GetAuthToken, fill in params["auth"]["token"]. */
    /* If true, perform request again (as token may have been outdated), receive JSON response. */
    
    /* Create request event, fill in JSON. */
    
	
	/* Alternatively, this could be passed as a string (both in an out actually), 
	 * and reassembled as a JSON object by the caller (JSONWriter, JSONReader). 
	 */
	writer.Write(response, event_string);

	/* The controller might be responsible for removing this memory, the wxWidgets API does not
	 * explicitly call out who owns this object. 
	 */
	wxCommandEvent evt(SERURO_API_RESULT, this->evtId);
	evt.SetString(event_string);

	/* Todo: is this a critical section? */
	this->evtHandler->AddPendingEvent(evt);

	wxLogMessage("SeruroRequest> Thread finished...");

	return (ExitCode)0;
}

wxString SeruroRequest::GetAuthToken()
{
    wxString token;
    /* Perform TLS request to create API session, receive a raw content (string) response. */
    
    /* Try to parse response as JSON (on failure, bail, meaning return blank token. */
    
    /* If "result" (boolean) is true, update token store for "email", and params["request"]["server"], set "token". */
    /* Warning, possible critical section. */
    wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
    
    /* Respond with "token" as string. */
    return token;
}

wxJSONValue SeruroRequest::DoRequest()
{
    wxJSONValue response;
    wxString raw_response;
    
	/* Get a crypto helper object for TLS requests. */
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
    
	/* Show debug statement, list the request. */
	wxLogMessage(wxT("SeruroRequest (TLS)> %s,%s,%s"),
        this->params["server"].AsString(), this->params["verb"].AsString(), this->params["object"].AsString());
    
	/* Perform the request, receive a raw content (string) response. */
	raw_response = cryptoHelper->TLSRequest(this->params["server"].AsString(), this->params["flags"].AsInt(),
        this->params["verb"].AsString(), this->params["object"].AsString(), this->params["data_string"].AsString());
    
	delete [] cryptoHelper;
    
    /* Try to parse response as JSON (on failure, bail, meaning fill in JSON error ourselves). */
    response = parseResponse(raw_response);
    
    return response;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth / GetRequest */
	params["auth"] = GetAuth();
	/* Todo: if ["auth"]["data"] then append to ["data"] */

	/* GetRequest will expect auth to exist. */
	params["request"] = GetRequest(name, params);
	/* Server name provided by SeruroConfig 
	 * This is passed to CreateRequest because a client may use multiple servers, in which case
	 * this was a selectable choosen by the user. 
	 */
	params["request"]["server"] = params["server"];

	wxString data_string;
	if (params["request"].HasMember("data")) {
		char *encoded = new char[256]; /* Todo: increate size of buffer. */
		wxString data_value;

		/* Construct a URL query string from JSON. */
		wxArrayString data_names = params["request"]["data"].GetMemberNames();
		for (size_t i = 0; i < data_names.size(); i++) {
			if (i > 0) data_string = data_string + wxString(wxT("&"));
			/* Value must be URLEncoded. */
			data_value = params["request"]["data"][data_names[i]].AsString();
			URLEncode(encoded, data_value.mbc_str(), 256);

			data_string = data_string + data_names[i] + wxString(wxT("=")) + 
				wxString(encoded);
		}
		delete encoded;
	}
	/* This exists by default for TLSRequest calling compatibility. */
	params["request"]["data_string"] = data_string;

	SeruroRequest *thread = new SeruroRequest(params["request"], evtHandler, evtId);

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI> Could not create thread."));
	}

	/* Add to datastructure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}

wxJSONValue SeruroServerAPI::GetAuth()
{
	/* return {"data": {}, "have_token": bool, "token": ""} */

	wxJSONValue auth;
	//auth["flags"] = SERURO_SECURITY_OPTIONS_NONE;
	
	/* If token is not present, add data object for authentication. */
	wxJSONValue data;
	auth["data"] = data;
	auth["data"]["user[email]"] = wxT("ted@valdrea.com");
	auth["data"]["user[password"] = wxT("password");


	return auth;
}

wxJSONValue SeruroServerAPI::GetRequest(api_name_t name, wxJSONValue params)
{
	wxJSONValue request;
	wxJSONValue data;

	/* All calls to the server are currently GETs. */
	request["verb"] = wxT("POST");
	request["flags"] = SERURO_SECURITY_OPTIONS_NONE;
	request["data"] = data; 

	wxLogMessage(wxT("SeruroServerAPI::GetRequest> Receive auth, options: %d, data: %s"), 
		params["auth"]["flags"].AsInt(), params["auth"]["data"].AsString());

	/* Switch over each API call and set it's URL */
	switch (name) {
		/* The SETUP api call (/api/setup) should return an encrypted P12 (using password auth) */
	case SERURO_API_GET_P12:
		request["object"] = wxT("getP12");
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember(wxT("search_string"))) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
		request["object"] = wxString(wxT("search/")) + params["search_string"].AsString();
		break;
	case SERURO_API_GET_CERT:
		if (! params.HasMember(wxT("request_address"))) {
			/* Error and stop!. */
		}
		request["object"] = wxString(wxT("cert/")) + params["request_address"].AsString();
		break;
	case SERURO_API_GET_CA:
		request["verb"] = wxT("GET");
		request["object"] = wxT("seruro/getCA");
		break;
	}

	/* Check to make sure server is in the params. */
	request["server"] = params["server"].AsString();

	/* Add optional auth data to request data. */
	if (params["auth"].HasMember("data")) {
		/* Todo: find out how to operator+ with wxJSONValues */
		request["data"]["auth"] = params["auth"]["data"]["auth"]; 
		//params["data"] = params["data"].AsMap().insert(
		//	params["auth"]["data"].AsMap().begin(), params["auth"]["data"].AsMap().end());
	}

	/* Add auth flags to request data. */
	request["flags"] = request["flags"].AsInt() | params["auth"]["flags"].AsInt();

	/* Add prefix of "/api/". */
	request["object"] = wxString(wxT("/api/")) + request["object"].AsString();
	return request;
}
