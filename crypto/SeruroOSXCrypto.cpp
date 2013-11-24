
/* First thing, detect OS */
#if defined(__WXMAC__) || defined(__WXOSX__)

#include <wx/log.h>
#include <wx/base64.h>
#include <wx/osx/core/cfstring.h>

/* For making HTTP/TLS reqeusts. */
#include <CoreServices/CoreServices.h>
//#include <CFNetwork/CFNetwork.h>
#include <Security/Security.h>

/* Code goes here */
#include "SeruroOSXCrypto.h"
#include "OSXCryptoUtils.h"

#include "../SeruroConfig.h"
#include "../logging/SeruroLogger.h"
#include "../SeruroClient.h"
#include "../api/Utils.h"

#define IDENTITY_KEYCHAIN       "login"
#define CERTIFICATE_KEYCHAIN    "login"
#define CA_KEYCHAIN             "login"

DECLARE_APP(SeruroClient);

void SeruroCryptoMAC::OnInit()
{
	DEBUG_LOG(_("SeruroCrypt::MAC> Initialized"));
}

wxString SeruroCryptoMAC::TLSRequest(wxJSONValue params)
{
	wxString response("");
    bool status;
    CFIndex bytes_read;
    /* Used to read data from stream sychronously. */
    UInt8 *read_buffer;
    /* HTTP response data structure. */
    CFHTTPMessageRef http_response;
    
    /* Set TLS version (hopfully TLS1.2 (fallback to TLS1), fail if not that (???). */
    wxString url_string = wxString::Format(_("https://%s:%d%s"), params["server"]["host"].AsString(),
        params["server"]["port"].AsInt(), params["object"].AsString());
    CFStringRef url_cfstring = CFStringCreateWithCString(kCFAllocatorDefault, 
		AsChar(url_string), kCFStringEncodingMacRoman);
    CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault, url_cfstring, NULL);
    
    /* Get user_agent from config "SERURO_DEFAULT_USER_AGENT" (???) */
	/* Todo: set user agent. */

    /* Set VERB and OBJECT (if post data exists, add.*/
    wxString verb = params["verb"].AsString();
    CFStringRef cfverb = CFStringCreateWithCString(kCFAllocatorDefault, 
		AsChar(verb), kCFStringEncodingMacRoman);
    CFHTTPMessageRef request = CFHTTPMessageCreateRequest(kCFAllocatorDefault,
        cfverb, url, kCFHTTPVersion1_1);
    
    wxString post_data_string;
    CFDataRef post_data;
    if (params["flags"].AsInt() & SERURO_SECURITY_OPTIONS_DATA) {
        wxLogMessage(wxT("SeruroCrypto::TLS> POST, must add headers."));
        CFHTTPMessageSetHeaderFieldValue(request,
            CFSTR("Content-Type"), CFSTR("application/x-www-form-urlencoded"));
        /* Todo: cleanup possible auth data in p_data parameter. */
        post_data_string = params["data_string"].AsString();
        post_data = CFDataCreate(kCFAllocatorDefault,
            (UInt8 *) AsChar(post_data_string), post_data_string.length());
        CFHTTPMessageSetBody(request, post_data);
    }
    
    /* Add request headers "Accept: application/json", calculate length. */
    CFHTTPMessageSetHeaderFieldValue(request, CFSTR("Accept"), CFSTR("application/json"));
    CFHTTPMessageSetHeaderFieldValue(request, CFSTR("User-Agent"), CFSTR(SERURO_DEFAULT_USER_AGENT));
    
    /* This will make a copy of the request, 
     * which can be released afterward (this will open the request!). */
    CFReadStreamRef read_stream = CFReadStreamCreateForHTTPRequest(kCFAllocatorDefault, request);
    status = CFReadStreamOpen(read_stream);
    
    /* See listing 3-2 for releasing data. */
    CFRelease(request);
    CFRelease(cfverb);
    CFRelease(url_cfstring);
    
    if (! status) {
        wxLogMessage(wxT("SeruroCrypto::TLS> could not open stream."));
        goto bailout;
    }
    
    /* Check server certificate, or only accept valid certificates. */
    /* Check that security is set to MAX. */
    
    /* Check for unhandled error states. */
    UInt64 response_code;
    CFDataRef http_response_body;

    /* Read data (sync, meaning no run loop). */
    bytes_read = 0;
    do {
        read_buffer = (UInt8 *) malloc(256 * sizeof(UInt8));
        bzero(read_buffer, 256);
        /* Todo: If there is a proxy with a dead backend this is hang. */
        bytes_read = CFReadStreamRead(read_stream, read_buffer, 256);
        
        /* A problem occured. */
        if (bytes_read < 0) break;
        
        response = response + wxString::FromAscii(read_buffer, bytes_read);
        /* Free allocated data. */
        delete read_buffer;
    } while (bytes_read != 0);
    
    /* Nothing to release yet (http_request is open). */
    if (bytes_read < 0) {
        wxLogMessage(wxT("SeruroCrypto::TLS> problem reading from stream."));
        goto bailout;
    }
    
    /* Create response (HTTP message) data structure. Use read_buffer again (for UInt8 conversion). */
    read_buffer = (UInt8 *) malloc(response.length() * sizeof(UInt8));
    memcpy(read_buffer, AsChar(response), response.length() * sizeof(UInt8));
    
    http_response = CFHTTPMessageCreateEmpty(kCFAllocatorDefault, false);
    status = CFHTTPMessageAppendBytes(http_response, read_buffer, response.length());
    delete read_buffer;
    
    /* Check the response code, expecting 200, but 401, 301 are acceptable. */
    response_code = CFHTTPMessageGetResponseStatusCode(http_response);
    wxLogMessage(wxT("SeruroCrypto::TLS> response code (%d)"), response_code);
    
    /* Now copy only the body data. */
    http_response_body = CFHTTPMessageCopyBody(http_response);
    if (http_response_body == NULL || CFDataGetLength(http_response_body) == 0) {
        /* There was no http body? */
        ERROR_LOG(_("SeruroCrypto> (TLSRequest) could not parse HTTP Response body."));
        goto bailout;
    }
    
    read_buffer = (UInt8 *) malloc(CFDataGetLength(http_response_body) * sizeof(UInt8));
    CFDataGetBytes(http_response_body, CFRangeMake(0, CFDataGetLength(http_response_body)), read_buffer);
    
    /* Finally convert to wxString */
    response = wxString::FromAscii((char *) read_buffer,
        CFDataGetLength(http_response_body));
    
    /* Clean up */
    CFRelease(http_response_body);
    delete read_buffer;
    
    wxLogMessage(wxT("SeruroCrypto::TLS> read body: %s"), response);
    wxLogMessage(wxT("SeruroCrypto::TLS> Response (TLS) success."));
    
    goto finished;
    
bailout:
    wxLogMessage(wxT("SeruroCrypto::TLS> error occured."));
    
finished:
    CFReadStreamClose(read_stream);
    CFRelease(read_stream);
    /* Todo, cleanup possible auth data in request. */
    
    return response;
}

/* Errors should be events. */

bool SeruroCryptoMAC::InstallP12(const wxMemoryBuffer &p12, const wxString &password, wxArrayString &fingerprints)
{
    OSStatus success;
    CFDataRef p12_data;
    CFStringRef password_data;
    
    /* Set the password as an option. */
    CFMutableDictionaryRef options;

    password_data = CFStringCreateWithCString(kCFAllocatorDefault, AsChar(password), kCFStringEncodingASCII);
    
    options = CFDictionaryCreateMutable(kCFAllocatorDefault, 3,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(options, kSecImportExportPassphrase, password_data);
    CFRelease(password_data);
    
    /* Convert type of p12. */
    p12_data = CFDataCreate(kCFAllocatorDefault,
        (UInt8 *) p12.GetData(), p12.GetDataLen());
    
    CFArrayRef p12_items;
    /* Perform import. */
    success = SecPKCS12Import(p12_data, options, &p12_items);
    
    /* Release used objects. */
    CFRelease(options);
    CFRelease(p12_data);
    
    if (success == errSecDecode) {
        wxLogMessage(_("SeruroCrypto> (InstallP12) could not decode p12."));
        return false;
    }
    
    if (success == errSecAuthFailed) {
        wxLogMessage(_("SeruroCrypto> (InstallP12) auth failed or corrupted p12 data."));
        return false;
    }
    
    if (success != errSecSuccess || p12_items == NULL) {
        wxLogMessage(_("SeruroCrypto> (InstallP12) no p12 items found (err=%d)."), success);
        return false;
    }

    /* The import function will have a list of items. */
    CFDictionaryRef item;
    SecIdentityRef identity;
    SecCertificateRef identity_cert;
    int identity_count = CFArrayGetCount(p12_items);
    
    wxLogMessage(_("SeruroCrypto> (InstallP12) found (%d) identities."), identity_count);
    
    /* Iterate through all p12 items. */
    for (int i = 0; i < identity_count; i++) {
        item = (CFDictionaryRef) CFArrayGetValueAtIndex(p12_items, i);
        
        /* Install P12s to keychain. */
        identity = (SecIdentityRef) CFDictionaryGetValue(item, kSecImportItemIdentity);
        
        if (! InstallIdentityToKeychain(identity, _(IDENTITY_KEYCHAIN))) {
            return false;
        }
        
        wxLogMessage(_("SeruroCrypto> (InstallP12) successfully installed identity (%d)."), i);
        
        /* Add fingerprint from the identity certificate (the persistent keychain reference). */
        SecIdentityCopyCertificate(identity, &identity_cert);
        fingerprints.Add(GetSKIDFromCertificate(identity_cert));
        CFRelease(identity_cert);
    }
    
    /* Release array of dictionaries. */
    if (CFArrayGetCount(p12_items) > 0) {
        /* All item from the store are held by the arrey store. */
    }
    CFRelease(p12_items);
    
    return true;
}

bool SeruroCryptoMAC::InstallCA(const wxMemoryBuffer &ca, wxString &fingerprint)
{
    bool status;
    status = InstallCertificateToKeychain(ca, _(CA_KEYCHAIN), fingerprint);
    return status;
}

bool SeruroCryptoMAC::InstallCertificate(const wxMemoryBuffer &cert, wxString &fingerprint)
{
    bool status;
    status = InstallCertificateToKeychain(cert, _(CERTIFICATE_KEYCHAIN), fingerprint);
    return status;
}

bool SeruroCryptoMAC::RemoveIdentity(wxArrayString fingerprints)
{
    bool status;
    
    for (size_t i = 0; i < fingerprints.size(); i++) {
        status = DeleteSKIDInKeychain(fingerprints[i], CRYPTO_SEARCH_IDENTITY, _(IDENTITY_KEYCHAIN));
        if (! status) return false;
    }
    return true;
}

bool SeruroCryptoMAC::RemoveCertificates(wxArrayString fingerprints)
{
    bool status;
    
    for (size_t i = 0; i < fingerprints.size(); i++) {
        status = DeleteSKIDInKeychain(fingerprints[i], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
        if (! status) return false;
    }
    return true;
}

bool SeruroCryptoMAC::RemoveCA(wxString fingerprint)
{
    bool status;
    
    status = DeleteSKIDInKeychain(fingerprint, CRYPTO_SEARCH_CERT, _(CA_KEYCHAIN));
    return status;
}

/* Methods to query certificates by their name (meaning SHA1) */
bool SeruroCryptoMAC::HaveCA(wxString server_name, wxString fingerprint)
{
    bool status;
	wxString ca_fingerprint;
    
	if (fingerprint.compare(wxEmptyString) == 0) {
		if (! theSeruroConfig::Get().HaveCA(server_name)) return false;
		ca_fingerprint = theSeruroConfig::Get().GetCA(server_name);
	} else {
		ca_fingerprint = fingerprint;
	}
    
    status = FindSKIDInKeychain(ca_fingerprint, CRYPTO_SEARCH_CERT, _(CA_KEYCHAIN));
    
    wxLogMessage(_("SeruroCrypto> (HaveCA) server (%s) in store: (%s)."), server_name, (status) ? _("true") : _("false"));
    return status;
}

bool SeruroCryptoMAC::HaveCertificates(wxString server_name, wxString address, wxString fingerprint)
{
    /* Overview: since OSX cannot search a certificate using it's fingerprint (SHA1).
     *   First find the CA certificate by matching SHA1 over all certificates in the given
     *   keychain. Then search all certificates matching the issues of that CA (and SHA1).
     */
    
    /* First get the fingerprint string from the config. */
	if (! theSeruroConfig::Get().HaveCertificates(server_name, address)) return false;
	wxArrayString certificates = theSeruroConfig::Get().GetCertificates(server_name, address);
    
	if (certificates.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveCertificates) the address (%s) (%s) does not have 2 certificates?"),
                     server_name, address);
		return false;
	}
    
	/* Looking at the address book store. */
	bool cert_1 = FindSKIDInKeychain(certificates[0], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
	bool cert_2 = FindSKIDInKeychain(certificates[1], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
	wxLogMessage(_("SeruroCrypto> (HaveCertificates) address (%s) (%s) in store: (1: %s, 2: %s)."),
                 server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMAC::HaveIdentity(wxString server_name, wxString address, wxString fingerprint)
{
    wxArrayString identity;
    
	/* First get the fingerprint string from the config. */
    if (fingerprint.compare(wxEmptyString) == 0) {
        identity = theSeruroConfig::Get().GetIdentity(server_name, address);
        if (identity.size() == 0) return false;
    } else {
        identity.Add(fingerprint);
    }

	/* Looking at the trusted Root store. */
    for (size_t i = 0; i < identity.size(); i++) {
        DEBUG_LOG(_("SeruroCrypto> (HaveIdentity) checking for fingerprint (%s)."), identity[i]);
        if (! FindSKIDInKeychain(identity[i], CRYPTO_SEARCH_CERT, _(IDENTITY_KEYCHAIN))) {
            /* Todo: not use CRYPTO_SEARCH_IDENTITY */
            return false;
        }
    }
    
    return true;
}

bool SeruroCryptoMAC::HasIdentityIssuer(const wxString &encoded_subject, const wxString &encoded_serial)
{
    /* A simple, textually helpful wrapper to find a certificate by a subject and serial. */
    return FindIssuerSerialInKeychain(encoded_subject, encoded_serial, _(IDENTITY_KEYCHAIN));
}

wxString SeruroCryptoMAC::GetIdentityIssuerSKID(const wxString &encoded_subject, const wxString &encoded_serial)
{
    return GetSKIDFromIssuerSerial(encoded_subject, encoded_serial, _(IDENTITY_KEYCHAIN));
}

bool SeruroCryptoMAC::GetSKIDIdentityIssuer(const wxString &fingerprint, wxString &issuer, wxString &serial)
{
    return GetIssuerSerialFromSKID(fingerprint, issuer, serial, _(IDENTITY_KEYCHAIN));
}


#endif