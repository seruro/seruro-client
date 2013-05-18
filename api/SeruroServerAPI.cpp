
#include "SeruroServerAPI.h"
#include "Utils.h"

#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "SeruroRequest.h"
//#include "../crypto/SeruroCrypto.h"

//#include "../wxJSON/wx/jsonreader.h"
//#include "../wxJSON/wx/jsonwriter.h"

DECLARE_APP(SeruroClient);

//enum api_name api_name_t;
//enum seruro_api_callbacks seruro_api_callbacks_t;

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
		wxArrayString address_list = wxGetApp().config->GetAddressList(server);
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
