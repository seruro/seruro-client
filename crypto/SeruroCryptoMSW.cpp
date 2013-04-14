
/* First thing, detect OS */
#if defined(__WXMSW__)

#include <wx/log.h>

#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp")
#pragma comment(lib, "crypt32")

/* Code goes here */
#include "../SeruroConfig.h"
#include "SeruroCryptoMSW.h"

/* Helper function to convert wxString to a L, the caller is responsible for memory. */
BSTR AsLongString(wxString &input)
{
	int size = lstrlenA(input.mb_str(wxConvUTF8));
	BSTR long_object = SysAllocStringLen(NULL, size);
	::MultiByteToWideChar(CP_ACP, 0, input, size, long_object, size);
	return long_object;
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
   This is used to send POST data if set. Otherwise it is attached as a header.
 * CLIENT: allow the client to lookup a matching client certificate to a server-provided list of 
 * acceptable client chains (signature chains). If the server requests a client certificate and the CLIENT
 * flag is not set, the request will fail.
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
	BSTR data = AsLongString(p_data);

	/* Create session object. */
	hSession = WinHttpOpen( userAgent, 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	::SysFreeString(userAgent);

	/* Create connection. */
	hConnect = WinHttpConnect(hSession, serverAddress, SERURO_DEFAULT_PORT, 0);
	::SysFreeString(serverAddress);

	/* Set TLS1.2 only! (doesn't seem to work) */
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

	/* Todo: check for requirement to add Data, Nonce, or other header (DATA not set) */

	/* Now the request can be sent, attach additional headers or POST data. */
	/* Todo: check that verb = PUT | POST? */
	DWORD dwOptionalLength = (p_options & SERURO_SECURITY_OPTIONS_DATA) ? p_data.length() : 0;
	if (dwOptionalLength > 0) {
		wxLogMessage("SeruroCrypto::TLSRequest> Sending optional POST data.");
	}
	bResults = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		(LPVOID) data, dwOptionalLength, dwOptionalLength, 0);

	/* HttpQuery structures of interest:
	 * http://msdn.microsoft.com/en-us/library/windows/desktop/aa384066(v=vs.85).aspx
	 *  WINHTTP_OPTION_SECURE_PROTOCOLS (unsigned long int)
	 *    (WINHTTP_FLAG_SECURE_PROTOCOL_ALL, WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2)
	 *  WINHTTP_OPTION_SECURE_CERTIFICATE_STRUCT (WINHTTP_CERTIFICATE_INFO struct)
	 *  WINHTTP_OPTION_SECURITY_FLAGS (unsigned long int)
	 *    (SECURITY_FLAG_STRENGTH_STRONG | SECURITY_FLAG_SECURE)
	 *  WINHTTP_OPTION_CLIENT_CERT_ISSUER_LIST
	 *  WINHTTP_OPTION_CLIENT_CERT_CONTEXT
	 */

	/* Todo: check for client certificate request. */

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

	/* Check for failure */
	//wxLogMessage(wxT("%s"), wxString::Format(wxT("%d"), GetLastError()));

	/* Consider pinning techniques here. */

	/* End the request. */
	bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (!bResults) {
		wxLogMessage(wxT("%s"), wxString::Format(wxT("%d"), GetLastError()));
		if (GetLastError() == ERROR_WINHTTP_SECURE_FAILURE) {
			wxLogMessage("SeruroCrypto::TLSRequest> server certificate failed.");
			goto bailout;
		}
		if (GetLastError() == ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED) {
			/* Check if we're expecting. */
			wxLogMessage("SeruroCrypto::TLSRequest> client auth cert needed.");
		}
		goto bailout;
	}

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
				//wxLogMessage(wxString::Format(wxT("%s"), pszOutBuffer));
				responseString = responseString + wxString::FromAscii(pszOutBuffer, dwDownloaded);
			}
			delete [] pszOutBuffer;
		}
	} while (dwSize > 0);
	//wxLogMessage(wxT("%s"), output);
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

#endif
