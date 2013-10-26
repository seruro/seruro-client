
#include "SeruroServerAPI.h"
#include "Utils.h"

#include "../SeruroConfig.h"
#include "../SeruroClient.h"
#include "SeruroRequest.h"
#include "../frames/dialogs/DecryptDialog.h"
#include "../crypto/SeruroCrypto.h"

#include "../logging/SeruroLogger.h"

#include <wx/base64.h>
#include <wx/buffer.h>

DECLARE_APP(SeruroClient);

bool CheckResponse(wxJSONValue response, wxString required_key)
{
	if (! response.HasMember("success") || ! response["success"].AsBool()) {
		DEBUG_LOG(_("ServerAPI> Bad Result."));
		return false;
	}

	/* The getP12 API call should respond with up to 3 P12s. */
	if (! response.HasMember(required_key)) {
		DEBUG_LOG(_("ServerAPI> Response does not include '%s' data."), required_key);
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
		DEBUG_LOG(_("ServerAPI> (Decode) Could not decode position %d in ca blob '%s'."), 
				decode_error, encoded);
		return false;
	}

	return true;
}

wxJSONValue SeruroServerAPI::GetServer(const wxString &server_uuid)
{
	//wxJSONValue server_info;
	return theSeruroConfig::Get().GetServer(server_uuid);
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
SeruroRequest *SeruroServerAPI::CreateRequest(api_name_t name, wxJSONValue params, seruro_api_callbacks_t evtId)
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
	int request_api_id = (int) evtId;
	SeruroRequest *thread = new SeruroRequest(params["request"], evtHandler, request_api_id);
    //params.Clear();

	if (thread->Create() != wxTHREAD_NO_ERROR) {
		DEBUG_LOG("ServerAPI> (CreateRequest) Could not create thread.");
	}

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
            auth["token"] = theSeruroConfig::Get().GetToken(params["server"]["uuid"].AsString(), 
				params["address"].AsString());
        }
	} else {
        /* Assume a uuid is present. */
        auth["token"] = theSeruroConfig::Get().GetActiveToken(params["server"]["uuid"].AsString());
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
        case SERURO_API_UPDATE:
            if (! params.HasMember("update")) {
                /* Error and stop. */
            }
            request["query"]["update"] = params["update"];
            request["object"] = _(SERURO_API_OBJECT_UPDATE);
            break;
        case SERURO_API_CA:
            request["object"] = _(SERURO_API_OBJECT_CA);
            break;
        case SERURO_API_CRL:
            request["object"] = _(SERURO_API_OBJECT_CRL);
            break;
        case SERURO_API_PING:
            request["object"] = _(SERURO_API_OBJECT_PING);
            break;
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
    bool result = true;
    
	SeruroCrypto crypto;
    
	/* Attempt to decode the CA certificate. */
    server_uuid = response["server_uuid"].AsString();
	ca_encoded = response["ca"][1].AsString();
	if (! DecodeBase64(ca_encoded, &ca_decoded)) return false;

	/* Set the fingerprint to the SKID returned by the API call. */
	ca_fingerprint = response["ca"][0].AsString();

	/* Check if the user already has the CA SKID installed. */
	if (! crypto.HaveCA(server_uuid, ca_fingerprint)) {
		/* Otherwise clear and set the fingerprint through the native API. */
		ca_fingerprint.Clear();
		result = crypto.InstallCA(ca_decoded, ca_fingerprint);
	} else {
		/* No action needed. */
		result = true;
	}

	/* Set the CA fingerprint. */
    if (result) {
        theSeruroConfig::Get().SetCAFingerprint(server_uuid, ca_fingerprint);
    }

    DEBUG_LOG(_("ServerAPI> (InstallCA) CA for (%s) status: %s"),
        server_uuid, (result) ? _("success") : _("failed"));
    
	return result;
}

bool SeruroServerAPI::InstallCertificate(wxJSONValue response)
{
	if (! CheckResponse(response, "certs")) return false;
    //if (! CheckResponse(response, "address")) return false;

	wxString cert_encoded;
	wxMemoryBuffer cert_decoded;
    wxString server_uuid, address;
    wxString cert_fingerprint;
    bool result;

	SeruroCrypto crypto;

	result = true;
    server_uuid = response["server_uuid"].AsString();
	address = response["address"].AsString();

	for (int i = 0; i < response["certs"].Size(); i++) {
		cert_encoded = response["certs"][i].AsString();

		if (! DecodeBase64(cert_encoded, &cert_decoded)) continue;

        cert_fingerprint = wxEmptyString; /* must reset the fingerprint */
		result = (result && crypto.InstallCertificate(cert_decoded, cert_fingerprint));
        if (! result) continue;

		/* Track this certificate. Todo: pull out of a loop. */
		theSeruroConfig::Get().AddCertificate(server_uuid, address,
			(i == 0) ? ID_AUTHENTICATION : ID_ENCIPHERMENT, cert_fingerprint);
	}
    
    DEBUG_LOG(_("ServerAPI> (InstallCertificate) Certificate for (%s, %s) status: %s"),
        server_uuid, address, (result) ? _("success") : _("failed"));
    
    if (! result) {
        /* Todo: Roll back any changes. */
    }

	return result;
}

bool SeruroServerAPI::InstallP12(wxString server_uuid, wxString address, identity_type_t cert_type,
    wxString encoded_p12, wxString unlock_code, bool force_install)
{
    wxArrayString  fingerprints;
    wxMemoryBuffer decoded_p12;
    wxString existing_fingerprint;
    
    if (! DecodeBase64(encoded_p12, &decoded_p12)) return false;
    
    /* Add the identity (p12) to the certificate store. */
    SeruroCrypto crypto;
    if (! crypto.InstallP12(decoded_p12, unlock_code, fingerprints)) {
        DEBUG_LOG(_("ServerAPI> (InstallP12) could not install (%d) p12."), cert_type);
        return false;
    } else {
        DEBUG_LOG(_("ServerAPI> (InstallP12) installed (%d) p12."), cert_type);
    }

	if (fingerprints.size() == 0) {
		DEBUG_LOG(_("ServerAPI> (InstallP12) p12 contained no identity certificates."));
		return false;
	}
    
    existing_fingerprint = theSeruroConfig::Get().GetIdentity(server_uuid, address, cert_type);
    theSeruroConfig::Get().RemoveIdentity(server_uuid, address, cert_type, false);
    theSeruroConfig::Get().AddIdentity(server_uuid, address, cert_type, fingerprints[0]);
    
    return true;
}

bool SeruroServerAPI::UninstallIdentity(wxString server_uuid, wxString address)
{
    wxArrayString fingerprints;
    SeruroCrypto crypto;
    
    fingerprints = theSeruroConfig::Get().GetIdentity(server_uuid, address);
    if (fingerprints.size() == 0) return false;
    
    if (! crypto.RemoveIdentity(fingerprints)) {
        DEBUG_LOG(_("ServerAPI> (UninstallIdentity) could not remove (%s) (%s)."), server_uuid, address);
        return false;
    }
    
    theSeruroConfig::Get().RemoveIdentity(server_uuid, address, ID_AUTHENTICATION, true);
    theSeruroConfig::Get().RemoveIdentity(server_uuid, address, ID_ENCIPHERMENT, true);
    
    return true;
}

bool SeruroServerAPI::UninstallCertificates(wxString server_uuid, wxString address)
{
    wxArrayString fingerprints;
    SeruroCrypto crypto;
    
    fingerprints = theSeruroConfig::Get().GetCertificates(server_uuid, address);
    if (fingerprints.size() == 0) return false;
    
    if (! crypto.RemoveCertificates(fingerprints)) {
        DEBUG_LOG(_("ServerAPI> (UninstallAddress) could not remove (%s) (%s)."), server_uuid, address);
        return false;
    }
    
    theSeruroConfig::Get().RemoveCertificate(server_uuid, address, ID_AUTHENTICATION, true);
    theSeruroConfig::Get().RemoveCertificate(server_uuid, address, ID_ENCIPHERMENT, true);
    return true;
}

bool SeruroServerAPI::UninstallCA(wxString server_uuid)
{
    wxString fingerprint;
    SeruroCrypto crypto;
    
    fingerprint = theSeruroConfig::Get().GetCA(server_uuid);
    if (fingerprint.compare(wxEmptyString) == 0) return false;
    
    if (! crypto.RemoveCA(fingerprint)) {
        DEBUG_LOG(_("ServerAPI> (UninstallCA) could not remove (%s)."), server_uuid);
        return false;
    }
    return theSeruroConfig::Get().RemoveCACertificate(server_uuid, true);
}


