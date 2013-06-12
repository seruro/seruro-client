
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

	/* Server name has been recently added to allow identity tracking
	 * in the case of duplicate certificates.
	 * 
	 * Identities could also be removed by enumerating from the 
	 * CA (as the issuer), but a fingerprint is most flexible w.r.t.
	 * future development allowing for arbitrary (to the max supported)
	 * intermediary CAs. 
	 */
	bool InstallP12(wxMemoryBuffer &p12, wxString &password,
		/* Place fingerprints here. */
		wxArrayString &fingerprints);	

	bool InstallCA(wxMemoryBuffer &ca);
	bool InstallCert(wxMemoryBuffer &cert);

	/* Todo: consider using Install Cert with an optional store name. */
	//bool InstallTLSCert(wxMemoryBuffer &cert);

	bool RemoveIdentity(wxString fingerprint);
	bool RemoveCA(wxString fingerprint);
	bool RemoveCerts(wxArrayString fingerprints);

	/* Methods to query certificates by their name (meaning SHA1) */
	bool HasCA(wxString server_name);
	bool HasCerts(wxString server_name, wxString address);
	bool HasIdentity(wxString server_name, wxString address);

	wxString GetFingerprint(wxMemoryBuffer &cert);
};

#endif 

#endif /* __WXMSW__ */