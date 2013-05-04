
/* First thing, detect OS */
#if defined(__WXMAC__)

#include <wx/log.h>

/* Code goes here */
#include "../SeruroConfig.h"
#include "SeruroCryptoMAC.h"

void SeruroCryptoMAC::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MSW> Initialized"));
    
	//TLSRequest(none, 0, verb, object, data); /* SERURO_SECURITY_OPTIONS_DATA */
}

/* Errors should be events. */

bool SeruroCryptoMAC::InstallP12(wxMemoryBuffer &p12, wxString &password)
{
    return false;
}

wxString SeruroCryptoMAC::TLSRequest(wxString p_serverAddress,
    int p_options, wxString p_verb, wxString p_object)
{
	wxString wx_data;
	return TLSRequest(p_serverAddress, p_options, p_verb, p_object, wx_data);
}

wxString SeruroCryptoMAC::TLSRequest(wxString p_serverAddress,
    int p_options, wxString p_verb, wxString p_object, wxString p_data)
{
	wxString responseString("");
    return responseString;
}

#endif