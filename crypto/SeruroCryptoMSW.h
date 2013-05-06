
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

	/* wxJSONValue, For easy overloading access. */
	wxString TLSRequest(wxJSONValue params);

	bool InstallP12(wxMemoryBuffer &p12, wxString &password);
};

#endif 

#endif /* __WXMSW__ */