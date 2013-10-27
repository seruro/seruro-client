
/* First thing, detect OS */
#if defined(__WXMSW__)

#ifndef H_SeruroCryptoMSW
#define H_SeruroCryptoMSW

#include <wx/string.h>

#include "../wxJSON/wx/jsonval.h"

#define CERTSTORE_TRUSTED_ROOT "Root"
#define CERTSTORE_CONTACTS	   "AddressBook"
#define CERTSTORE_PERSONAL	   "My"

/* Given an addres/hash:
 * (1) Check if the hash is installed, (2) if it belongs to the address. */
bool IsHashInstalledAndSet(wxString address, wxMemoryBuffer hash);
/* Return the SHA1 hex presentation of a SKID fingerprint. */
wxString GetIdentityHashHex(wxString fingerprint);

/* The windows store uses SHA1s to search. */
enum search_type_t {
	BY_HASH,
	BY_SKID
};

/* Internal store searching functions used by applications. */
wxString GetFingerprintFromCertificate(PCCERT_CONTEXT &cert, 
	search_type_t match_type);
bool HaveCertificateByFingerprint(wxString fingerprint, wxString store_name, 
	search_type_t match_type = BY_SKID);
PCCERT_CONTEXT GetCertificateByFingerprint(wxString fingerprint, wxString store_name, 
	search_type_t match_type);

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