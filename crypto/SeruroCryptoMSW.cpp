
/* First thing, detect OS */
#if defined(__WXMSW__)

#include <wx/log.h>

#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp")

/* Code goes here */
#include "SeruroCryptoMSW.h"

void SeruroCryptoMSW::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MSW> Initialized"));
}

void TLSRequest()
{
	//DWORD dwSize = 0;
	//DWORD dwDownloaded = 0;
	//LPSTR pszOutBuffer;

	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;

	hSession = WinHttpOpen( L"SeruroClient/1.0", 
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	hConnect = WinHttpConnect(hSession, L"valdrea.com", INTERNET_DEFAULT_HTTP_PORT, 0);
	hRequest = WinHttpOpenRequest(hConnect, L"GET", NULL, NULL, 
		WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

}

#endif
