
/* First thing, detect OS */
#if defined(__WXMAC__)

#ifndef H_SeruroCryptoMAC
#define H_SeruroCryptoMAC

#include "../wxJSON/wx/jsonval.h"

#include <wx/string.h>

class SeruroCryptoMAC
{
public:
	SeruroCryptoMAC() {}
	void OnInit();
    
	/* wxJSONValue For easy overloading access. */
	wxString TLSRequest(wxJSONValue params);
    
	bool InstallP12(wxMemoryBuffer &p12, wxString &password);
};

#endif 

#endif /* __WXMAC__ */