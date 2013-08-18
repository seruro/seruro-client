
/* First thing, detect OS */
#if defined(__WXMSW__)

#ifndef H_SeruroCryptoMSW
#define H_SeruroCryptoMSW

#include <wx/string.h>

#include "../wxJSON/wx/jsonval.h"

wxString GetIdentityHashHex(wxString fingerprint);

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
	bool InstallP12(const wxMemoryBuffer &p12, const wxString &password,
		/* Place fingerprints here. */
		wxArrayString &fingerprints);	

	bool InstallCA(const wxMemoryBuffer &ca, wxString &fingerprint);
	/* Todo: change name to Certificate. */
	bool InstallCertificate(const wxMemoryBuffer &cert, 
		wxString &fingerprint);

	/* Todo: consider using Install Cert with an optional store name. */
	//bool InstallTLSCert(wxMemoryBuffer &cert);

	bool RemoveIdentity(wxArrayString fingerprints);
	bool RemoveCA(wxString fingerprint);
	bool RemoveCertificates(wxArrayString fingerprints);

	/* Used for App configurations. */
	bool HaveIdentityByHash(wxString hash);
	wxString GetIdentitySKIDByHash(wxString hash);
	wxString GetIdentityHashBySKID(wxString skid);

	/* Methods to query certificates by their name (meaning SHA1) */
	bool HaveCA(wxString server_name,
		wxString fingerprint = wxEmptyString);
	bool HaveCertificates(wxString server_name, wxString address,
        wxString fingerprint = wxEmptyString);
	bool HaveIdentity(wxString server_name, wxString address,
        wxString fingerprint = wxEmptyString);

	//wxString GetFingerprint(wxMemoryBuffer &cert);
};

#endif 

#endif /* __WXMSW__ */