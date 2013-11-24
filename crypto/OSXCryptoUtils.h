
#if defined(__WXMAC__) || defined(__WXOSX__)

#ifndef H_OSXCryptoUtils
#define H_OSXCryptoUtils

#include <wx/string.h>
//#include "../wxJSON/wx/jsonval.h"

/* OSX CFtypes. */
#include <CoreServices/CoreServices.h>

enum search_types_t
{
    CRYPTO_SEARCH_CERT,
    CRYPTO_SEARCH_IDENTITY
};

typedef struct {
    uint8_t size;
    uint8_t oid[3];
    char *value;
} issuer_attr_t;

typedef struct {
    uint8_t attribute_count;
    issuer_attr_t *attributes;
} issuer_t;

//bool GetReferenceFromSubjectKeyID(wxString subject_key, search_types_t type, wxString keychain_name, CFTypeRef *reference);
//bool GetReferenceFromIssuerSerial(wxString issuer, wxString serial, wxString keychain_name, CFTypeRef *reference);

bool FindIssuerSerialInKeychain(wxString issuer, wxString serial, wxString keychain_name);
bool FindSKIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name);
bool DeleteSKIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name);

//bool GetPropertyFromCertificate(const SecCertificateRef &cert, const CFTypeRef &oid, CFDataRef *property);

wxString GetSerialFromCertificate(const SecCertificateRef &cert);
wxString GetIssuerFromCertificate(const SecCertificateRef &cert);
wxString GetSKIDFromCertificate(const SecCertificateRef &cert);

wxString GetSKIDFromIssuerSerial(wxString issuer, wxString serial, wxString keychain_name);
bool GetIssuerSerialFromSKID(const wxString &subject_key, wxString &issuer, wxString &serial, wxString keychain_name);

bool SetTrustPolicy(SecCertificateRef &cert);
bool InstallIdentityToKeychain(const SecIdentityRef &identity, wxString keychain_name);
bool InstallCertificateToKeychain(const wxMemoryBuffer &cert_binary, wxString keychain_name, wxString &fingerprint);


#endif

#endif