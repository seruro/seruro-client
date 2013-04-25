
/* First thing, detect OS */
#if defined(__WXMSW__)

#ifndef H_SeruroCryptoMSW
#define H_SeruroCryptoMSW

#include <wx/string.h>

class SeruroCryptoMSW
{
public:
	SeruroCryptoMSW() {}
	void OnInit();

	/* For easy overloading access. */
	wxString SeruroCryptoMSW::TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object);
	wxString TLSRequest(wxString &p_serverAddress, 
		int p_options, wxString &p_verb, wxString &p_object, 
		wxString &p_data);

	bool InstallP12(wxMemoryBuffer &p12, wxString &password);
};

#endif 

#endif /* __WXMSW__ */