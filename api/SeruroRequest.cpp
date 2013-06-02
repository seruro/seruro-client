
#include "SeruroRequest.h"
#include "Utils.h"

#include "../SeruroClient.h"

#include "../wxJSON/wx/jsonwriter.h"

DEFINE_EVENT_TYPE(SERURO_API_RESULT);

DECLARE_APP(SeruroClient);

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
	//writer.Write(response, event_string);
    
	/* The controller might be responsible for removing this memory, the wxWidgets API does not
	 * explicitly call out who owns this object.
	 */
	//wxCommandEvent evt(SERURO_API_RESULT, this->evtId);
	//evt.SetString(event_string);
    
    /* Create a request/response event with the response data. 
     * The ID determines which function receives the event. 
     */
    SeruroRequestEvent event(this->evtId);
    event.SetResponse(response);
    
	/* Todo: is this a critical section? */
	this->evtHandler->AddPendingEvent(event);
    
	wxLogMessage("SeruroRequest> Thread finished...");
    
	return (ExitCode)0;
}

wxString SeruroRequest::GetAuthToken()
{
	wxJSONValue response;
	wxJSONValue auth_params;
	wxString address;
	bool wrote_token;
    
	/* Perform TLS request to create API session, receive a raw content (string) response. */
	wxLogMessage(wxT("SeruroRequest::GetAuthToken> requesting token."));
    
	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	auth_params["object"] = wxT(SERURO_API_OBJECT_LOGIN);
	auth_params["verb"] = wxT("POST");
    
	/* If there was an explicit address set in the request parameters. */
	address = (params.HasMember("address")) ? params["address"].AsString() : wxString(wxEmptyString);
    
	/* Loop until they get a 'good' token, or cancel this madness. */
	int selected_address = 0;
	wxJSONValue data_string;
    wxString server_name;
    
	while (! response.HasMember("success") || response["success"].AsBool() == false) {
		server_name = this->params["server"]["name"].AsString();
		wxJSONValue data_string = getAuthFromPrompt(server_name,
            address, selected_address);
		if (! data_string.HasMember(SERURO_API_AUTH_FIELD_EMAIL)) {
			/* The user canceled the request for auth. */
			//return response;
			break;
		}
        
		selected_address = (data_string.HasMember("selected")) ? data_string["selected"].AsInt() : 0;
		data_string.Remove("selected");
        
		/* Todo: data_string potentially contains a user's password, ensure proper cleanup. */
		auth_params["data_string"] = encodeData(data_string);
		response = performRequest(auth_params);
	}
    
	/* Warning, possible critical section. */
    wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
    
    /* If "result" (boolean) is true, update token store for "email", set "token". */
	if (response.HasMember("error") || ! response["success"].AsBool() || ! response.HasMember("token")) {
		wxLogMessage(wxT("SeruroRequest::GetAuthToken> failed (%s)."), response["error"].AsString());
		goto finished;
	}
    
	wxLogMessage(wxT("SeruroRequest::GetAuthToken> received token (%s)."), response["token"].AsString());
    
	/* Warning: depends on response["email"] */
	wrote_token = wxGetApp().config->WriteToken(auth_params["server"]["name"].AsString(),
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
    wxString query_string;
    
	request_params["object"] = params["object"].AsString() +
    wxT("?") + wxT(SERURO_API_AUTH_TOKEN_PARAMETER) + wxT("=") +
    params["auth"]["token"].AsString();
    
    if (request_params.HasMember("query")) {
        query_string = encodeData(request_params["query"]);
        request_params["object"] = request_params["object"].AsString() + wxT("&") + query_string;
    }
    
	return performRequest(request_params);
}
