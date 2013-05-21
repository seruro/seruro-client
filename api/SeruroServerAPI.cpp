
#include "SeruroServerAPI.h"
#include "Utils.h"

#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "SeruroRequest.h"
#include "../frames/dialogs/DecryptDialog.h"
#include "../crypto/SeruroCrypto.h"

#include <wx/base64.h>
#include <wx/buffer.h>

DECLARE_APP(SeruroClient);

bool CheckResponse(wxJSONValue response, wxString required_key)
{
	if (! response.HasMember("success") || ! response["success"].AsBool()) {
		wxLogMessage(wxT("SeruroServerAPI> Bad Result."));
		return false;
	}

	/* The getP12 API call should respond with up to 3 P12s. */
	if (! response.HasMember(required_key)) {
		wxLogMessage(wxT("SeruroServerAPI> Response does not include '%s' data."), required_key);
		return false;
	}

	return true;
}

bool DecodeBase64(wxString &encoded, wxMemoryBuffer *decode)
{
	//wxString ca_encoded;
	size_t decode_error;
	//wxMemoryBuffer internal_decoded;

	//ca_encoded = response["ca"].AsString();
	decode_error = 0;
	(*decode) = wxBase64Decode(encoded, wxBase64DecodeMode_Relaxed, &decode_error);

	if (decode_error != 0) {
		wxLogMessage(wxT("ServerAPI> (Decode) Could not decode position %d in ca blob '%s'."), 
				decode_error, encoded);
		return false;
	}

	return true;
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
	wxLogMessage(wxT("ServerAPI::GetRequest> Current auth token (%s)."), 
		params["auth"]["token"].AsString());

	/* Switch over each API call and set it's URL */
	switch (name) {
		/* The SETUP api call (/api/setup) should return an encrypted P12 (using password auth) */
	case SERURO_API_GET_P12:
		request["object"] = wxT("getP12s");
		/* Include support for optional explicit address to retreive from. */
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember(wxT("query"))) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
        request["verb"] = wxT("GET");
        request["object"] = wxString(wxT("findCerts"));// + params["query"].AsString();
        request["query"]["query"] = params["query"];
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

bool SeruroServerAPI::InstallCA(wxJSONValue response)
{
	if (! CheckResponse(response, "ca")) return false;

	wxString ca_encoded;
	wxMemoryBuffer ca_decoded;

	ca_encoded = response["ca"].AsString();
	if (! DecodeBase64(ca_encoded, &ca_decoded)) return false;

	bool result;
	SeruroCrypto *cryptoHelper = new SeruroCrypto();
	result = cryptoHelper->InstallCA(ca_decoded);
	delete cryptoHelper;

	return result;
}

bool SeruroServerAPI::InstallCert(wxJSONValue response)
{
	if (! CheckResponse(response, "certs")) return false;

	wxString cert_encoded;
	wxMemoryBuffer cert_decoded;

	SeruroCrypto *cryptoHelper = new SeruroCrypto();

	bool result;
	wxArrayString cert_blobs = response["certs"].GetMemberNames();
	for (size_t i = 0; i < cert_blobs.size(); i++) {
		cert_encoded = response["certs"][cert_blobs[i]].AsString();

		if (! DecodeBase64(cert_encoded, &cert_decoded)) continue;

		result = cryptoHelper->InstallCert(cert_decoded);
	}

	delete cryptoHelper;
	return result;
}

bool SeruroServerAPI::InstallP12(wxJSONValue response)
{
	if (! CheckResponse(response, "p12")) return false;

	/* Get password from user for p12 containers. */
	wxString p12_key;
	DecryptDialog *decrypt_dialog = new DecryptDialog(response["method"].AsString());
	if (decrypt_dialog->ShowModal() == wxID_OK) {
		p12_key = decrypt_dialog->GetValue();
	} else {
		return false;
	}
	/* Remove the modal from memory. */
	decrypt_dialog->Destroy();

	SeruroCrypto *cryptoHelper = new SeruroCrypto();

	/* Install all P12 b64 blobs. */
	wxArrayString p12_blobs = response["p12"].GetMemberNames();
	wxString p12_encoded;
	wxMemoryBuffer p12_decoded;

	bool result;
	for (size_t i = 0; i < p12_blobs.size(); i++) {
		p12_encoded = response["p12"][p12_blobs[i]].AsString();

		if (! DecodeBase64(p12_encoded, &p12_decoded)) continue;

		result = cryptoHelper->InstallP12(p12_decoded, p12_key);
	}

	/* Cleanup. */
	delete cryptoHelper;
	p12_key.Empty();

	return result;
}

