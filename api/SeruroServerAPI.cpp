
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
        wxLogStatus(wxT("SeruroRequest (parse)> could not parse response data as JSON."));
    }
	if (! response.HasMember("success")) {
		wxLogMessage(wxT("SeruroRequest (parse)> response does not contain a 'success' key."));
		/* Fill in JSON data with TLS/connection error. */
		response["success"] = false;
        if (! response.HasMember("error")) {
            response["error"] = wxT("Connection failed.");
        }
	}
    
    /* Error may be a string, dict, or list? */
    if (! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroRequest (parse)> failed: (%s)."), response["error"].AsString());
	}
    
    /* Todo: consider checking for error string that indicated invalid token. */
    
    return response;
}

wxJSONValue performRequest(wxJSONValue params)
{
    wxJSONValue response;
    wxString raw_response;
    
	/* Get a crypto helper object for TLS requests. */
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
    
	/* Show debug statement, list the request. */
	wxLogMessage(wxT("SeruroRequest (request)> Server (%s), Verb (%s), Object (%s)."),
        params["server"].AsString(), params["verb"].AsString(), params["object"].AsString());
    
	/* Perform the request, receive a raw content (string) response. */
	raw_response = cryptoHelper->TLSRequest(params);
    
	delete [] cryptoHelper;
    
    /* Try to parse response as JSON (on failure, bail, meaning fill in JSON error ourselves). */
    response = parseResponse(raw_response);
    
    return response;
}

wxString encodeData(wxJSONValue data) 
{
	wxString data_string;
	wxArrayString data_names;
	
	wxString data_value;
	char *encoded_value;

	encoded_value = (char *) malloc(256 * sizeof(char)); /* Todo: increate size of buffer. */
	
	/* Construct a URL query string from JSON. */
	data_names = data.GetMemberNames();
	for (size_t i = 0; i < data_names.size(); i++) {
		if (i > 0) data_string = data_string + wxString(wxT("&"));
		/* Value must be URLEncoded. */
		data_value = data[data_names[i]].AsString();
		URLEncode(encoded_value, data_value.mbc_str(), 256);

		data_string = data_string + data_names[i] + wxString(wxT("=")) + wxString(encoded_value);
	}
	delete encoded_value;

	wxLogMessage(wxT("SeruroRequest (encode)> Encoded: (%s)."), data_string);

	return data_string;
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
	bool requested_token;
    
	wxLogMessage("SeruroRequest> Thread started...");

    /* Check for params["auth"]["have_token"], if false perform GetAuthToken,. */
	requested_token = false;
	wxLogMessage(wxT("SeruroRequest::Entry> have token (%s), token (%s)."), 
		params["auth"]["have_token"].AsString(), params["auth"]["token"].AsString());

	if (! params["auth"].HasMember("have_token") || ! params["auth"]["have_token"].AsBool()) {
		wxLogMessage(wxT("SeruroRequest::Entry> no auth token present."));
		params["auth"]["token"] = this->GetAuthToken();
		requested_token = true;
	}
    
    /* Perform Request, receive JSON response. */
    response = this->DoRequest();
       
    /* Match error of "Invalid authentication token." if true, perform GetAuthToken. */
    if (::wxStricmp(response["error"].AsString(), wxT(SERURO_API_ERROR_INVALID_AUTH)) == 0) {
		/* If true, perform request again (as token may have been outdated), receive JSON response. */
		if (! requested_token) {
			wxLogMessage(wxT("SeruroRequest::Entry> invalid auth token detected (retrying)."));
			/* If we requested the token during this "request" then bubble error to caller. */
			params["auth"]["token"] = this->GetAuthToken();
			response = this->DoRequest();
		}
	}

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
	wxJSONValue response;
	wxJSONValue auth_params;
	bool wrote_token;

	wxLogMessage(wxT("SeruroRequest::GetAuthToken> requesting token."));

	/* Get password from user for p12 containers. */
	wxString p12_key;
	wxPasswordEntryDialog *get_key = new wxPasswordEntryDialog(wxGetApp().GetFrame(), wxT("Enter decryption key"));
	if (get_key->ShowModal() == wxID_OK) {
		p12_key = get_key->GetValue();
	} else {
		return wxString("");
	}
	/* Remove the modal from memory. */
	get_key->Destroy();

	/* Perform TLS request to create API session, receive a raw content (string) response. */
	auth_params["data_string"] = encodeData(this->params["auth"]["data"]);

	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	//auth_params["port"] = wxT("443");
	auth_params["object"] = wxT(SERURO_API_OBJECT_SESSION_CREATE);
	auth_params["verb"] = wxT("POST");

	response = performRequest(auth_params);
    
	/* Warning, possible critical section. */
    wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);

    /* If "result" (boolean) is true, update token store for "email", set "token". */
	if (response.HasMember("error") || ! response["success"].AsBool() || ! response.HasMember("token")) {
		wxLogMessage(wxT("SeruroRequest::GetAuthToken> failed (%s)."), response["error"].AsString());
		goto finished;
	}

	wxLogMessage(wxT("SeruroRequest::GetAuthToken> received token (%s)."), response["token"].AsString());
    
	/* Warning: depends on response["email"] */
	wrote_token = wxGetApp().config->WriteToken(auth_params["server"].AsString(), 
		response["email"].AsString(), response["token"].AsString());
	if (! wrote_token) {
		wxLogMessage(wxT("SeruroRequest::GetAuthToken> failed to write token."));
	}

finished:
    /* Respond with "token" as string. */
    return response["token"].AsString();
}

wxJSONValue SeruroRequest::DoRequest()
{
	wxJSONValue request_params = this->params;
	request_params["object"] = params["object"].AsString() + wxT("?token=") + 
		params["auth"]["token"].AsString();
	return performRequest(request_params);
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth / GetRequest */
	params["auth"] = GetAuth(params["server"].AsString());

	/* GetRequest will expect auth to exist. */
	params["request"] = GetRequest(name, params);

	if (params["request"].HasMember("data")) {
		params["request"]["data_string"] = encodeData(params["request"]["data"]);
	}

	SeruroRequest *thread = new SeruroRequest(params["request"], evtHandler, evtId);

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI::CreateRequest> Could not create thread."));
	}

	/* Add to datastructure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}

wxJSONValue SeruroServerAPI::GetAuth(wxString &server)
{
	/* return {"data": {}, "have_token": bool, "token": ""} */

	wxJSONValue auth;
	wxJSONValue data;
	wxString address;
	//auth["flags"] = SERURO_SECURITY_OPTIONS_NONE;

	auth["data"] = data;

	/* Determine address to request token for (from given server). */
	/* Todo: prompt to get address, perhaps from a list (might as well get password too). */
	address = wxT("ted@valdrea.com");

	/* Try to find auth token. */
	auth["token"] = wxGetApp().config->GetToken(server, address);

	auth["have_token"] = (auth["token"].AsString().size() > 0) ? true : false;

	if (! auth["have_token"].AsBool()) {
		wxLogMessage(wxT("SeruroServerAPI::GetAuth> failed to find valid auth token."));
		/* Failed to find a valid token. */

		//auth["data"]["user[email]"] = wxT("ted@valdrea.com");
		auth["data"][SERURO_API_AUTH_FIELD_EMAIL] = address;
		auth["data"][SERURO_API_AUTH_FIELD_PASSWORD] = wxT("password");
		/* Todo: prompt for password (but provide override method, like input to ::GetRequest). */
	}

	return auth;
}

wxJSONValue SeruroServerAPI::GetRequest(api_name_t name, wxJSONValue params)
{
	wxJSONValue request;
	wxJSONValue data;

	/* All calls to the server are currently GETs. */
	request["verb"] = wxT("POST");
	request["flags"] = SERURO_SECURITY_OPTIONS_NONE;
	/* Set multi-value (dict) "data" to a JSON Value/ */
	request["data"] = data; 

	/* Debug log. */
	wxLogMessage(wxT("SeruroServerAPI::GetRequest> Current auth token (%s)."), 
		params["auth"]["token"].AsString());

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
		request["object"] = wxT("getCA");
		break;
	}

	/* Check to make sure server is in the params. */
	request["server"] = params["server"]; //.AsString()
	request["auth"] = params["auth"];

	/* Add optional auth data to request data. */
	//if (params["auth"].HasMember("data")) {
		//request["auth"]
		//request["data"].Append(params["auth"]["data"]); 
		//params["data"] = params["data"].AsMap().insert(
		//	params["auth"]["data"].AsMap().begin(), params["auth"]["data"].AsMap().end());
	//}

	/* Add auth flags to request data. */
	//request["flags"] = request["flags"].AsInt() | params["auth"]["flags"].AsInt();

	/* Add prefix of "/api/". */
	request["object"] = wxString(wxT("/api/seruro/")) + request["object"].AsString();
	return request;
}
