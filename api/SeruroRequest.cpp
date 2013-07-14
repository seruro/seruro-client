
#include "SeruroRequest.h"
#include "Utils.h"

#include "../SeruroClient.h"

#include "../wxJSON/wx/jsonwriter.h"

DEFINE_EVENT_TYPE(SERURO_API_RESULT);

DECLARE_APP(SeruroClient);

SeruroRequest::SeruroRequest(wxJSONValue api_params, wxEvtHandler *parent, int parentEvtId)
	: wxThread(), params(api_params), evtHandler(parent), evtId(parentEvtId)
{
    wxLogMessage(_("SeruroRequest> creating thread for event id (%d)."), evtId);
	/* Catch-all for port configurations. */
	params["server"]["port"] = wxGetApp().config->GetPortFromServer(params["server"]);
}

SeruroRequest::~SeruroRequest()
{
    wxLogMessage(_("SeruroRequest> deleting thrread for event id (%d)."), evtId);
	/* Start client (all) threads accessor, to delete this, with a critial section. */
	wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);
    
	wxArrayThread& threads = wxGetApp().seruro_threads;
	threads.Remove(this);
    
	/* Todo: (check for an empty threads array) might want to signal to main thread if
	 * this was the last item, meaning it is OK to shutdown if it were waiting. */
	
}

void SeruroRequest::Reply(wxJSONValue response)
{
	/* Copy initialization callback data (called meta) into response. */
	if (params.HasMember("meta")) {
		response["meta"] = params["meta"];
	}

	/* Set the server name. */
	response["server_name"] = params["server"]["name"];

    /* Create a request/response event with the response data. 
     * The ID determines which function receives the event. 
     */
    SeruroRequestEvent event(this->evtId);
    event.SetResponse(response);
    
    /* Event handler requires a critical section. */
    //wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);
	this->evtHandler->AddPendingEvent(event);
}

/* Async-Requesting:
 * All API calls are over TLS, when the request thread begins, create a Crypto object.
 * When the TLS request returns, create a SERURO_API_RESULT event.
 * The TLS response will be either (1) a JSON value, or (2) error state from the connection.
 * This response is attached to the new event and the event is queued for the caller to catch.
 */
wxThread::ExitCode SeruroRequest::Entry()
{
    wxJSONValue response;
    wxString error_string;
    
	wxLogMessage("SeruroRequest> (Entry) Thread started...");
    
    /* Check for params["auth"]["have_token"], if false perform GetAuthToken,. */
	requested_token = false;
	wxLogMessage(wxT("SeruroRequest> (Entry) have token (%s), token (%s)."),
		params["auth"]["have_token"].AsString(), params["auth"]["token"].AsString());
    
    /* If there is no have_token boolean set, we must create one. */
    if (! params["auth"].HasMember("have_token") || ! params["auth"]["have_token"].AsBool()) {
        if (! params["auth"].HasMember("address") || ! params["auth"].HasMember("password")) {
            wxLogMessage(_("SeruroRequest> (Entry) no auth and missing an address/password."));
            //stop and create missing_auth event.
        }
        
        /* If we failed at receiving a token. */
        if (! this->DoAuth()) {
            wxLogMessage(_("SeruroRequest> (Entry) auth failed."));
            if (params.HasMember("password")) {
                //there was an explicit password used, fail this request.
                //meaning reply with something helpful.
            } else {
                //stop and create missing_auth event.
            }
        }
    }
    
    /* Perform Request, receive JSON response. */
    response = this->DoRequest();
    error_string = (response.HasMember("error")) ? response["error"].AsString() : _("General Failure");
    
    /* Match error of "Invalid authentication token." if true, the auth failed. */
    if (error_string.compare(_(SERURO_API_ERROR_INVALID_AUTH)) == 0) {
        //stop and create missing_auth event.
	}
    
	wxLogMessage("SeruroRequest> Thread finished...");
	this->Reply(response);
	return (ExitCode)0;
}

bool SeruroRequest::DoAuth()
{
    wxJSONValue response;
    wxJSONValue auth_params, data_string;
    
    wxLogMessage(_("SeruroRequest> (DoAuth) requesting token."));
    
	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	auth_params["object"] = _(SERURO_API_OBJECT_LOGIN);
	auth_params["verb"] = _("POST");
    
    /* There must be an account/password. */
    data_string[SERURO_API_AUTH_FIELD_EMAIL] = address;
    data_string[SERURO_API_AUTH_FIELD_PASSWORD] = password;
    auth_params["data_string"] = encodeData(data_string);
    
    /* Perform auth request. */
    response = performRequest(auth_params);
    
	/* If "result" (boolean) is true, update token store for "email", set "token". */
	if (response.HasMember("error") || ! response["success"].AsBool() || ! response.HasMember("token")) {
		wxLogMessage(wxT("SeruroRequest> (Do Auth) failed (%s)."), response["error"].AsString());
        return false;
	}
    
	wxLogMessage(_("SeruroRequest> (Do Auth) received token (%s)."), response["token"].AsString());
    
	/* Warning: depends on response["email"] */
	wrote_token = wxGetApp().config->WriteToken(auth_params["server"]["name"].AsString(),
            response["email"].AsString(), response["token"].AsString());
	wrote_token = (wrote_token && wxGetApp().config->SetActiveToken(auth_params["server"]["name"].AsString(),
            response["email"].AsString()));
	if (! wrote_token) {
		wxLogMessage(_("SeruroRequest> (Do Auth) failed to write token."));
        return false;
	}
    
    return true;
}

wxJSONValue SeruroRequest::DoRequest()
{
	wxJSONValue request_params = this->params;
    wxString query_string;
    
	request_params["object"] = params["object"].AsString() +
        _("?") + _(SERURO_API_AUTH_TOKEN_PARAMETER) + _("=") + params["auth"]["token"].AsString();
    
    if (request_params.HasMember("query")) {
        query_string = encodeData(request_params["query"]);
        request_params["object"] = request_params["object"].AsString() + _("&") + query_string;
    }
    
	return performRequest(request_params);
}
