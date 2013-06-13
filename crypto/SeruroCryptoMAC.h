
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
    
	bool InstallP12(wxMemoryBuffer &p12, wxString &password,
        /* Place fingerprints here. */
        wxArrayString &fingerprints);
    
	bool InstallCA(wxMemoryBuffer &ca);
	/* Todo: change name to Certificate. */
	bool InstallCertificate(wxMemoryBuffer &cert);
    
	/* Todo: consider using Install Cert with an optional store name. */
	//bool InstallTLSCert(wxMemoryBuffer &cert);
    
	bool RemoveIdentity(wxString fingerprint);
	bool RemoveCA(wxString fingerprint);
	bool RemoveCertificates(wxArrayString fingerprints);
    
	/* Methods to query certificates by their name (meaning SHA1) */
	bool HaveCA(wxString server_name);
	bool HaveCertificates(wxString server_name, wxString address);
	bool HaveIdentity(wxString server_name, wxString address);
    
	wxString GetFingerprint(wxMemoryBuffer &cert);
};

#endif 

#endif /* __WXMAC__ */