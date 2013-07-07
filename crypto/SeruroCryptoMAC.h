
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
    
	bool InstallP12(const wxMemoryBuffer &p12, const wxString &password,
        /* Place fingerprints here. */
        wxArrayString &fingerprints);
    
	bool InstallCA(const wxMemoryBuffer &ca, wxString &fingerprint);
	/* Todo: change name to Certificate. */
	bool InstallCertificate(const wxMemoryBuffer &cert, wxString &fingerprint);
    
	/* Todo: consider using Install Cert with an optional store name. */
	//bool InstallTLSCert(wxMemoryBuffer &cert);
    
	bool RemoveIdentity(wxArrayString fingerprints);
	bool RemoveCA(wxString fingerprint);
	bool RemoveCertificates(wxArrayString fingerprints);
    
	/* Methods to query certificates by their name (meaning SHA1) */
	bool HaveCA(wxString server_name);
	bool HaveCertificates(wxString server_name, wxString address);
	bool HaveIdentity(wxString server_name, wxString address);
    
	//wxString GetFingerprint(wxMemoryBuffer &cert);
};

#endif 

#endif /* __WXMAC__ */