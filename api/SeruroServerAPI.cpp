
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
	wxLogMessage("SeruroRequest> Thread started...");

	/* Do some stuff */
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
    
	wxLogMessage(wxT("SeruroRequest (TLS)> %s,%s,%s"),
		params["server"].AsString(), params["verb"].AsString(), params["object"].AsString());

	wxString response = cryptoHelper->TLSRequest(params["server"].AsString(), params["flags"].AsInt(),
		params["verb"].AsString(), params["object"].AsString(), params["data_string"].AsString());

	delete [] cryptoHelper;

	/* The controller might be responsible for remove this memory, the wxWidgets API does not
	 * explicitly call out who owns this object. 
	 */
	wxCommandEvent evt(SERURO_API_RESULT, this->evtId);

	/* Parse the file into JSON. */
    wxJSONReader responseReader;
	wxJSONValue responseData;
    int numErrors = responseReader.Parse(response, &responseData);
    if (numErrors > 0) {
        //wxLogMessage(reader.GetErrors());
        wxLogStatus(wxT("SeruroRequest> could not parse response data."));
    }
	if (! responseData.HasMember("result")) {
		wxLogMessage(wxT("SeruroRequest> response does not contain a result."));
		/* Fill in JSON data with TLS/connection error. */
		responseData["result"] = false;
		responseData["error"] = wxT("Connection failed");
	} else if (! responseData["result"].AsBool()) {
		wxLogMessage(wxT("SeruroRequest> failed: %s."), responseData["error"].AsString());
	}
	
	/* Alternatively, this could be passed as a string (both in an out actually), 
	 * and reassembled as a JSON object by the caller (JSONWriter, JSONReader). 
	 */
	wxJSONWriter writer;
	wxString responseString;
	writer.Write(responseData, responseString);

	//evt.SetClientData((void *) new wxString(responseString));
	//evt.
	evt.SetString(responseString);

	/* Todo: is this a critical section? */
	this->evtHandler->AddPendingEvent(evt);

	wxLogMessage("SeruroRequest> Thread finished...");

	return (ExitCode)0;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth / GetRequest */
	params["auth"] = GetAuth(name, params);
	/* GetRequest will expect auth to exist. */
	params["request"] = GetRequest(name, params);
	params["request"]["server"] = params["server"];

	wxString data_string;
	if (params["request"].HasMember("data")) {
		char *encoded = new char[256]; /* Todo: increate size of buffer. */
		wxString data_value;
		/* Convert optional data JSON into string format, for HTTP POST. */
		//wxJSONWriter writer;

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

wxJSONValue SeruroServerAPI::GetAuth(api_name_t name, wxJSONValue params)
{
	/* The authorization property will be scanned and should match the property
	 * layout of the request, namely "flags" and "data"
	 */
	wxJSONValue auth;
	wxJSONValue data;
	auth["data"] = data;
	auth["flags"] = SERURO_SECURITY_OPTIONS_NONE;
	
	if (name == SERURO_API_GET_P12) {
		/* Setup auth will be transported as "data" as extra headers or POST */
		auth["data"]["auth"] = wxT("teddy.reed\npassword");
		//auth["data"]["nonce"] = wxT("thisisanonce");
		//auth["data"]["date"] = wxT("today");
		/* Set flags indicating data. */
		auth["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	} else {
		auth["flags"] = SERURO_SECURITY_OPTIONS_CLIENT;
	}

	wxLogMessage(wxT("SeruroServerAPI::GetAuth> auth options: %d, data: %s."), auth["flags"].AsInt(), auth["data"].AsString());
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
		request["object"] = wxString(wxT("seruro/getCA"));
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
