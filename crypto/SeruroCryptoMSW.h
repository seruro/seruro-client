
/* First thing, detect OS */
#if defined(__WXMSW__)

#ifndef H_SeruroCryptoMSW
#define H_SeruroCryptoMSW

#include <wx/string.h>

#include "../wxJSON/wx/jsonval.h"

class SeruroCryptoMSW
{
public:
	SeruroCryptoMSW() {}
	void OnInit();

	/* wxJSONValue, For easy overloading access. */
	wxString TLSRequest(wxJSONValue params);

	bool InstallP12(wxMemoryBuffer &p12, wxString &password);

	bool InstallCA(wxMemoryBuffer &ca);
	bool InstallCert(wxMemoryBuffer &cert);

	/* Todo: consider using Install Cert with an optional store name. */
	//bool InstallTLSCert(wxMemoryBuffer &cert);

	bool RemoveIdentity(wxString thumbprint);
	bool RemoveCA(wxString thumbprint);
	bool RemoveCerts(wxArrayString thumbprints);
};

#endif 

#endif /* __WXMSW__ */