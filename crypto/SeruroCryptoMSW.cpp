
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

#define CERTIFICATE_STORE_TRUSTED_ROOT "Root"
#define CERTIFICATE_STORE_CONTACTS	   "AddressBook"
#define CERTIFICATE_STORE_PERSONAL	   "My"

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

void SeruroCryptoMSW::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MSW> Initialized"));

	//TLSRequest(none, 0, verb, object, data); /* SERURO_SECURITY_OPTIONS_DATA */
}

bool InstallCertToStore(const wxMemoryBuffer &cert, wxString store_name)
{
	BOOL bResult;
	DWORD error;

	HCERTSTORE cert_store;
	BSTR store = AsLongString(store_name);

	/* The canonical name of the certificate store is passed to the function */
	cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, 
		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, store);

	if (cert_store == NULL) {
		error = GetLastError();
		wxLogMessage(wxT("SeruroCrypto::InstallCA> could not open CURRENT_USER/'%s' store (%u)."), 
			store_name, error); 
		return false;
	}

	BYTE *data = (BYTE *) cert.GetData();
	DWORD len = cert.GetDataLen();

	bResult = CertAddEncodedCertificateToStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		data, len, CERT_STORE_ADD_REPLACE_EXISTING, NULL);

	if (bResult == false) {
		error = GetLastError();
		wxLogMessage(wxT("SeruroCrypto::InstallCA> could not decode cert (%u)."), error);
		CertCloseStore(cert_store, 0);
		return false;
	}

	wxLogMessage(wxT("SeruroCrypto::InstallCA> cert installed."));
	CertCloseStore(cert_store, 0);
	return true;
}

bool HaveCertificateByFingerprint(wxString fingerprint, wxString store_name)
{
	HCERTSTORE cert_store;
	BSTR store = AsLongString(store_name);

	/* Open the requested certificate store. */
	cert_store = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0,
		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, store);

	if (cert_store == NULL) {
		DWORD error = GetLastError();
		wxLogMessage(_("SeruroCrypto> (HaveCertificateByFingerprint) could not open CURRENT_USER/(%s) (%u)."),
			store_name, error);
		return false;
	}

	/* Convert the base64 fingerprint to the 160-bit SHA1 hash. */
	CRYPT_HASH_BLOB hash;
	wxMemoryBuffer hash_value = wxBase64Decode(fingerprint);
	if (hash_value.GetDataLen() != 20) {
		wxLogMessage(_("SeruroCrypto> (HaveCertificateByFingerprint) hash value was not 20 bytes?"));
		return false;
	}
	hash.pbData = (BYTE *) hash_value.GetData();
	hash.cbData = hash_value.GetDataLen();

	/* Search the store for the hash. */
	PCCERT_CONTEXT cert;
	cert = CertFindCertificateInStore(cert_store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 
		0, CERT_FIND_HASH, (void *) &hash, NULL);

	return (cert != NULL);
}

/* Calculate SHA1 for thumbprinting. */
wxString GetFingerprintFromBuffer(const wxMemoryBuffer &cert)
{
	HCRYPTPROV crypto_provider;
	HCRYPTHASH hash;

	BYTE *data = (BYTE *) cert.GetData();
	DWORD len = cert.GetDataLen();

	CryptAcquireContext(&crypto_provider, NULL, NULL, PROV_RSA_FULL, 0);
	CryptCreateHash(crypto_provider, CALG_SHA1, 0, 0, &hash);
	CryptHashData(hash, data, len, 0);

	BYTE hash_value[20];
	CryptGetHashParam(hash, HP_HASHVAL, (BYTE*) hash_value, &len, 0);

	//char digits[] = "0123456789abcdef";
	//for (DWORD i = 0; i < len; i++) {
	//	wxLogMessage(_("%c%c"), digits[hash_value[i] >> 4], digits[hash_value[i] & 0xf]);
	//}

	/* Convert raw data to base64, then to string. */
	wxMemoryBuffer hash_buffer;
	hash_buffer.AppendData((void *) hash_value, 20);

	return wxBase64Encode(hash_buffer);
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
				responseString = responseString + wxString::FromAscii(pszOutBuffer, dwDownloaded);
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
    
	status = InstallCertToStore(ca, _(CERTIFICATE_STORE_TRUSTED_ROOT));
    fingerprint.Append(GetFingerprintFromBuffer(ca));
    
    return status;
}

bool SeruroCryptoMSW::InstallCertificate(const wxMemoryBuffer &cert, wxString &fingerprint)
{
	/* Store name: AddressBook */
    bool status;
    
	status = InstallCertToStore(cert, _(CERTIFICATE_STORE_CONTACTS));
    fingerprint.Append(GetFingerprintFromBuffer(cert));
    
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
	HCERTSTORE pfxStore;
	BSTR password = AsLongString(p_password);
	pfxStore = PFXImportCertStore(&blob, password, CRYPT_USER_KEYSET | CRYPT_EXPORTABLE);
	::SysFreeString(password);

	/* PFXImport... will not return a valid handle if the operation was unsuccessfull.
	 * GetLastError() will yield the exact error. */
	if (pfxStore == NULL) {
		wxLogMessage(wxT("SeruroCrypto::InstallP12> could not decrypt P12."));
		return false;
	}

	HCERTSTORE myStore;
	wxString store_name = _(CERTIFICATE_STORE_PERSONAL);
	BSTR store = AsLongString(store_name);
	/* This should set the "existing" flag and MUST set the "CURRENT_USER" high word. */
	/* This is the user's "Personal" store, while iterating, this should identify included
	 * CA certificates and potentially handle them differently. */
	myStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, 0, 
		CERT_STORE_OPEN_EXISTING_FLAG | CERT_SYSTEM_STORE_CURRENT_USER, store);

	if (myStore == NULL) {
		wxLogMessage(_("SeruroCrypto::InstallP12> could not open CURRENT_USER/(%s) store."), store_name); 
		CertCloseStore(pfxStore, 0);
		return false;
	}

	PCCERT_CONTEXT cert = NULL;
	BOOL bResult;
	wchar_t certName[256];
	/* Iterate through certificates within P12 cert store, using the current cert as a "previous" iterator. */
	while (NULL != (cert = CertEnumCertificatesInStore(pfxStore, cert))) {
		bResult = CertGetNameString(cert, CERT_NAME_FRIENDLY_DISPLAY_TYPE, 0, 0, certName, 256);
		certName[256-1] = 0;
		if (! bResult) {
			wxLogMessage(wxT("SeruroCrypto::InstallP12> could not get certificate display name."));
		} else {
			wxLogMessage(wxT("SeruroCrypto::InstallP12> found certificate: %s"), certName);
		}

		/* fingerprint of P12. */
		wxMemoryBuffer cert_buffer;
		cert_buffer.AppendData(cert->pbCertEncoded, cert->cbCertEncoded);
		//wxGetApp().config->AddIdentity(server_name, address, GetFingerprint(cert_buffer));
		fingerprints.Add(GetFingerprintFromBuffer(cert_buffer));

		bResult = CertAddCertificateContextToStore(myStore, cert, CERT_STORE_ADD_NEW, 0);
		if (! bResult && GetLastError() == CRYPT_E_EXISTS) {
			wxLogMessage(wxT("SeruroCrypto::InstallP12> Warning! certificate exists, replacing."));
			bResult = CertAddCertificateContextToStore(myStore, cert, CERT_STORE_ADD_REPLACE_EXISTING, 0);
			if (! bResult) {
				/* At this point we should also backup and remove any certs added. */
				wxLogMessage(wxT("SeruroCrypto::InstallP12> problem overwriting certificate."));
				//delete [] certName;
				CertCloseStore(myStore, 0);
				CertCloseStore(pfxStore, 0);
				return false;
			}
		} else if (! bResult) {
			/* At this point we should also backup and remove any certs added. */
			wxLogMessage(wxT("SeruroCrypto::InstallP12> unhandled error while adding certificate."));
			//delete [] certName;
			CertCloseStore(myStore, 0);
			CertCloseStore(pfxStore, 0);
			return false;
		}
	}

	//delete [] certName;
	CertCloseStore(myStore, 0);
	CertCloseStore(pfxStore, 0);

	return true;
}

bool SeruroCryptoMSW::HaveIdentity(wxString server_name, wxString address)
{
	/* First get the fingerprint string from the config. */
	if (! wxGetApp().config->HaveIdentity(server_name, address)) return false;
	wxArrayString identity = wxGetApp().config->GetIdentity(server_name, address);

	if (identity.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveIdentity) the identity (%s) (%s) does not have 2 certificates?"),
			server_name, address);
		return false;
	}

	/* Looking at the trusted Root store. */
	bool cert_1 = HaveCertificateByFingerprint(identity[0], CERTIFICATE_STORE_PERSONAL);
	bool cert_2 = HaveCertificateByFingerprint(identity[1], CERTIFICATE_STORE_PERSONAL);
	wxLogMessage(_("SeruroCrypto> (HaveIdentity) identity (%s) (%s) in store: (1: %s, 2: %s)."), 
		server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMSW::HaveCA(wxString server_name)
{ 
	/* First get the fingerprint string from the config. */
	if (! wxGetApp().config->HaveCA(server_name)) return false;
	wxString ca_fingerprint = wxGetApp().config->GetCA(server_name);

	/* Looking at the personal (my) store. */
	bool in_store = HaveCertificateByFingerprint(ca_fingerprint, CERTIFICATE_STORE_TRUSTED_ROOT);
	wxLogMessage(_("SeruroCrypto> (HaveCA) certificate (%s) in store: (%s)."), 
		server_name, (in_store) ? "true" : "false");
	return in_store;
}

bool SeruroCryptoMSW::HaveCertificates(wxString server_name, wxString address)
{ 
	/* First get the fingerprint string from the config. */
	if (! wxGetApp().config->HaveCertificates(server_name, address)) return false;
	wxArrayString identity = wxGetApp().config->GetCertificates(server_name, address);

	if (identity.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveCertificates) the address (%s) (%s) does not have 2 certificates?"),
			server_name, address);
		return false;
	}

	/* Looking at the address book store. */
	bool cert_1 = HaveCertificateByFingerprint(identity[0], CERTIFICATE_STORE_CONTACTS);
	bool cert_2 = HaveCertificateByFingerprint(identity[1], CERTIFICATE_STORE_CONTACTS);
	wxLogMessage(_("SeruroCrypto> (HaveCertificates) address (%s) (%s) in store: (1: %s, 2: %s)."), 
		server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMSW::RemoveIdentity(wxArrayString fingerprints) { return false; }

bool SeruroCryptoMSW::RemoveCA(wxString fingerprint) { return false; }

bool SeruroCryptoMSW::RemoveCertificates(wxArrayString fingerprints) { return false; }

#endif
