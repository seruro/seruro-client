
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

wxJSONValue SeruroServerAPI::GetServer(const wxString &server_uuid)
{
	//wxJSONValue server_info;
	return wxGetApp().config->GetServer(server_uuid);
	//return server_info;
}

/* Create a SeruroRequest object for an API call 'name' using 'params' 
 * and generating an 'evtId' on response.
 *
 * params:
 * |--server = {[uuid], host, [port]}, the name is used in an auth prompt (for a token).
 * |--[address], used to bind the request to a specific account/address (i.e., for P12s).
 * |--[password], if a token does not exist, do not use an auth prompt, instead this password.
 * |--[require_password], if the provided password should not be re-entered/changed.
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
	/* Create the request based on the API call. (Fills in object, verb, method, data, query.) */
	params["request"] = this->GetRequest(name, params);

    /* Create the auth structure for the request. */
    params["request"]["auth"] = this->GetAuth(params);

	if (params["request"].HasMember("data")) {
		params["request"]["data_string"] = encodeData(params["request"]["data"]);
        
        /* Clean up potential password. */
        //params["request"]["data"].Clear();
	}

	/* Copy callback meta-data into the request, which will end up in the response event. */
	if (params.HasMember("meta")) {
		params["request"]["meta"] = params["meta"];
	}

	/* Create the request thread after all data is filled in. */
	SeruroRequest *thread = new SeruroRequest(params["request"], evtHandler, evtId);
    //params.Clear();

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		wxLogError(wxT("SeruroServerAPI::CreateRequest> Could not create thread."));
	}

	/* Add to data structure accessed by thread */
	wxCriticalSectionLocker enter(wxGetApp().seruro_critSection);
	wxGetApp().seruro_threads.Add(thread);

	return thread;
}

wxJSONValue SeruroServerAPI::GetAuth(wxJSONValue params)
{
	wxJSONValue auth;

	/* Determine address to request token for (from given server). If an address is not given, search for ANY token for this server. */
    auth["token"] = wxEmptyString;
	if (params.HasMember("address")) {
        /* Pin this request to an address. If the auth fails, the UI control will not allow the user to auth with another address. */
        auth["address"] = params["address"];
        if (params["server"].HasMember("uuid")) {
            auth["token"] = wxGetApp().config->GetToken(params["server"]["uuid"].AsString(), params["address"].AsString());
        }
	} else {
        /* Assume a uuid is present. */
        auth["token"] = wxGetApp().config->GetActiveToken(params["server"]["uuid"].AsString());
    }

    if (params.HasMember("password")) {
        /* There was a password provided. */
        auth["password"] = params["password"];
        //params["password"].Clear();
        
        /* An auth attempt may require an explicit password (if another UI is handling login), else a UI prompt may appear. */
        auth["require_password"] = params.HasMember("require_password");
    }

    /* Set a boolean indicating weather a token is available. */
	auth["have_token"] = (auth["token"].AsString().size() > 0) ? true : false;

	if (! auth["have_token"].AsBool()) {
		wxLogMessage(wxT("SeruroServerAPI::GetAuth> failed to find valid auth token."));
	}

    return auth;
}

wxJSONValue SeruroServerAPI::GetRequest(api_name_t name, wxJSONValue params)
{
	wxJSONValue request;

	/* Most API calls to the server are currently GETs. */
	request["verb"] = _("GET");
	request["flags"] = SERURO_SECURITY_OPTIONS_NONE;
    
	/* (POST-DATA) Set multi-value (dict) "data" to a JSON Value. */
	request["data"] = wxJSONValue(wxJSONTYPE_OBJECT);
    /* (QUERY-STRING) Set multi-value (dict) "query" to a JSON value. */
    request["query"] = wxJSONValue(wxJSONTYPE_OBJECT);

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

	return request;
}

bool SeruroServerAPI::InstallCA(wxJSONValue response)
{
	if (! CheckResponse(response, "ca")) return false;

	wxString ca_encoded;
	wxMemoryBuffer ca_decoded;
    wxString server_uuid;
    wxString ca_fingerprint;
    bool result;
    
	SeruroCrypto crypto;
    
    server_uuid = response["server_uuid"].AsString();
	ca_encoded = response["ca"].AsString();
	if (! DecodeBase64(ca_encoded, &ca_decoded)) return false;

    ca_fingerprint = wxEmptyString;
	result = crypto.InstallCA(ca_decoded, ca_fingerprint);

	/* Set the CA fingerprint. */
    if (result) {
        wxGetApp().config->SetCAFingerprint(server_uuid, ca_fingerprint);
    }

    wxLogMessage(_("SeruroServerAPI> (InstallCA) CA for (%s) status: %s"),
        server_uuid, (result) ? _("success") : _("failed"));
    
	return result;
}

bool SeruroServerAPI::InstallCertificate(wxJSONValue response)
{
	if (! CheckResponse(response, "certs")) return false;
    //if (! CheckResponse(response, "address")) return false;

	wxString cert_encoded;
	wxMemoryBuffer cert_decoded;
    wxString server_uuid;
    wxString cert_fingerprint;
    bool result;

	SeruroCrypto crypto;

	result = true;
    server_uuid = response["server_uuid"].AsString();
	for (int i = 0; i < response["certs"].Size(); i++) {
		cert_encoded = response["certs"][i].AsString();

		if (! DecodeBase64(cert_encoded, &cert_decoded)) continue;

        cert_fingerprint = wxEmptyString; /* must reset the fingerprint */
		result = (result && crypto.InstallCertificate(cert_decoded, cert_fingerprint));
        if (! result) continue;

		/* Track this certificate. */
		wxGetApp().config->AddCertificate(server_uuid, response["address"].AsString(), cert_fingerprint);
	}
    
    wxLogMessage(_("SeruroServerAPI> (InstallCertificate) Certificate for (%s, %s) status: %s"),
        server_uuid, response["address"].AsString(), (result) ? _("success") : _("failed"));
    
    if (! result) {
        /* Roll back any changes. */
    }

	return result;
}

bool SeruroServerAPI::InstallP12(wxJSONValue response, wxString key, bool force_install)
{
	if (! CheckResponse(response, "p12")) return false;

	/* Get password from user for p12 containers. */
	wxString p12_key;

	if (key.compare(wxEmptyString) == 0 && force_install == false) {
        /* If a key is not provided, then prompt the UI for one, unless the install is forced. */
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
	wxString server_uuid = response["server_uuid"].AsString();
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
	wxArrayString old_fingerprints = wxGetApp().config->GetIdentity(server_uuid, address);
	for (size_t i = 0; i < old_fingerprints.size(); i++) {
		wxGetApp().config->RemoveIdentity(server_uuid, address, false);
	}

	/* Add the fingerprints. */
	for (size_t i = 0; i < fingerprints.size(); i++) {
		wxGetApp().config->AddIdentity(server_uuid, address, fingerprints[i]);
	}

	return true;
}

bool SeruroServerAPI::UninstallIdentity(wxString server_uuid, wxString address)
{
    wxArrayString fingerprints;
    SeruroCrypto crypto;
    
    fingerprints = wxGetApp().config->GetIdentity(server_uuid, address);
    if (fingerprints.size() == 0) return false;
    
    if (! crypto.RemoveIdentity(fingerprints)) {
        wxLogMessage(_("SeruroServerAPI> (UninstallIdentity) could not remove (%s) (%s)."), server_uuid, address);
        return false;
    }
    return wxGetApp().config->RemoveIdentity(server_uuid, address, true);
}

bool SeruroServerAPI::UninstallCertificates(wxString server_uuid, wxString address)
{
    wxArrayString fingerprints;
    SeruroCrypto crypto;
    
    fingerprints = wxGetApp().config->GetCertificates(server_uuid, address);
    if (fingerprints.size() == 0) return false;
    
    if (! crypto.RemoveCertificates(fingerprints)) {
        wxLogMessage(_("SeruroServerAPI> (UninstallAddress) could not remove (%s) (%s)."), server_uuid, address);
        return false;
    }
    return wxGetApp().config->RemoveCertificates(server_uuid, address, true);
}

bool SeruroServerAPI::UninstallCA(wxString server_uuid)
{
    wxString fingerprint;
    SeruroCrypto crypto;
    
    fingerprint = wxGetApp().config->GetCA(server_uuid);
    if (fingerprint.compare(wxEmptyString) == 0) return false;
    
    if (! crypto.RemoveCA(fingerprint)) {
        wxLogMessage(_("SeruroServerAPI> (UninstallCA) could not remove (%s)."), server_uuid);
        return false;
    }
    return wxGetApp().config->RemoveCACertificate(server_uuid, true);
}


