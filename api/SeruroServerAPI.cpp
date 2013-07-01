
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

bool DecodeBase64(const wxString &encoded, wxMemoryBuffer *decode)
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

wxJSONValue SeruroServerAPI::GetServer(const wxString &server)
{
	//wxJSONValue server_info;
	return wxGetApp().config->GetServer(server);
	//return server_info;
}

/* Create a SeruroRequest object for an API call 'name' using 'params' 
 * and generating an 'evtId' on response.
 *
 * params:
 * |--server = {name, host, [port]}, the name is used in an auth prompt (for a token).
 * |--[address], used to bind the request to a specific account/address (i.e., for P12s).
 * |--[password], if a token does not exist, do not use an auth prompt, instead this password.
 * |--[meta], a list of key/value pairs to pass back to the generated event (callback data).
 * The following additional params members are created corresponding to the API:
 * |--auth = {token, have_token}, a token if the an account exists for the server, a boolean.
 * |--request = {object, verb, [data], [query], auth, [address], [meta]}
 *    |--object: the object portion of the URI
 *    |--verb: the HTTP verb for the request.
 *    |--[data]: if the verb is POST, key/value pairs to post as data.
 *    |--[query]: query string key/value pairs.
 *    |--auth: a copy of params[auth].
 *    |--[address]: a copy of params[address].
 *    |--[meta]: a copy of params[meta].
 * |--request will generate "data_string" based on [data].
 *
 * The SeruroRequest will use params[request] as it's input, along with the parent object's 
 * event handler object and the provided 'evtId'.
 */
SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, int evtId)
{
	/* Add to params with GetAuth / GetRequest */
    wxString server_name, address;

	/* Create the request based on the API call. (Fills in object, verb, method, data, query.) */
	params["request"] = GetRequest(name, params);

	/* (Optionally) Pin this request to an address. */
	if (params.HasMember("address")) {
		address = params["address"].AsString();
		params["request"]["address"] = address;
	} else {
		address = wxEmptyString;
	}
	
	/* (Optionally) This request has a pinned password provided. */
	if (params.HasMember("password")) {
		params["request"]["password"] = params["password"];
	}

	/* Request auth */
	server_name = params["server"]["name"].AsString();
	params["request"]["auth"] = GetAuth(server_name, address);

	if (params["request"].HasMember("data")) {
		params["request"]["data_string"] = encodeData(params["request"]["data"]);
	}

	/* Copy callback meta-data into the request, which will end up in the response event. */
	if (params.HasMember("meta")) {
		params["request"]["meta"] = params["meta"];
	}

	/* Create the request thread after all data is filled in. */
	SeruroRequest *thread = new SeruroRequest(params["request"], evtHandler, evtId);

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI::CreateRequest> Could not create thread."));
	}

	/* Add to datastructure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}

wxJSONValue SeruroServerAPI::GetAuth(const wxString &server, const wxString &address)
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
	//wxJSONValue data;

	//wxJSONValue query;

	/* Most API calls to the server are currently GETs. */
	request["verb"] = _("GET");
	request["flags"] = SERURO_SECURITY_OPTIONS_NONE;
    
	/* (POST-DATA) Set multi-value (dict) "data" to a JSON Value. */
	request["data"] = wxJSONValue(wxJSONTYPE_OBJECT);
    /* (QUERY-STRING) Set multi-value (dict) "query" to a JSON value. */
    request["query"] = wxJSONValue(wxJSONTYPE_OBJECT);
    
	/* Allow address pinning, for authentication */
	//request["address"] = (params.HasMember("address")) ? params["address"] : wxEmptyString;

	/* Debug log. */
	//wxLogMessage(wxT("ServerAPI::GetRequest> Current auth token (%s)."), 
	//	params["auth"]["token"].AsString());

	/* Switch over each API call and set it's URL */
	switch (name) {
		/* The SETUP api call (/api/setup) should return an encrypted P12 (using password auth) */
	case SERURO_API_P12S:
		request["verb"] = _("POST");
		request["object"] = _(SERURO_API_OBJECT_P12S);
		/* Include support for optional explicit address to retreive from. */
		break;
	case SERURO_API_SEARCH:
		if (! params.HasMember("query")) {
			/* Return some error (not event, we are not in a thread yet) and stop. */
		}
        request["query"]["query"] = params["query"];
		request["object"] = _(SERURO_API_OBJECT_SEARCH);
		break;
	case SERURO_API_CERTS:
		if (! params.HasMember("request_address")) {
			/* Error and stop!. */
		}
        request["query"]["address"] = params["request_address"];
        request["object"] = _(SERURO_API_OBJECT_CERTS);
		break;
	case SERURO_API_CA:
		request["object"] = _(SERURO_API_OBJECT_CA);
		break;
	case SERURO_API_PING:
		request["object"] = _(SERURO_API_OBJECT_PING);
	}

	/* Todo: Check to make sure server is in the params. */
	request["server"] = params["server"];
	//request["auth"] = params["auth"];

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

	/* Set the CA fingerprint. */
	wxGetApp().config->SetCAFingerprint(response["server_name"].AsString(),
		cryptoHelper->GetFingerprint(ca_decoded));

	delete cryptoHelper;

	return result;
}

bool SeruroServerAPI::InstallCertificate(wxJSONValue response)
{
	if (! CheckResponse(response, "certs")) return false;
    //if (! CheckResponse(response, "address")) return false;

	wxString cert_encoded;
	wxMemoryBuffer cert_decoded;

	SeruroCrypto *cryptoHelper = new SeruroCrypto();

	bool result;
	//wxArrayString cert_blobs = response["certs"].GetMemberNames();
	for (int i = 0; i < response["certs"].Size(); i++) {
		cert_encoded = response["certs"][i].AsString();

		if (! DecodeBase64(cert_encoded, &cert_decoded)) continue;

		result = cryptoHelper->InstallCertificate(cert_decoded);

		/* Track this certificate. */
		wxGetApp().config->AddCertificate(response["server_name"].AsString(), 
			response["address"].AsString(), cryptoHelper->GetFingerprint(cert_decoded));
	}

	delete cryptoHelper;
	return result;
}

bool SeruroServerAPI::InstallP12(wxJSONValue response, wxString key)
{
	if (! CheckResponse(response, "p12")) return false;

	/* Get password from user for p12 containers. */
	wxString p12_key;

	if (key.compare(wxEmptyString) == 0) {
        /* If a key is not provided, then prompt the UI for one. */
		DecryptDialog *decrypt_dialog = new DecryptDialog(response["method"].AsString());
		if (decrypt_dialog->ShowModal() == wxID_OK) {
			p12_key = decrypt_dialog->GetValue();
		} else {
			return false;
		}
		/* Remove the modal from memory. */
		decrypt_dialog->Destroy();
	} else {
		p12_key = key;
	}

	/* Request details. */
	wxString server_name = response["server_name"].AsString();
	wxString address = response["address"].AsString();

	/* Install all P12 b64 blobs. */
	wxArrayString p12_blobs = response["p12"].GetMemberNames();
	wxString p12_encoded;
	wxMemoryBuffer p12_decoded;

	/* Extract fingerprints from the install. */
	wxArrayString fingerprints;

	bool result = true;
	SeruroCrypto crypto = SeruroCrypto();
	for (size_t i = 0; i < p12_blobs.size(); i++) {
		p12_encoded = response["p12"][p12_blobs[i]].AsString();

		if (! DecodeBase64(p12_encoded, &p12_decoded)) continue;
		/* Note: identity tracking happens within the crypto helper. */
		result = (result & crypto.InstallP12(p12_decoded, p12_key, fingerprints));
        if (! result) {
            wxLogMessage(_("SeruroServerAPI> (InstallP12) could not install (%s) p12."), p12_blobs[i]);
        } else {
            wxLogMessage(_("SeruroServerAPI> (InstallP12) installed (%s) p12."), p12_blobs[i]);
        }
	}

	/* Cleanup. */
	//delete cryptoHelper;
	p12_key.Empty();

	if (! result) return false;

	/* Remove previous fingerprints. */
	wxArrayString old_fingerprints = wxGetApp().config->GetIdentity(server_name, address); 
	for (size_t i = 0; i < old_fingerprints.size(); i++) {
		wxGetApp().config->RemoveIdentity(server_name, address, false);
	}

	/* Add the fingerprints. */
	for (size_t i = 0; i < fingerprints.size(); i++) {
		wxGetApp().config->AddIdentity(server_name, address, fingerprints[i]);
	}

	return true;
}

