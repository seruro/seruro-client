
#include "SeruroRequest.h"
#include "../logging/SeruroLogger.h"
#include "Utils.h"

#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "../frames/dialogs/AuthDialog.h"

#include "../wxJSON/wx/jsonwriter.h"

#define TEXT_AUTH_CANCEL "Login request canceled."

//DEFINE_EVENT_TYPE(SERURO_REQUEST_RESULT);
wxDEFINE_EVENT(SERURO_REQUEST_RESPONSE, SeruroRequestEvent);

DECLARE_APP(SeruroClient);

void SendFailureEvent(wxEvtHandler *handler, int event_id,
    const wxString &server_uuid, size_t request_id, const wxString &error_msg = wxEmptyString);

SeruroRequestEvent::SeruroRequestEvent(int id)
    : wxCommandEvent(SERURO_REQUEST_RESPONSE, id)
{
    response_data = wxJSONValue(wxJSONTYPE_OBJECT);
	seruro_handler = NULL;
	seruro_event_id = id;
}

SeruroRequestEvent::SeruroRequestEvent(const SeruroRequestEvent &event)
	: wxCommandEvent(event)
{
	this->SetResponse(event.GetResponse());
	this->SetOriginalEvent(event.GetEventHandler(), event.GetEventID());
}

void SendFailureEvent(wxEvtHandler *handler, int event_id,
    const wxString &server_uuid, size_t request_id, const wxString &error_msg)
{
    wxJSONValue response;
    
    /* Simple failure response. */
    response["error"] = error_msg;
    response["success"] = false;
    
    /* Add meta-data. */
    response["server_uuid"] = server_uuid;
    response["request_id"] = (unsigned int) request_id;
    
    /* Create and send response. */
    SeruroRequestEvent event(event_id);
    event.SetResponse(response);

	/* If the original event was created without a handler, let the SeruroClient handle. */
	if (handler == NULL || ! handler) {
		wxGetApp().AddEvent(event);
	} else {
		handler->AddPendingEvent(event);
	}
}

void PerformRequestAuth(SeruroRequestEvent &event)
{
    wxJSONValue og_params, new_auth;
    
    AuthDialog *dialog;
    wxString address, server_uuid;
	
	size_t request_id;
	/* Auto-select an address from the list (displayed). */
	int selected_address;

    /* Original request params are held in the event response. */
	request_id = 0;
	selected_address = -1;
    og_params = event.GetResponse();

	/* Grab original event handler/id from the request event. */
	wxEvtHandler *event_handler = event.GetEventHandler();
	int event_id = event.GetEventID();

    address = wxEmptyString;
	server_uuid = og_params["server"]["uuid"].AsString();

    if (og_params.HasMember("selected_address")) {
        /* The dialog was opened previously, use the same selected address. */
        selected_address = og_params["auth"]["selected_address"].AsInt();
    } else if (og_params["auth"].HasMember("address")) {
        /* If there is not a "previously" selected address, pin the address. */
        address = og_params["auth"]["address"].AsString();
    }
    
    /* Selected forces the user to use the provided address. Otherwise they may choose any. */
	dialog = new AuthDialog(server_uuid, address, selected_address);
	if (dialog->ShowModal() == wxID_OK) {
		DEBUG_LOG(_("SeruroRequest> (PerformRequestAuth) OK"));
		new_auth = dialog->GetValues();
	}
	/* password_control potentially contains a user's password, ensure proper cleanup. */
	delete dialog;
    
    /* The user cancled the auth prompt, reply with failure to the original handler. */
    if (! new_auth.HasMember(SERURO_API_AUTH_FIELD_EMAIL)) {
		if (og_params["request_id"].GetType() == wxJSONTYPE_UINT) { 
			/* Optionally, set the request_id of the failed event, so correct handler may respond. */
			request_id = og_params["request_id"].AsUInt(); 
		}
        SendFailureEvent(event_handler, event_id, server_uuid, request_id, _(TEXT_AUTH_CANCEL));
        return;
    }
    
	/* Copy auth params from dialog to the original auth event (request) params. */
    og_params["auth"]["address"] = new_auth[SERURO_API_AUTH_FIELD_EMAIL];
    og_params["auth"]["password"] = new_auth[SERURO_API_AUTH_FIELD_PASSWORD];
    if (new_auth["selected_address"].AsInt() > 0) {
        /* Only set a selected address if it is not the first address, this applies to unpinned addresses. */
        og_params["auth"]["selected_address"] = new_auth["selected_address"];
    }
    
	/* Remove the existing token for this user. */
	theSeruroConfig::Get().WriteToken(server_uuid, address, wxEmptyString);
	/* Remove the token from the original auth parameters, allowing the new request to auth. */
	og_params["auth"].Remove("token");
	og_params["auth"]["have_token"] = false;

    /* Create and dispatch new request. */
    (new SeruroRequest(og_params, event_handler, event_id))->Run();
}

size_t SeruroRequest::current_request_id = 0;

SeruroRequest::SeruroRequest(wxJSONValue api_params, wxEvtHandler *parent, int parentEvtId)
	: wxThread(wxTHREAD_DETACHED), params(api_params)
{
	/* Catch-all for port configurations. */
	params["server"]["port"] = theSeruroConfig::Get().GetPortFromServer(params["server"]);

    /* Generate an instance-unique request ID */
    this->request_id = ++SeruroRequest::current_request_id;
	this->evtHandler = parent;
	this->evtId = parentEvtId;
    
	/* Add to data structure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critsection_thread);
	wxGetApp().seruro_threads.Add(this);

	DEBUG_LOG(_("SeruroRequest> creating thread for event id (%d)."), evtId);
}

SeruroRequest::~SeruroRequest()
{
    DEBUG_LOG(_("SeruroRequest> (0/1) deleting thread for event id (%d)."), evtId);
	/* Start client (all) threads accessor, to delete this, with a critial section. */
	wxCriticalSectionLocker locker(wxGetApp().seruro_critsection_thread);
    
	wxArrayThread& threads = wxGetApp().seruro_threads;
	threads.Remove(this);
    DEBUG_LOG(_("SeruroRequest> (1/1) deleted."), evtId);
    
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
    response["request_id"] = (unsigned int) this->request_id;
    if (response.HasMember("uuid")) {
        response["server_uuid"] = response["uuid"];
    } else {
        DEBUG_LOG(_("SeruroRequest> (Reply) the server did not respond with a uuid?"));
    }

    /* Create a request/response event with the response data.
     * The ID determines which function receives the event. 
     */
    SeruroRequestEvent event(this->evtId);
    event.SetResponse(response);
    
    /* Event handler requires a critical section. */
    if (this->evtHandler != NULL) {
        this->evtHandler->AddPendingEvent(event);
    } else {
		wxGetApp().AddEvent(event);
	}
}

void SeruroRequest::ReplyWithFailure(const wxString &error_msg)
{
    /* The failure event is generic. */
    SendFailureEvent(this->evtHandler, this->evtId,
        params["server"]["uuid"].AsString(), this->request_id, error_msg);
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
    
	DEBUG_LOG("SeruroRequest> (Entry) Thread started...");
    
    /* Check for a token, if missing then check/request credentials. */
	DEBUG_LOG(wxT("SeruroRequest> (Entry) have token (%s), token (%s)."),
		params["auth"]["have_token"].AsString(), params["auth"]["token"].AsString());
    
    if (params.HasMember("not_api")) {
        /* Allow a caller to skip standard API checks. */
        response = performRequest(params);
        return (ExitCode)0;
    }
    
    /* If there is no have_token boolean set, we must create one. */
    error_string = wxEmptyString;
    if (! params["auth"]["have_token"].AsBool()) {
        if (! params["auth"].HasMember("address") || ! params["auth"].HasMember("password")) {
            DEBUG_LOG(_("SeruroRequest> (Entry) no auth and missing an address/password."));
            
            if (params["auth"]["no_prompt"].AsBool()) {
                this->ReplyWithFailure(_("Cannot perform authentication."));
            } else {
                /* Request auth from UI. */
                this->RequestAuth();
            }
            return (ExitCode)0;
        }
        
        /* If we failed at receiving a token, but have credentails, we can attempt an auth. */
        if (! this->DoAuth(error_string)) {
            //LOG(wxT("2. %s"), error_string);
            DEBUG_LOG(_("SeruroRequest> (Entry) auth failed."));
            if (params["auth"]["require_password"].AsBool() || params["no_prompt"].AsBool()) {
                /* There was an explicit password, do not request another. */
                this->ReplyWithFailure(error_string);
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
        DEBUG_LOG("SeruroRequest> (Entry) complete.");
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
}

bool SeruroRequest::DoAuth(wxString &error_string)
{
    wxJSONValue response;
    wxJSONValue auth_params, data_string;
    //wxString encoded_data_string;
    bool wrote_token;
    
    DEBUG_LOG(_("SeruroRequest> (DoAuth) requesting token."));
    
	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	auth_params["object"] = _(SERURO_API_OBJECT_LOGIN);
	auth_params["verb"] = _("POST");
    
    /* There must be an account/password. */
    data_string[SERURO_API_AUTH_FIELD_EMAIL] = this->params["auth"]["address"];
    data_string[SERURO_API_AUTH_FIELD_PASSWORD] = this->params["auth"]["password"];
    //encodeData(data_string, encoded_data_string);
    
    /* Add encoded data string and clear potential passwords from memory. */
    auth_params["data_string"] = encodeData(data_string);
    
    /* Perform auth request. */
    response = performRequest(auth_params);
    
    if (response.HasMember("error")) {
        /* Optionally place the error into the output parameter. */
        error_string.Append(response["error"].AsString());
    }
    
	/* If "result" (boolean) is true, update token store for "email", set "token". */
	if (! response["success"].AsBool() || ! response.HasMember("token")) {
		DEBUG_LOG(_("SeruroRequest> (DoAuth) failed (%s)."), response["error"].AsString());
        return false;
	}
    
	DEBUG_LOG(_("SeruroRequest> (DoAuth) received token (%s)."), response["token"].AsString());
    
	/* Warning: depends on response["email"], response["uuid"] */
    wrote_token = false;
    if (response.HasMember("email") && response.HasMember("uuid")) {
        wrote_token = theSeruroConfig::Get().WriteToken(response["uuid"].AsString(),
            response["email"].AsString(), response["token"].AsString());
        /* Setting the active token will fail if the account is being added. */
        theSeruroConfig::Get().SetActiveToken(response["uuid"].AsString(), response["email"].AsString());
    } else {
        DEBUG_LOG(_("SeruroRequest (DoAuth) cannot find 'email' or 'uuid' in response?"));
    }
    
	if (! wrote_token) {
		DEBUG_LOG(_("SeruroRequest> (DoAuth) failed to write token."));
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
    
    request_params["object"] = wxString::Format(_("%s?%s=%s"), params["object"].AsString(),
        _(SERURO_API_AUTH_TOKEN_PARAMETER), params["auth"]["token"].AsString());
	//request_params["object"] = params["object"].AsString() +
    //    _("?") + _(SERURO_API_AUTH_TOKEN_PARAMETER) + _("=") + params["auth"]["token"].AsString();
    
    if (request_params.HasMember("query")) {
        query_string = encodeData(request_params["query"]);
        request_params["object"] = wxString::Format(_("%s&%s"), request_params["object"].AsString(), query_string);
        //request_params["object"] = request_params["object"].AsString() + _("&") + query_string;
    }
    
	return performRequest(request_params);
}
