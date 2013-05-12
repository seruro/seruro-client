
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
        params["server"]["host"].AsString(), params["verb"].AsString(), params["object"].AsString());
    
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

wxJSONValue getAuthFromPrompt(wxString &server, const wxString &address = wxEmptyString, int selected = 0)
{
    /* Todo: Get all users (emails) for given server. */
    wxJSONValue auth;

	AuthDialog *dialog = new AuthDialog(server, address, selected);
	if (dialog->ShowModal() == wxID_OK) {
		wxLogMessage(wxT("SeruroServerAPI::getAuthFromPrompt> OK"));
		auth = dialog->GetValues();
	}
	/* Todo: password_control potentially contains a user's password, ensure proper cleanup. */
	delete dialog;

	return auth;
}

DecryptDialog::DecryptDialog(const wxString &method) :
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Decrypt Certificates")),
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizer* const sizer_top = new wxBoxSizer(wxVERTICAL);

	/* Todo: This should be switching on an enumeration. */
	wxString method_text;
	if (method.compare("sms") == 0) {
		method_text = wxT(TEXT_DECRYPT_METHOD_SMS);
	} else {
		method_text = wxT(TEXT_DECRYPT_METHOD_EMAIL);
	}

	/* Show a textual message. */
	wxStaticText *msg = new wxStaticText(this, wxID_ANY,
		wxString(method_text + wxT(" ") + wxT(TEXT_DECRYPT_EXPLAINATION)));
	msg->Wrap(300);
	sizer_top->Add(msg, wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT, 10));

	wxSizer* const sizer_info = new wxStaticBoxSizer(wxVERTICAL, this, "&Certificates Password");
	/* Password selection. */
	sizer_info->Add(new wxStaticText(this, wxID_ANY, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY, 
		wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxTE_PASSWORD);
	sizer_info->Add(password_control, wxSizerFlags().Expand().Border(wxBOTTOM));

	/* Default buttons. */
	sizer_top->Add(sizer_info, wxSizerFlags().Expand().Border(wxALL, 10));
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizer_top->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Right().Border());
	SetSizerAndFit(sizer_top);
}

wxString DecryptDialog::GetValue()
{
	return password_control->GetValue();
}

AuthDialog::AuthDialog(const wxString &server, const wxString &address, int selected) : 
	wxDialog(wxGetApp().GetFrame(), wxID_ANY, wxString(wxT("Seruro Server Login")),
	wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
	wxSizer* const sizerTop = new wxBoxSizer(wxVERTICAL);

	/* Show a textual message. */
	wxStaticText *msg = new wxStaticText(this, wxID_ANY, 
		wxString(wxT(TEXT_ACCOUNT_LOGIN) + server));
	msg->Wrap(300);
	sizerTop->Add(msg, wxSizerFlags().Expand().Border(wxTOP | wxLEFT | wxRIGHT, 10));

	wxSizer* const sizerInfo = new wxStaticBoxSizer(wxVERTICAL, this, "&Account Information");

	/* Email address selection */
	sizerInfo->Add(new wxStaticText(this, wxID_ANY, "&Email Address:"));
	/* Get available addresses: */
	//this->is_list = false;

	wxArrayString address_list;
	if (address.compare(wxEmptyString) == 0) {
		address_list = wxGetApp().config->GetAddresses(server);
		//this->is_list = address_list.size() > 1;
		if (address_list.size() == 0) {
			/* There is something very wrong here! */
		} //else if (! this->is_list) {
			//address_control = new wxTextCtrl(this, wxID_ANY);
			//address_list.Add(
		//} else {
			//address_list_control = new wxChoice(this, wxID_ANY, 
			//	wxDefaultPosition, wxDefaultSize, address_list);
			//address_list_control->SetSelection(0);
		//}
	} else {
		//address_control = new wxTextCtrl(this, wxID_ANY);
		address_list.Add(address);
	}


	address_control = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, address_list);
	address_control->SetSelection(selected);
	//if (this->is_list) {
	//	sizerInfo->Add(address_list_control, wxSizerFlags().Expand().Border(wxBOTTOM));
	//} else {
	sizerInfo->Add(address_control, wxSizerFlags().Expand().Border(wxBOTTOM));
	//}

	/* Password selection. */
	sizerInfo->Add(new wxStaticText(this, wxID_ANY, "&Password:"));
	password_control = new wxTextCtrl(this, wxID_ANY, 
		wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxTE_PASSWORD);
	//password_control->SetDefaultStyle(wxTextAttr(*wxTE_PASSWORD));
	sizerInfo->Add(password_control, wxSizerFlags().Expand().Border(wxBOTTOM));

	/* Default buttons. */
	sizerTop->Add(sizerInfo, wxSizerFlags().Expand().Border(wxALL, 10));
	/* Note, the standard buttons allow us to use this dialog as a modal. Do not change
	 * the button selections or the modal will no longer respond.
	 */
	sizerTop->Add(CreateStdDialogButtonSizer(wxOK | wxCANCEL), wxSizerFlags().Right().Border());
	SetSizerAndFit(sizerTop);
}

wxJSONValue AuthDialog::GetValues()
{
	wxJSONValue values;

	//if (this->is_list) {
	int selected = this->address_control->GetSelection();
	/* Save the selected index for a better user experience. */
	values["selected"] = selected;
	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetString(selected);
	//} else {
	//	values[SERURO_API_AUTH_FIELD_EMAIL] = this->address_control->GetValue();
	//}
	values[SERURO_API_AUTH_FIELD_PASSWORD] = this->password_control->GetValue();

	return values;
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
	wxString address;
	bool wrote_token;

	/* Perform TLS request to create API session, receive a raw content (string) response. */
	wxLogMessage(wxT("SeruroRequest::GetAuthToken> requesting token."));

	auth_params["flags"] = SERURO_SECURITY_OPTIONS_DATA;
	auth_params["server"] = this->params["server"];
	auth_params["object"] = wxT(SERURO_API_OBJECT_SESSION_CREATE);
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

wxJSONValue SeruroServerAPI::GetServer(wxString &server)
{
	//wxJSONValue server_info;
	return wxGetApp().config->GetServer(server);
	//return server_info;
}

SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth / GetRequest */
    wxString server = params["server"]["name"].AsString();
	wxString address = (params.HasMember("address")) ? params["address"].AsString() : wxString(wxEmptyString);
	params["auth"] = GetAuth(server, address);

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

wxJSONValue SeruroServerAPI::GetAuth(wxString &server, const wxString &address)
{
	wxJSONValue auth;

	/* Determine address to request token for (from given server).
	 * If an address is not given, search for ANY token for this server. 
	 */
	if (address.compare(wxEmptyString) == 0) {
		wxString token;
		wxArrayString address_list = wxGetApp().config->GetAddresses(server);
		for (size_t i = 0; i < address_list.size(); i++) {
			token = wxGetApp().config->GetToken(server, address_list[i]);
			if (token.compare(wxEmptyString) != 0) {
				break;
			}
		}
		auth["token"] = token;
	} else {
		auth["token"] = wxGetApp().config->GetToken(server, address);
	}

	auth["have_token"] = (auth["token"].AsString().size() > 0) ? true : false;

	if (! auth["have_token"].AsBool()) {
		wxLogMessage(wxT("SeruroServerAPI::GetAuth> failed to find valid auth token."));
	}

	return auth;
}

wxJSONValue SeruroServerAPI::GetRequest(api_name_t name, wxJSONValue params)
{
	wxJSONValue request;
	wxJSONValue data;

	wxJSONValue query;
    //wxString query_string;

	/* All calls to the server are currently POSTs. */
	request["verb"] = wxT("POST");
	request["flags"] = SERURO_SECURITY_OPTIONS_NONE;
    
	/* (POST-DATA) Set multi-value (dict) "data" to a JSON Value. */
	request["data"] = data;
    /* (QUERY-STRING) Set multi-value (dict) "query" to a JSON value. */
    request["query"] = query;
    
	/* Allow address pinning, for authentication */
	request["address"] = (params.HasMember("address")) ? params["address"] : wxEmptyString;

	/* Debug log. */
	wxLogMessage(wxT("SeruroServerAPI::GetRequest> Current auth token (%s)."), 
		params["auth"]["token"].AsString());

	/* Switch over each API call and set it's URL */
	switch (name) {
		/* The SETUP api call (/api/setup) should return an encrypted P12 (using password auth) */
	case SERURO_API_GET_P12:
		request["object"] = wxT("getP12s");
		/* Include support for optional explicit address to retreive from. */
		//if (params.HasMember("address")) {
		//	request["data"]["address"] = params["address"];
		//}
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember(wxT("query"))) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
        request["verb"] = wxT("GET");
        request["object"] = wxString(wxT("findCerts"));// + params["query"].AsString();
        request["query"]["query"] = params["query"];
		//request["verb"] = wxT("GET");
		/* Create query string for query parameter. */
		//wxJSONValue query;
		//querys["query"] = params["query"];
		//query_string = encodeData(querys);
		//request["object"] = request["object"].AsString() + query_string;

		break;
	case SERURO_API_GET_CERT:
		if (! params.HasMember(wxT("request_address"))) {
			/* Error and stop!. */
		}
		request["object"] = wxString(wxT("cert/")) + params["request_address"].AsString();
		break;
	case SERURO_API_GET_CA:
		request["verb"] = wxT("POST");
		request["object"] = wxT("getCA");
		break;
	}

	/* Check to make sure server is in the params. */
	request["server"] = params["server"]; //.AsString()
	request["auth"] = params["auth"];

	/* Add prefix of "/api/". */
	request["object"] = wxString(wxT("/api/seruro/")) + request["object"].AsString();
	return request;
}
