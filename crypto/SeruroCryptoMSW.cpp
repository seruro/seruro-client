
/* First thing, detect OS */
#if defined(__WXMSW__)

#include <wx/log.h>
#include <wx/base64.h>

#include <Windows.h>
#include <winhttp.h>
#include <comutil.h>

#pragma comment(lib, "winhttp")
#pragma comment(lib, "crypt32")
#pragma comment(lib, "comsuppw")

/* Code goes here */
#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "SeruroCryptoMSW.h"

DECLARE_APP(SeruroClient);

/* Helper function to convert wxString to a L, the caller is responsible for memory. */
BSTR AsLongString(const wxString &input)
{
	int size = lstrlenA(input.mb_str(wxConvUTF8));
	BSTR long_object = SysAllocStringLen(NULL, size);
	::MultiByteToWideChar(CP_ACP, 0, input, size, long_object, size);
	return long_object;
}

void AsMultiByte(const wxString &input, LPSTR *result)
{
	BSTR object = AsLongString(input);
	*result = _com_util::ConvertBSTRToString(object);
	::SysFreeString(object);
}

/* Overload for easiness! */
BSTR AsLongString(const char* input)
{
	wxString wx_input(input);
	return AsLongString(wx_input);
}

bool IsHashInstalledAndSet(wxString address, wxMemoryBuffer hash)
{
	SeruroCryptoMSW crypto_helper;
	wxString fingerprint;

	/* Crypto expects all certificate as base64 encoded buffers. */
	if (! crypto_helper.HaveIdentityByHash(wxBase64Encode(hash))) {
		/* No certificate exists. */
		return false;
	}
	
	/* Search the certificate store for that HASH value, and retreive the SKID, then check config. */
	fingerprint = crypto_helper.GetIdentitySKIDByHash(wxBase64Encode(hash));
	/* This should always work, the certificate DOES exist. */

	/* Get all identities, no server is specified, and try to match the fingerprint. */
	wxArrayString server_list, address_list, cert_list;

	server_list = theSeruroConfig::Get().GetServerList();
	for (size_t i = 0; i < server_list.size(); i++ ) {
		address_list = theSeruroConfig::Get().GetAddressList(server_list[i]);
		for (size_t j = 0; j < address_list.size(); j++) {
			if (address_list[j] != address) { continue; }

			/* Check both certificates. */
			cert_list = theSeruroConfig::Get().GetIdentity(server_list[i], address_list[j]);
			for (size_t k = 0; k < cert_list.size(); k++) {
				if (fingerprint == cert_list[k]) {
					/* Possibly fill in some server value. */
					return true;
				}
			}
		}
	}

	return false;
}

void SeruroCryptoMSW::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MSW> Initialized"));

	//TLSRequest(none, 0, verb, object, data); /* SERURO_SECURITY_OPTIONS_DATA */
}

bool OpenCertificateStore(wxString store_name, HCERTSTORE &cert_store)
{
	DWORD error_num;
	BSTR store = AsLongString(store_name);

	/* This should set the "existing" flag and MUST set the "CURRENT_USER" high word. */
	/* The canonical name of the certificate store is passed to the function */
	cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, 
		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, store);

	if (cert_store == NULL) {
		error_num = GetLastError();
		wxLogMessage(wxT("SeruroCrypto::OpenCertificateStore> could not open CURRENT_USER/'%s' store (%u)."), 
			store_name, error_num); 
		return false;
	}

	return true;
}

bool InstallCertificateToStore(const wxMemoryBuffer &cert, wxString store_name, wxString &fingerprint)
{
	HCERTSTORE cert_store;
	PCCERT_CONTEXT cert_context;

	/* The canonical name of the certificate store is passed to the function */
	if (! OpenCertificateStore(store_name, cert_store)) {
		return false;
	}

	BOOL result;
	DWORD error_num;	
	//BYTE *data = (BYTE *) cert.GetData();
	//DWORD len = cert.GetDataLen();

	/* Add the encoded certificate, receive the decoded context (for fingerprinting). */
	result = CertAddEncodedCertificateToStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		(BYTE *) cert.GetData(), cert.GetDataLen(), CERT_STORE_ADD_REPLACE_EXISTING, &cert_context);

	if (result == false) {
		error_num = GetLastError();
		wxLogMessage(wxT("SeruroCrypto::InstallCA> could not decode cert (%u)."), error_num);
		CertCloseStore(cert_store, 0);
		return false;
	}

	/* Retreive the subject key ID as the certificate fingerprint. */
	fingerprint.Append(GetFingerprintFromCertificate(cert_context, BY_SKID));

	wxLogMessage(wxT("SeruroCrypto::InstallCA> cert installed."));
	CertFreeCertificateContext(cert_context);
	CertCloseStore(cert_store, 0);
	return true;
}

bool RemoveCertificatesFromStore(wxArrayString &fingerprints, wxString store_name)
{
	PCCERT_CONTEXT cert_context;
	bool status = true;

	for (size_t i = 0; i < fingerprints.size(); ++i) {
		cert_context = GetCertificateByFingerprint(fingerprints[i], store_name, BY_SKID);
		status = status && (CertDeleteCertificateFromStore(cert_context) == true) ? true : false;
	}

	return status;
}

bool InstallIdentityToStore(const PCCERT_CONTEXT &identity, wxString store_name)
{
	BOOL result;
	//DWORD error_num;
	HCERTSTORE identity_store;

	if (! OpenCertificateStore(store_name, identity_store)) {
		return false;
	}

	/* This is the user's "Personal" store, while iterating, this should identify included
	 * CA certificates and potentially handle them differently. */

	/* Get a display-representation of the certificate name (for logging). */
	wchar_t cert_name[256];
	result = CertGetNameString(identity, CERT_NAME_FRIENDLY_DISPLAY_TYPE, 0, 0, cert_name, 256);
	cert_name[256-1] = 0;

	if (! result) {
		wxLogMessage(wxT("SeruroCrypto::InstallP12> could not get certificate display name."));
	} else {
		wxLogMessage(wxT("SeruroCrypto::InstallP12> found certificate: %s"), cert_name);
	}

	/* Add the certificate to the store as a "NEW" certificate, to detect potential duplicates. */
	result = CertAddCertificateContextToStore(identity_store, identity, CERT_STORE_ADD_NEW, 0);
	if (! result && GetLastError() == CRYPT_E_EXISTS) {
		wxLogMessage(_("SeruroCrypto> (InstallP12) Warning! certificate exists, replacing."));
		result = CertAddCertificateContextToStore(identity_store, identity, CERT_STORE_ADD_REPLACE_EXISTING, 0);
	}

	CertCloseStore(identity_store, 0);

	if (! result) {
		/* At this point we should also backup and remove any certs added. */
		wxLogMessage(_("SeruroCrypto> (InstallP12) problem overwriting certificate."));
		return false;
	}
	return true;
}

bool CertificateHasKey(const PCCERT_CONTEXT &cert)
{
	DWORD property_id;
	//DWORD property_size;
	//void *property_data;

	property_id = 0;
	while (property_id = CertEnumCertificateContextProperties(cert, property_id)) {
		if (property_id == CERT_KEY_PROV_INFO_PROP_ID) {
			return true;
		}
	}

	return false;
}

bool HaveCertificateByFingerprint(wxString fingerprint, wxString store_name, search_type_t match_type)
{
	bool have_cert;
	PCCERT_CONTEXT cert;

	cert = GetCertificateByFingerprint(fingerprint, store_name, match_type);
	have_cert = (cert != NULL);

	return have_cert;
}

wxString GetSKIDByHash(wxString hash, wxString store_name)
{
	PCCERT_CONTEXT cert;

	cert = GetCertificateByFingerprint(hash, store_name, BY_HASH);
	if (cert == NULL) {
		return wxEmptyString;
	}

	/* Extract SKID (szOID_SUBJECT_KEY_IDENTIFIER). */
	return GetFingerprintFromCertificate(cert, BY_SKID);
}

wxString GetHashBySKID(wxString skid, wxString store_name)
{
	PCCERT_CONTEXT cert;

	cert = GetCertificateByFingerprint(skid, store_name, BY_SKID);
	if (cert == NULL) {
		return wxEmptyString;
	}

	return GetFingerprintFromCertificate(cert, BY_HASH);
}

PCCERT_CONTEXT GetCertificateByFingerprint(wxString fingerprint, wxString store_name, search_type_t match_type)
{
	HCERTSTORE cert_store;

	if (! OpenCertificateStore(store_name, cert_store)) {
		return NULL;
	}

	/* Convert the base64 fingerprint to the 160-bit SHA1 hash. */
	CRYPT_HASH_BLOB hash;
	wxMemoryBuffer hash_value = wxBase64Decode(fingerprint);
	if (hash_value.GetDataLen() != 20) {
		wxLogMessage(_("SeruroCrypto> (GetCertificateByFingerprint) hash value was not 20 bytes?"));
		return NULL;
	}
	hash.pbData = (BYTE *) hash_value.GetData();
	hash.cbData = hash_value.GetDataLen();

	/* Search the store for the hash. */
	PCCERT_CONTEXT cert;
	if (match_type == BY_SKID) {
		cert = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
			0, CERT_FIND_KEY_IDENTIFIER, (void *) &hash, NULL);
	} else {
		cert = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
			0, CERT_FIND_HASH, (void *) &hash, NULL);
	}

	/* Todo: should this close the certificate store? */

	return cert;
}

wxString GetEncodedByFingerprint(wxString fingerprint, wxString store_name, search_type_t match_type)
{
    /* A wrapper for the above method, but representes the certificate as an encoded string. */
    PCCERT_CONTEXT cert;
    wxString cert_data;

    /* Get the certificate context using the given parameters. */
    cert = GetCertificateByFingerprint(fingerprint, store_name, match_type);
    cert_data = wxBase64Encode(cert->pbCertEncoded, cert->cbCertEncoded);
    
    CertFreeCertificateContext(cert);
    return cert_data;
}

/* Calculate SHA1 for thumbprinting. */
//wxString GetCertificateHash(const wxMemoryBuffer &cert)
//{
//	HCRYPTPROV crypto_provider;
//	HCRYPTHASH hash;
//
//	BYTE *data = (BYTE *) cert.GetData();
//	DWORD len = cert.GetDataLen();
//
//	CryptAcquireContext(&crypto_provider, NULL, NULL, PROV_RSA_FULL, 0);
//	CryptCreateHash(crypto_provider, CALG_SHA1, 0, 0, &hash);
//	CryptHashData(hash, data, len, 0);
//
//	BYTE hash_value[20];
//	CryptGetHashParam(hash, HP_HASHVAL, (BYTE*) hash_value, &len, 0);
//
//	/* Convert raw data to base64, then to string. */
//	wxMemoryBuffer hash_buffer;
//	hash_buffer.AppendData((void *) hash_value, 20);
//
//	return wxBase64Encode(hash_buffer);
//}

/* Get subject key id from certificate context. */
wxString GetFingerprintFromCertificate(PCCERT_CONTEXT &cert, search_type_t match_type)
{
	BOOL result;
	BYTE *id_data;
	DWORD id_size, error_num;

	/* CERT_HASH_PROP_ID */
	/* Get the "szOID_SUBJECT_KEY_IDENTIFIER" or hash of public key from certificate. */
	if (match_type == BY_SKID) {
		result = CertGetCertificateContextProperty(cert, CERT_KEY_IDENTIFIER_PROP_ID, NULL, &id_size);
	} else {
		result = CertGetCertificateContextProperty(cert, CERT_HASH_PROP_ID, NULL, &id_size);
	}

	if (! result || id_size <= 0) {
		// error 
		error_num = GetLastError();
		wxLogMessage(_("SeruroCrypto> (GetFingerprintFromCert) cannot get size of key data (%u)."), error_num);
		return wxEmptyString;
	}

	id_data = (BYTE *) malloc(sizeof(BYTE) * id_size);
	if (match_type == BY_SKID) {
		result = CertGetCertificateContextProperty(cert, CERT_KEY_IDENTIFIER_PROP_ID, id_data, &id_size);
	} else {
		result = CertGetCertificateContextProperty(cert, CERT_HASH_PROP_ID, id_data, &id_size);
	}

	if (! result) { 
		error_num = GetLastError();
		wxLogMessage(_("SeruroCrypto> (GetFingerprintFromCert) cannot get key data (%u)."), error_num);
		return wxEmptyString;
	}

	wxMemoryBuffer id_buffer;
	id_buffer.AppendData(id_data, id_size);

	return wxBase64Encode(id_buffer);
}

/* Todo: Errors should be events. */

/* The TLS Request will assure the server meets the client's requirements for security.
 * We can optionally lower security expectations for TLS and session key size.
 * 
 * Usage guide: 
 *   params["options"] = (DATA | STRONG | TLS12)
 * The important options for this set of flags are DATA and (removed CLIENT). 
 * DATA: will attach the params["data"] string to the request, after the headers. 
 *  This is used to send POST data if set. Otherwise it is attached as a header.
 */
wxString SeruroCryptoMSW::TLSRequest(wxJSONValue params)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;

	wxString responseString("");

	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

	/* Get useragent from config */
	BSTR userAgent = AsLongString(SERURO_DEFAULT_USER_AGENT);

	/* Get the server address from config */
	BSTR serverAddress = AsLongString(params["server"]["host"].AsString());
	BSTR verb = AsLongString(params["verb"].AsString());
	BSTR object = AsLongString(params["object"].AsString());
	wxString data_string = params["data_string"].AsString();
	//const char *data_raw = params["data_string"].AsString().mb_str(wxConvUTF8);
	LPCSTR data = data_string.mb_str(wxConvUTF8);

	/* Create session object. */
	hSession = WinHttpOpen( userAgent, 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	::SysFreeString(userAgent);

	/* Create connection. */
	hConnect = WinHttpConnect(hSession, serverAddress, params["server"]["port"].AsInt(), 0);
	::SysFreeString(serverAddress);

	wxLogMessage(wxT("SeruroCrypto::TLS> Received, options: %d."), params["flags"].AsInt());

	/* Set TLS1.2 only! (...doesn't seem to work) */
	BOOL bResults = FALSE;
	DWORD dwOpt = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1; /* Todo: change me. TLS1_2 */
	bResults = WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwOpt, sizeof(dwOpt));
	if (! bResults) {
		wxLogMessage("SeruroCrypto::TLS> Cannot set client support TLS1.");
		/* Todo: fail if cannot support 1_2 */
	}

	/* Open, and request SSL, defaults to TLSv1.0. */
	hRequest = WinHttpOpenRequest(hConnect, verb, object, NULL, 
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	::SysFreeString(verb);
	::SysFreeString(object);

	/* Calculate length of "data", which comes after heads, if any, add a urlencoded Content-Type. */
	DWORD dwOptionalLength = (params["flags"].AsInt() & SERURO_SECURITY_OPTIONS_DATA) ? 
		params["data_string"].AsString().length() : 0;
	/* no Content-Type: application/json support. */
	WinHttpAddRequestHeaders(hRequest, 
		L"Content-Type: application/x-www-form-urlencoded\r\nAccept: application/json", (ULONG)-1L,
		WINHTTP_ADDREQ_FLAG_ADD);

	/* Send VERB and OBJECT (if post data exists it will be sent as well). */
	bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		 (LPVOID) data, dwOptionalLength, dwOptionalLength, 0);

	if (! bResults) {
		wxLogMessage(wxT("WinHttpSendResponse error: %s"), wxString::Format(wxT("%d"), GetLastError()));
		if (GetLastError() == ERROR_WINHTTP_SECURE_FAILURE) {
			/* The server certificate is invalid. */
			wxLogMessage("SeruroCrypto::TLS> server certificate verification failed.");
			goto bailout;
		}
	}

	/* Make sure the connection is MAX security. */
	DWORD securitySupport;
	dwSize = sizeof(DWORD);
	bResults = WinHttpQueryOption(hRequest,
		WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID) &securitySupport, &dwSize);

	if (! bResults || !(securitySupport & SECURITY_FLAG_STRENGTH_STRONG)) {
		wxLogMessage("SeruroCrypto::TLS> Server does not support strong security!");
		/* Warning: The server DOES not meet requirements! */
		/* Todo: fail here and report. */
		goto bailout;
	}

	/* As a check, try to set MAX security, if this fails, that is a good thing. */
	/* Consider pinning techniques here. */

	/* End the request. */
	bResults = WinHttpReceiveResponse(hRequest, NULL);

	/* Check for unhandled error states. */
	if (! bResults) {
		wxLogMessage(wxT("WinHttpReceiveResponse error: %s"), wxString::Format(wxT("%d"), GetLastError()));
		/* Unhandled error state. */
		goto bailout;
	}

	/* Read response data. */
	do {
		dwSize = 0;
		if (! WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			wxLogMessage(wxT("SeruroCryptp:TLS> error in WinHttpReadData."));
		}
		pszOutBuffer = new char[dwSize+1];
		if (!pszOutBuffer) {
			// Out of memory
			dwSize = 0;
		} else {
			ZeroMemory(pszOutBuffer, dwSize+1);
			if (! WinHttpReadData(hRequest, (LPVOID) pszOutBuffer, dwSize, &dwDownloaded)) {
				//error
			} else {
				/* Ruby errors are UFT-8. */
				responseString = responseString + wxString::FromUTF8(pszOutBuffer, dwDownloaded);
			}
			delete [] pszOutBuffer;
		}
	} while (dwSize > 0);
	wxLogMessage("SeruroCrypto::TLS> Response (TLS) success.");

	wxLogMessage(wxT("SeruroCrypto::TLS> read body: %s"), responseString);


	/* Call some provided callback (optional). */

	goto finished;

bailout:
	/* Send error event */
	wxLogMessage("SeruroCrypto::TLS> error occured.");

finished:
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return responseString;
}

bool SeruroCryptoMSW::InstallCA(const wxMemoryBuffer &ca, wxString &fingerprint)
{
    bool status;
    
	status = InstallCertificateToStore(ca, _(CERTSTORE_TRUSTED_ROOT), fingerprint);
    //fingerprint.Append(GetFingerprintFromBuffer(ca));
    
    return status;
}

bool SeruroCryptoMSW::InstallCertificate(const wxMemoryBuffer &cert, wxString &fingerprint)
{
	/* Store name: AddressBook */
    bool status;
    
	status = InstallCertificateToStore(cert, _(CERTSTORE_CONTACTS), fingerprint);
    //fingerprint.Append(GetFingerprintFromBuffer(cert));
    
    return status;
}

bool SeruroCryptoMSW::InstallP12(const wxMemoryBuffer &p12, const wxString &p_password, 
	wxArrayString &fingerprints)
{
	CRYPT_DATA_BLOB blob;

	blob.cbData = p12.GetDataLen();
	blob.pbData = (BYTE*) p12.GetData();

	/* Test input P12. */
	if (! PFXIsPFXBlob(&blob)) {
		wxLogMessage(wxT("SeruroCrypto::InstallP12> p12 (size: %d) is not a recognized PFX."), 
			p12.GetDataLen());
		return false;
	}

	/* Create temporary store (MSDN does not say if this means in-memory). */
	HCERTSTORE pfx_store;
	BSTR password = AsLongString(p_password);
	pfx_store = PFXImportCertStore(&blob, password, CRYPT_USER_KEYSET | CRYPT_EXPORTABLE);
	::SysFreeString(password);

	/* PFXImport... will not return a valid handle if the operation was unsuccessfull.
	 * GetLastError() will yield the exact error. */
	if (pfx_store == NULL) {
		wxLogMessage(wxT("SeruroCrypto::InstallP12> could not decrypt P12."));
		return false;
	}

	PCCERT_CONTEXT cert = NULL;
	BOOL result;
	/* Iterate through certificates within P12 cert store, using the current cert as a "previous" iterator. */
	while (NULL != (cert = CertEnumCertificatesInStore(pfx_store, cert))) {
		if (! CertificateHasKey(cert)) {
			/* Do not add CAs or other additional certificates from the P12. */
			continue;
		}
		result = InstallIdentityToStore(cert, _(CERTSTORE_PERSONAL));
		if (result) {
			fingerprints.Add(GetFingerprintFromCertificate(cert, BY_SKID));
		}
	}

	CertCloseStore(pfx_store, 0);

	return true;
}

bool SeruroCryptoMSW::HaveIdentityByHash(wxString hash)
{
	return HaveCertificateByFingerprint(hash, CERTSTORE_PERSONAL, BY_HASH);
}

wxString SeruroCryptoMSW::GetIdentitySKIDByHash(wxString hash)
{
	return GetSKIDByHash(hash, CERTSTORE_PERSONAL);
}

wxString SeruroCryptoMSW::GetIdentityHashBySKID(wxString skid)
{
	return GetHashBySKID(skid, CERTSTORE_PERSONAL);
}

bool SeruroCryptoMSW::HaveIdentity(wxString server_name, wxString address, wxString fingerprint)
{
    wxArrayString identity;
    bool cert_exists = true;
    
	/* First get the fingerprint string from the config. */
    if (fingerprint.compare(wxEmptyString) == 0) {
        if (! theSeruroConfig::Get().HaveIdentity(server_name, address)) return false;
        identity = theSeruroConfig::Get().GetIdentity(server_name, address);
    } else {
        identity.Add(fingerprint);
    }

	/* Looking at the trusted Root store. */
    for (size_t i = 0; i < identity.size(); i++) {
        cert_exists = (cert_exists && HaveCertificateByFingerprint(identity[i], CERTSTORE_PERSONAL));
    }
    
    return cert_exists;
}

bool SeruroCryptoMSW::HaveCA(wxString server_name, wxString fingerprint)
{ 
	wxString ca_fingerprint;

	/* First get the fingerprint string from the config. */
	if (fingerprint.compare(wxEmptyString) == 0) {
		if (!theSeruroConfig::Get().HaveCA(server_name)) return false;
		ca_fingerprint = theSeruroConfig::Get().GetCA(server_name);
	} else {
		ca_fingerprint = fingerprint;
	}

	/* Looking at the personal (my) store. */
	bool in_store = HaveCertificateByFingerprint(ca_fingerprint, CERTSTORE_TRUSTED_ROOT);
	wxLogMessage(_("SeruroCrypto> (HaveCA) certificate (%s) in store: (%s)."), 
		server_name, (in_store) ? "true" : "false");
	return in_store;
}

bool SeruroCryptoMSW::HaveCertificates(wxString server_name, wxString address, wxString fingerprint)
{ 
	/* First get the fingerprint string from the config. */
	if (! theSeruroConfig::Get().HaveCertificates(server_name, address)) return false;
	wxArrayString identity = theSeruroConfig::Get().GetCertificates(server_name, address);

	if (identity.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveCertificates) the address (%s) (%s) does not have 2 certificates?"),
			server_name, address);
		return false;
	}

	/* Looking at the address book store. */
	bool cert_1 = HaveCertificateByFingerprint(identity[0], CERTSTORE_CONTACTS);
	bool cert_2 = HaveCertificateByFingerprint(identity[1], CERTSTORE_CONTACTS);
	wxLogMessage(_("SeruroCrypto> (HaveCertificates) address (%s) (%s) in store: (1: %s, 2: %s)."), 
		server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMSW::RemoveIdentity(wxArrayString fingerprints)
{
	bool status;
	status = RemoveCertificatesFromStore(fingerprints, _(CERTSTORE_PERSONAL));
	return status;
}

bool SeruroCryptoMSW::RemoveCA(wxString fingerprint)
{ 
	bool status;
	wxArrayString fingerprints;

	fingerprints.Add(fingerprint);
	status = RemoveCertificatesFromStore(fingerprints, _(CERTSTORE_TRUSTED_ROOT));
	return status;

}

bool SeruroCryptoMSW::RemoveCertificates(wxArrayString fingerprints)
{
	bool status;
	status = RemoveCertificatesFromStore(fingerprints, _(CERTSTORE_CONTACTS));
	return status;
}

#endif
