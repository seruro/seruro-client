
/* First thing, detect OS */
#if defined(__WXMAC__)

#ifndef H_SeruroCryptoMAC
#define H_SeruroCryptoMAC

#include <wx/string.h>

class SeruroCryptoMAC
{
public:
	SeruroCryptoMAC() {}
	void OnInit();
    
	/* For easy overloading access. */
	wxString TLSRequest(wxString p_serverAddress,
        int p_options, wxString p_verb, wxString p_object);
	wxString TLSRequest(wxString p_serverAddress,
        int p_options, wxString p_verb, wxString p_object, wxString p_data);
    
	bool InstallP12() {return false;}
};

#endif 

#endif /* __WXMAC__ */