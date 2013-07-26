
#include "SeruroRequest.h"
#include "Utils.h"

#include "../SeruroClient.h"
#include "../frames/dialogs/AuthDialog.h"

#include "../wxJSON/wx/jsonwriter.h"

//DEFINE_EVENT_TYPE(SERURO_REQUEST_RESULT);
wxDEFINE_EVENT(SERURO_REQUEST_RESPONSE, SeruroRequestEvent);

DECLARE_APP(SeruroClient);

SeruroRequestEvent::SeruroRequestEvent(int id)
    : wxCommandEvent(SERURO_REQUEST_RESPONSE, id)
{
    response_data = wxJSONValue(wxJSONTYPE_OBJECT);
}

void SendFailureEvent(wxEvtHandler *handler, int event_id)
{
    wxJSONValue response;
    
    /* Simple failure response. */
    response["error"] = _(SERURO_API_ERROR_INVALID_AUTH);
    response["success"] = false;
    
    /* Create and send response. */
    SeruroRequestEvent event(event_id);
    event.SetResponse(response);
    //handler->AddPendingEvent(event);
    handler->AddPendingEvent(event);
}

void PerformRequestAuth(SeruroRequestEvent &event)
{
    wxJSONValue og_params, new_auth;
    
    AuthDialog *dialog;
    wxString address;
    int selected_address;
    
    /* Original request params are held in the event response. */
    og_params = event.GetResponse();
    
    address = wxEmptyString;
    if (og_params.HasMember("selected_address")) {
        /* The dialog was opened previously, use the same selected address. */
        selected_address = og_params["auth"]["selected_address"].AsInt();
    } else if (og_params["auth"].HasMember("address")) {
        /* If there is not a "previously" selected address, pin the address. */
        address = og_params["auth"]["address"].AsString();
    }
    
    /* Selected forces the user to use the provided address. Otherwise they may choose any. */
	dialog = new AuthDialog(og_params["server"]["uuid"].AsString(), address, selected_address);
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("SeruroRequest> (PerformRequestAuth) OK"));
		new_auth = dialog->GetValues();
	}
	/* Todo: password_control potentially contains a user's password, ensure proper cleanup. */
	delete dialog;
    
    /* The user cancled the auth prompt, reply with failure to the original handler. */
    if (! new_auth.HasMember(SERURO_API_AUTH_FIELD_EMAIL)) {
        SendFailureEvent(event.GetEventHandler(), event.GetEventID());
        return;
    }
    
    og_params["auth"]["address"] = new_auth["address"];
    og_params["auth"]["password"] = new_auth["password"];
    if (new_auth["selected_address"].AsInt() > 0) {
        /* Only set a selected address if it is not the first address, this accounts for pinned addresses. */
        og_params["auth"]["selected_address"] = new_auth["selected_address"];
    }
    
    /* Create and dispatch new request. */
    (new SeruroRequest(og_params, event.GetEventHandler(), event.GetEventID()))->Run();
}

SeruroRequest::SeruroRequest(wxJSONValue api_params, wxEvtHandler *parent, int parentEvtId)
	: wxThread(), params(api_params), evtHandler(parent), evtId(parentEvtId)
{
    wxLogMessage(_("SeruroRequest> creating thread for event id (%d)."), evtId);
	/* Catch-all for port configurations. */
    //if (params["server"])
	params["server"]["port"] = wxGetApp().config->GetPortFromServer(params["server"]);
}

SeruroRequest::~SeruroRequest()
{
    wxLogMessage(_("SeruroRequest> (0/1) deleting thread for event id (%d)."), evtId);
	/* Start client (all) threads accessor, to delete this, with a critial section. */
	wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);
    
	wxArrayThread& threads = wxGetApp().seruro_threads;
	threads.Remove(this);
    wxLogMessage(_("SeruroRequest> (1/1) deleted."), evtId);
    
	/* Todo: (check for an empty threads array) might want to signal to main thread if
	 * this was the last item, meaning it is OK to shutdown if it were waiting. */
	
}

void SeruroRequest::Reply(wxJSONValue response)
{
	/* Copy initialization callback data (called meta) into response. */
	if (params.HasMember("meta")) {
		response["meta"] = params["meta"];
	}

	/* Set the server uuid. */
    if (response.HasMember("uuid")) {
        response["server_uuid"] = response["uuid"];
    } else {
        wxLogMessage(_("SeruroRequest> (Reply) the server did not respond with a uuid?"));
    }

    /* Create a request/response event with the response data.
     * The ID determines which function receives the event. 
     */
    SeruroRequestEvent event(this->evtId);
    event.SetResponse(response);
    
    /* Event handler requires a critical section. */
    //wxCriticalSectionLocker locker(wxGetApp().seruro_critSection);
	this->evtHandler->AddPendingEvent(event);
}

void SeruroRequest::ReplyWithFailure()
{
    /* The failure event is generic. */
    SendFailureEvent(this->evtHandler, this->evtId);
}

/* Async-Requesting:
 * All API calls are over TLS, when the request thread begins, create a Crypto object.
 * When the TLS request returns, create a SERURO_REQUEST_RESPONSE event.
 * The TLS response will be either (1) a JSON value, or (2) error state from the connection.
 * This response is attached to the new event and the event is queued for the caller to catch.
 */
wxThread::ExitCode SeruroRequest::Entry()
{
    wxJSONValue response;
    wxString error_string;
    
	wxLogMessage("SeruroRequest> (Entry) Thread started...");
    
    /* Check for a token, if missing then check/request credentials. */
	//requested_token = false;
	wxLogMessage(wxT("SeruroRequest> (Entry) have token (%s), token (%s)."),
		params["auth"]["have_token"].AsString(), params["auth"]["token"].AsString());
    
    if (params.HasMember("not_api")) {
        /* Allow a caller to skip standard API checks. */
        response = performRequest(params);
        return (ExitCode)0;
    }
    
    /* If there is no have_token boolean set, we must create one. */
    if (! params["auth"]["have_token"].AsBool()) {
        if (! params["auth"].HasMember("address") || ! params["auth"].HasMember("password")) {
            wxLogMessage(_("SeruroRequest> (Entry) no auth and missing an address/password."));
            
            if (params["auth"]["no_prompt"].AsBool()) {
                this->ReplyWithFailure();
            } else {
                /* Request auth from UI. */
                this->RequestAuth();
            }
            return (ExitCode)0;
        }
        
        /* If we failed at receiving a token, but have credentails, we can attempt an auth. */
        if (! this->DoAuth()) {
            wxLogMessage(_("SeruroRequest> (Entry) auth failed."));
            if (params["auth"]["require_password"].AsBool() || params["no_prompt"].AsBool()) {
                /* There was an explicit password, do not request another. */
                this->ReplyWithFailure();
            } else {
                /* Request auth from UI. */
                this->RequestAuth();
            }
            return (ExitCode)0;
        }
    }
    
    /* Perform Request, receive JSON response. */
    response = this->DoRequest();
    error_string = (response.HasMember("error")) ? response["error"].AsString() : _("General Failure");
    
    /* Match error of "Invalid authentication token." if true, the auth failed. */
    if (error_string.compare(_(SERURO_API_ERROR_INVALID_AUTH)) == 0) {
        /* Request auth from UI. */
        this->RequestAuth();
	} else {
        wxLogMessage("SeruroRequest> (Entry) complete.");
        this->Reply(response);
    }
    
	return (ExitCode)0;
}

void SeruroRequest::RequestAuth()
{
    /* Create a request-special response event, only requests should make this event. */
    SeruroRequestEvent event(SERURO_REQUEST_CALLBACK_AUTH);
    
    event.SetResponse(params);
    event.SetOriginalEvent(this->evtHandler, this->evtId);
    
    /* Use the client event handler, because it's accessable and makes sense. */
    wxGetApp().AddEvent(event);
    //this->evtHandler->AddPendingEvent(event);
}

bool SeruroRequest::DoAuth()
{
    wxJSONValue response;
    wxJSONValue auth_params, data_string;
    //wxString encoded_data_string;
    bool wrote_token;
    
    wxLogMessage(_("SeruroRequest> (DoAuth) requesting token."));
    
	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	auth_params["object"] = _(SERURO_API_OBJECT_LOGIN);
	auth_params["verb"] = _("POST");
    
    /* There must be an account/password. */
    data_string[SERURO_API_AUTH_FIELD_EMAIL] = this->params["auth"]["address"];
    data_string[SERURO_API_AUTH_FIELD_PASSWORD] = this->params["auth"]["password"];
    //encodeData(data_string, encoded_data_string);
    
    /* Add encoded data string and clear potential passwords from memory. */
    auth_params["data_string"] = encodeData(data_string); // = encoded_data_string;
    //encoded_data_string.Clear();
    //data_string.Clear();
    //this->params["auth"].Clear();
    
    /* Perform auth request. */
    response = performRequest(auth_params);
    //auth_params.Clear();
    
	/* If "result" (boolean) is true, update token store for "email", set "token". */
	if (! response["success"].AsBool() || ! response.HasMember("token")) {
		wxLogMessage(wxT("SeruroRequest> (DoAuth) failed (%s)."), response["error"].AsString());
        return false;
	}
    
	wxLogMessage(_("SeruroRequest> (DoAuth) received token (%s)."), response["token"].AsString());
    
	/* Warning: depends on response["email"], response["uuid"] */
    wrote_token = false;
    if (response.HasMember("email") && response.HasMember("uuid")) {
        wrote_token = wxGetApp().config->WriteToken(response["uuid"].AsString(),
            response["email"].AsString(), response["token"].AsString());
        /* Setting the active token will fail if the account is being added. */
        wxGetApp().config->SetActiveToken(response["uuid"].AsString(), response["email"].AsString());
    } else {
        wxLogMessage(_("SeruroRequest (DoAuth) cannot find 'email' or 'uuid' in response?"));
    }
    
	if (! wrote_token) {
		wxLogMessage(_("SeruroRequest> (DoAuth) failed to write token."));
        return false;
	}
    
    this->params["auth"]["token"] = response["token"];
    this->params["auth"]["have_token"] = true;
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
        //query_string.Clear();
    }
    
	return performRequest(request_params);
}
