
/* First thing, detect OS */
#if defined(__WXMSW__)

#include <wx/log.h>

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
BSTR AsLongString(wxString &input)
{
	int size = lstrlenA(input.mb_str(wxConvUTF8));
	BSTR long_object = SysAllocStringLen(NULL, size);
	::MultiByteToWideChar(CP_ACP, 0, input, size, long_object, size);
	return long_object;
}

void AsMultiByte(wxString &input, LPSTR *result)
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

/* Errors should be events. */

bool GetClientCert(HINTERNET hRequest, wxString &p_serverAddress, PCCERT_CONTEXT &p_clientCert);

wxString SeruroCryptoMSW::TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object)
{
	wxString wx_data;
	return TLSRequest(p_serverAddress, p_options, p_verb, p_object, wx_data);
}

/* The TLS Request will assure the server meets the client's requirements for security.
 * We can optionally lower security expectations for TLS and session key size.
 * 
 * Usage guide: 
 *   p_options = (DATA | CLIENT | STRONG | TLS12)
 * The important options for this set of flags are DATA and CLIENT. 
 * DATA: will attach the p_data string to the request, after the headers. 
 *  This is used to send POST data if set. Otherwise it is attached as a header.
 * CLIENT: allow the client to lookup a matching client certificate to a server-provided list of 
 * acceptable client chains (signature chains). If the server requests a client certificate and 
 * the CLIENT flag is not set, the request will fail.
 */
wxString SeruroCryptoMSW::TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object, wxString &p_data)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;

	wxString responseString("");

	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

	/* Get useragent from config */
	BSTR userAgent = AsLongString(SERURO_DEFAULT_USER_AGENT);

	/* Get the server address from config */
	BSTR serverAddress = AsLongString(p_serverAddress);
	BSTR verb = AsLongString(p_verb);
	BSTR object = AsLongString(p_object);
	LPCSTR data = p_data.mb_str();

	/* Create session object. */
	hSession = WinHttpOpen( userAgent, 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	::SysFreeString(userAgent);

	/* Create connection. */
	hConnect = WinHttpConnect(hSession, serverAddress, SERURO_DEFAULT_PORT, 0);
	::SysFreeString(serverAddress);

	wxLogMessage(wxT("SeruroCrypto::TLSRequest> Received, options: %d."), p_options);

	/* Set TLS1.2 only! (...doesn't seem to work) */
	BOOL bResults = FALSE;
	DWORD dwOpt = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1; /* Todo: change me. TLS1_2 */
	bResults = WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS, &dwOpt, sizeof(dwOpt));
	if (! bResults) {
		wxLogMessage("SeruroCrypto::TLSRequest> Cannot set client support TLS1.2.");
		/* Todo: fail if cannot support 1_2 */
	}

	/* Open, and request SSL, defaults to TLSv1.0. */
	hRequest = WinHttpOpenRequest(hConnect, verb, object, NULL, 
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	::SysFreeString(verb);
	::SysFreeString(object);

	/* Calculate length of "data", which comes after heads, if any, add a urlencoded Content-Type. */
	DWORD dwOptionalLength = (p_options & SERURO_SECURITY_OPTIONS_DATA) ? p_data.length() : 0;
	if (dwOptionalLength > 0) {
		WinHttpAddRequestHeaders(hRequest, L"Content-Type: application/x-www-form-urlencoded", (ULONG)-1L,
			WINHTTP_ADDREQ_FLAG_ADD);
	}

	/* Send VERB and OBJECT (if post data exists it will be sent as well). */
	bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		 (LPVOID) data, dwOptionalLength, dwOptionalLength, 0);

	if (! bResults) {
		wxLogMessage(wxT("WinHttpSendResponse error: %s"), wxString::Format(wxT("%d"), GetLastError()));
		if (GetLastError() == ERROR_WINHTTP_SECURE_FAILURE) {
			/* The server certificate is invalid. */
			wxLogMessage("SeruroCrypto::TLSRequest> server certificate verification failed.");
			goto bailout;
		}
	}

	/* Make sure the connection is MAX security. */
	DWORD securitySupport;
	dwSize = sizeof(DWORD);
	bResults = WinHttpQueryOption(hRequest,
		WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID) &securitySupport, &dwSize);

	if (! bResults || !(securitySupport & SECURITY_FLAG_STRENGTH_STRONG)) {
		wxLogMessage("SeruroCrypto::TLSRequest> Server does not support strong security!");
		/* Warning: The server DOES not meet requirements! */
		/* Todo: fail here and report. */
		goto bailout;
	}

	/* As a check, try to set MAX security, if this fails, that is a good thing. */
	/* Consider pinning techniques here. */

	/* End the request. */
	bResults = WinHttpReceiveResponse(hRequest, NULL);

	/* Todo: check for client certificate request. */
	if (! bResults) {
		wxLogMessage(wxT("WinHttpReceiveResponse error: %s"), wxString::Format(wxT("%d"), GetLastError()));
		if (GetLastError() == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED) {
			/* Check if we're expecting. */
			wxLogMessage("SeruroCrypto::TLSRequest> client auth cert needed.");
			/* MSDN says ISSUER list works for Windows 2008/Vista? */
			PCCERT_CONTEXT clientCert = NULL;
			bResults = GetClientCert(hRequest, p_serverAddress, clientCert);
			if (! bResults) {
				/* Could not find a valid TLS client cert. */
				goto bailout;
			}
			/* Set option */
			WinHttpSetOption(hRequest, WINHTTP_OPTION_CLIENT_CERT_CONTEXT, 
				(LPVOID) clientCert, sizeof(CERT_CONTEXT)); /* PCC... is a const pointer. */
			/* Only need to free certificate if one was created/allocated. */
			CertFreeCertificateContext(clientCert);
		} else {
			/* Unhandled error state. */
			goto bailout;
		}
	}

	/* Read response data. */
	do {
		dwSize = 0;
		if (! WinHttpQueryDataAvailable(hRequest, &dwSize)) {
			wxLogMessage(wxT("SeruroCrypt> error in WinHttpReadData."));
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
	wxLogMessage("SeruroCrypto::TLSRequest> Response (TLS) success.");

	/* Call some provided callback (optional). */

	goto finished;

bailout:
	/* Send error event */
	wxLogMessage("SeruroCrypto::TLSRequest> error occured.");

finished:
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return responseString;
}

/* Given a server address, and optional issuer list, fill in clientCert and return a
 * success status if the client cert was found and can be used.
 * This method will:
 * (1) Ask the SeruroConfig for the configured [email] address for the given serverAddress.
 * (2) Use that [email] address to match a subject string for a PKCS#7 certificate.
 * (3) If no certificate are found: try to request the server's issuer list.
 *
 * Note: (3) does not always apply because the TLS certificate the server presents, and it's
 * issuer may be external to the Seruro CA used for S/MIME, thus we support allowing the
 * server application to make TLS client authorization decisions.
 */
bool GetClientCert(HINTERNET hRequest, wxString &p_serverAddress, PCCERT_CONTEXT &p_clientCert)
{
	wxString syncSubject = wxGetApp().config->GetSyncSubjectFromServer(p_serverAddress);
	wxLogMessage(wxT("Got sync subject: \"%s\" from %s."), syncSubject, p_serverAddress);

	BSTR subjectName = AsLongString(syncSubject);

	/* Todo: find appropriate name for certificate store. */
	HCERTSTORE hCertStore = CertOpenSystemStore(0, TEXT("MY"));
	if (! hCertStore) {
		wxLogMessage(wxT("Could not open 'MY' certificate store."));
		return false;
	}

	/* Look up a TLS client cert matching the naming convention inherited by SeruroConfig::GetSyncSubject... */
	p_clientCert = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0,
		CERT_FIND_SUBJECT_STR, (LPVOID) subjectName, NULL);
	::SysFreeString(subjectName);
	if (! p_clientCert) {
		wxLogMessage(wxT("Could not find a certificate matching the subjectName."));
		/* Repeat close, goto statement not needed. */
		CertCloseStore(hCertStore, 0);
		return false;
	}

	CertCloseStore(hCertStore, 0);
	return true;
}

#endif
