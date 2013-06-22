
/* First thing, detect OS */
#if defined(__WXMAC__)

#include <wx/log.h>
#include <wx/base64.h>
#include <wx/wx.h>

/* Note: requires C++ flags: "-framework Security" */
//#include <Security/Security.h>
//#include <netinet/ip.h>
//#include <netdb.h>
//#include <sys/socket.h>

/* For making HTTP/TLS reqeusts. */
#include <CFNetwork/CFNetwork.h>
#include <Security/Security.h>
/* For certificate/keychain import. */
#include <Security/SecCertificate.h>
/* For hashing/fingerprinting (SHA1). */
#include <CommonCrypto/CommonDigest.h>
/* For reading p12 data. */
#include <Security/SecImportExport.h>
/* For adding identity to keychain. */
#include <Security/SecItem.h>

/* Code goes here */
#include "../SeruroConfig.h"
#include "SeruroCryptoMAC.h"

#define IDENTITY_KEYCHAIN       "login"
#define CERTIFICATE_KEYCHAIN    "login"
#define CA_KEYCHAIN             "System Roots"

const char* AsChar(wxString &input)
{
	return input.mb_str(wxConvUTF8);
}

wxString GetFingerprintFromBuffer(wxMemoryBuffer &cert)
{
    /* Documentation missing from offline view. */
    CC_SHA1_CTX hash_ctx;
    CC_SHA1_Init(&hash_ctx);
    
    /* Add contents of memory buffer. */
    CC_SHA1_Update(&hash_ctx, (const void *)cert.GetData(), (CC_LONG)cert.GetDataLen());
    
    /* Store byte-data of hash in character buffer. */
    unsigned char hash_digest[CC_SHA1_DIGEST_LENGTH];
    CC_SHA1_Final(hash_digest, &hash_ctx);
    
    /* Store in memory buffer. */
    wxMemoryBuffer hash_buffer;
    hash_buffer.AppendData( (void *) hash_digest, 20);
    
    /* Todo: (Optional), zero out digest buffer. */
    
    return wxBase64Encode(hash_buffer);
}

wxString GetFingerprintFromIdentity(SecIdentityRef &identity)
{
    /* The certificate is extracted from the identity to find the fingerprint. */
    SecCertificateRef identity_cert;
    CFDataRef cert_data;
    wxMemoryBuffer cert_buffer;
    
    /* Place the fingerprint belonging to the identity into result fingerprints. */
    SecIdentityCopyCertificate(identity, &identity_cert);
    cert_data = SecCertificateCopyData(identity_cert);
    
    cert_buffer.AppendData(CFDataGetBytePtr(cert_data), CFDataGetLength(cert_data));
    
    /* Release created objects. */
    CFRelease(identity_cert);
    CFRelease(cert_data);
    
    return GetFingerprintFromBuffer(cert_buffer);
}

bool InstallIdentityToKeychain(SecIdentityRef &identity, wxString keychain_name)
{
    //kSecUseKeychain
    OSStatus success;
    /* Todo: implement keychain access. */
    SecKeychainRef keychain = NULL;

    /* Find the identity item, add it to a dictionary, add it to the keychain. */
    CFMutableDictionaryRef identity_item;
    identity_item = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    //key = CFStringGetCStringPtr(kSecValueRef, kCFStringEncodingASCII);
    //CFDictionarySetValue(identity_item, (const void *) key, (const void *) itentity);
    CFDictionarySetValue(identity_item, kSecClass, kSecClassIdentity);
    CFDictionarySetValue(identity_item, kSecValueRef, (const void *) identity);
    CFDictionarySetValue(identity_item, kSecUseKeychain, (const void *) keychain);
    
    /* Add the identity to the key chain, without error handling. */
    success = SecItemAdd(identity_item, NULL);
    
    /* Release. */
    CFRelease(identity_item);
    
    if (success != errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (InstallP12) could not add identity to keychain."));
        return false;
    }
    return true;
}

//from MSW: InstallCertToStore (wxMemoryBuffer &cert, wxString store_name)
bool InstallCertificateToKeychain(wxMemoryBuffer &cert_binary, wxString keychain_name)
{
    /* By default all certificates go to the default (login) keychain. */
    SecKeychainRef keychain = NULL;
    /* Todo: add to specific keychain if name is given. */
    
    SecCertificateRef certificate;
    CFDataRef cert_data;
    
    /* The memory buffer will hold the DER in byte form. */
    cert_data = CFDataCreate(kCFAllocatorDefault,
        (UInt8 *) cert_binary.GetData(), cert_binary.GetDataLen());
    certificate = SecCertificateCreateWithData(kCFAllocatorDefault, cert_data);
    
    /* SecCertificateRef SecCertificateCreateWithData ( //from: SecCertificate.h
     *   CFAllocatorRef allocator, //NULL for the default allocator?
     *   CFDataRef data); //a DER-encoded representation of x509
     * Notes: SecCertificateRef will be NULL if the data is not formatted correctly
     */
    
    if (certificate == NULL) {
        wxLogMessage(_("SeruroCrypto> (InstallToKeychain) certificate is null."));
        return false;
    }
    
    /* OSStatus SecCertificateAddToKeychain ( //from: SecCertificate.h
     *   SecCertificateRef certificate, //cert object
     *   SecKeychainRef keychain); //NULL to add to the default keychain
     * Notes: if the certificate already exists in the keychain then:
     * errSecDuplicateItem (–25299)
     */
    
    OSStatus success;
    /* Todo: there's a more generic SecItemAdd, which is avilable in iOS. */
    //success = SecCertificateAddToKeychain(certificate, keychain);

    /* Find the identity item, add it to a dictionary, add it to the keychain. */
    CFMutableDictionaryRef cert_item;
    cert_item = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(cert_item, kSecClass, kSecClassCertificate);
    CFDictionarySetValue(cert_item, kSecValueRef, (const void *) certificate);
    CFDictionarySetValue(cert_item, kSecUseKeychain, (const void *) keychain);
    
    /* Add to keychain. */
    success = SecItemAdd(cert_item, NULL);
    
    /* Release objects. */
    CFRelease(certificate);
    CFRelease(cert_data);
    CFRelease(cert_item);
    
    if (success == errSecDuplicateItem) {
        wxLogMessage(_("SeruroCrypto> (InstallToKeychain) duplicate certificate detected."));
        return true;
    }
    
    if (success != errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (InstallToKeychain) error (%d)."), success);
        return false;
    }
    return true;
}

void SeruroCryptoMAC::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MAC> Initialized"));
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
    wxString url_string = wxString(wxT("https://") +
        params["server"]["host"].AsString() +
        wxT(":") + params["server"]["port"].AsString() +
		params["object"].AsString());
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
        bytes_read = CFReadStreamRead(read_stream, read_buffer, 246);
        
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

bool SeruroCryptoMAC::InstallP12(wxMemoryBuffer &p12,
    wxString &password, wxArrayString &fingerprints)
{
    /* OSStatus SecPKCS12Import (
     *   CFDataRef pkcs12_data, //p12 data
     *   CFDictionaryRef options, //options?
     *   CFArrayRef *items); //array of two dictionaries, identities and certificates
     * Notes: should return SecIdentityRef and SecCertificateRef. 
     * Errors: (errSecDecode) if p12 is bad, ( errSecAuthFailed) is password is bad,
     *   or the data in the p12 was damaged.
     *
     * Options: extern CFStringRef kSecImportExportPassphrase; (CFStringRef)
     *   extern CFStringRef kSecImportExportKeychain; (SecKeychainRef)
     *   extern CFStringRef kSecImportExportAccess; (SecAccessRef)
     *
     * To use: (1) iterate through the returned CFArrayRef (each dictionary)
     *   will have (keys: kSecImportItemIdentity, kSecImportItemCertChain)
     *   (2) first iterate through the cert_chain, of CFArrayRef type (SecCertificateRef)
     *   (3) then import the identity of SecIdentityRef type
     */
    
    /* Todo: what if kSecImportExportAccess is missing? */
    
    OSStatus success;
    CFDataRef p12_data;
    const char *key;
    const char *password_data;
    
    /* Set the password as an option. */
    CFMutableDictionaryRef options;
    key = CFStringGetCStringPtr(kSecImportExportPassphrase, kCFStringEncodingASCII);
    password_data = AsChar(password);
    
    options = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(options, (const void *) key, (const void *) password_data);
    
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
    
    //wxString keychain_name = _(IDENTITY_KEYCHAIN);
    /* The import function will have a list of items. */
    CFDictionaryRef item;
    SecIdentityRef identity;
    /* Iterate through all p12 items. */
    for (int i = CFArrayGetCount(p12_items)-1; i >= 0; i--) {
        item = (CFDictionaryRef) CFArrayGetValueAtIndex(p12_items, i);
        
        /* Install P12s to keychain. */
        key = CFStringGetCStringPtr(kSecImportItemIdentity, kCFStringEncodingASCII);
        identity = (SecIdentityRef) CFDictionaryGetValue(item, (const void *) key);
        
        if (! InstallIdentityToKeychain(identity, _(IDENTITY_KEYCHAIN))) {
            return false;
        }
        
        /* Add fingerprint from the identity certificate. */
        fingerprints.Add(GetFingerprintFromIdentity(identity));
    }
    
    /* Release array of dictionaries. */
    if (CFArrayGetCount(p12_items) > 0) {
        /* All item from the store are held by the arrey store. */
        //CFRelease(item);
    }
    CFRelease(p12_items);
    //CFRelease(identity_item);
    
    return true;
}

bool SeruroCryptoMAC::InstallCA(wxMemoryBuffer &ca)
{
    return InstallCertificateToKeychain(ca, _(CA_KEYCHAIN));
}

bool SeruroCryptoMAC::InstallCertificate(wxMemoryBuffer &cert)
{
    return InstallCertificateToKeychain(cert, _(CERTIFICATE_KEYCHAIN));
}

bool SeruroCryptoMAC::RemoveIdentity(wxString fingerprint) { return true; }
bool SeruroCryptoMAC::RemoveCA(wxString fingerprint) { return true; }
bool SeruroCryptoMAC::RemoveCertificates(wxArrayString fingerprints)
{ return true; }

bool FindFingerprintInKeychain(wxString &fingerprint, wxString keychain_name)
{
    // 1. Get SecKeychainRef
    // (item class is certificate, attr-list does not allow SHA1?)
    
    /* Todo: this is deprecated. */
    /* OSStatus SecKeychainSearchCreateFromAttributes (
     *   CFTypeRef keychainOrArray, // can be a SecKeychainRef
     *   SecItemClass itemClass, //kSecCertificateItemClass
     *   const SecKeychainAttributeList *attrList,
     *   SecKeychainSearchRef *searchRef);
     */
    
    // Use SecKeychainSearchCopyNext to get a keychain item
    // Use  SecKeychainItemCopyAttributesAndData to get item data
    // Calculate SHA1
    // Compare
    
    return true;
}

/* Methods to query certificates by their name (meaning SHA1) */
bool SeruroCryptoMAC::HaveCA(wxString server_name)
{
    if (! wxGetApp().config->HaveCA(server_name)) return false;
	wxString ca_fingerprint = wxGetApp().config->GetCA(server_name);
    
    return FindFingerprintInKeychain(ca_fingerprint, _(CA_KEYCHAIN));
}
bool SeruroCryptoMAC::HaveCertificates(wxString server_name, wxString address)
{
    /* Overview: since OSX cannot search a certificate using it's fingerprint.
     *   First find the CA certificate by matching SHA1 over all certificates in the given
     *   keychain. Then search all certificates matching the issues of that CA (and SHA1).
     */
    
    /* First get the fingerprint string from the config. */
	if (! wxGetApp().config->HaveCertificates(server_name, address)) return false;
	wxArrayString certificates = wxGetApp().config->GetCertificates(server_name, address);
    
	if (certificates.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveCertificates) the address (%s) (%s) does not have 2 certificates?"),
                     server_name, address);
		return false;
	}
    
	/* Looking at the address book store. */
	bool cert_1 = FindFingerprintInKeychain(certificates[0], _(CERTIFICATE_KEYCHAIN));
	bool cert_2 = FindFingerprintInKeychain(certificates[1], _(CERTIFICATE_KEYCHAIN));
	wxLogMessage(_("SeruroCrypto> (HaveCertificates) address (%s) (%s) in store: (1: %s, 2: %s)."),
                 server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMAC::HaveIdentity(wxString server_name, wxString address)
{
	/* First get the fingerprint string from the config. */
	if (! wxGetApp().config->HaveIdentity(server_name, address)) return false;
	wxArrayString identity = wxGetApp().config->GetIdentity(server_name, address);
    
	if (identity.size() != 2) {
		wxLogMessage(_("SeruroCrypto> (HaveIdentity) the identity (%s) (%s) does not have 2 certificates?"),
                     server_name, address);
		return false;
	}
    
	/* Looking at the trusted Root store. */
	bool cert_1 = FindFingerprintInKeychain(identity[0], _(IDENTITY_KEYCHAIN));
	bool cert_2 = FindFingerprintInKeychain(identity[1], _(IDENTITY_KEYCHAIN));
	wxLogMessage(_("SeruroCrypto> (HaveIdentity) identity (%s) (%s) in store: (1: %s, 2: %s)."),
                 server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

wxString SeruroCryptoMAC::GetFingerprint(wxMemoryBuffer &cert)
{
    return GetFingerprintFromBuffer(cert);
}


#endif