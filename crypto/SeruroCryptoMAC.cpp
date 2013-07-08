
/* First thing, detect OS */
#if defined(__WXMAC__)

#include <wx/log.h>
#include <wx/base64.h>
#include <wx/osx/core/cfstring.h>


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
#include "../SeruroClient.h"

#define IDENTITY_KEYCHAIN       "login"
#define CERTIFICATE_KEYCHAIN    "login"
//#define CA_KEYCHAIN             "System Roots"
#define CA_KEYCHAIN             "login"

DECLARE_APP(SeruroClient);

const char* AsChar(const wxString &input)
{
	return input.mb_str(wxConvUTF8);
}

/* Returns SHA1 of a wxMemoryBuffer. */
wxString GetFingerprintFromBuffer(const wxMemoryBuffer &cert)
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

wxString GetSubjectKeyIDFromCertificate(SecCertificateRef &cert)
{
    wxMemoryBuffer subject_data;
    CFDictionaryRef certificate_values;
    CFDictionaryRef skid_value;
    CFMutableArrayRef keys;
    
    bool status;
    // (for searching) kSecAttrSubjectKeyID, of type kSecClassCertificate, returns, CFDataRef
    // (for retreiving) kSecOIDSubjectKeyIdentifier, 
    
    /* Set the list of attributes. */
    keys = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    CFArrayAppendValue(keys, kSecOIDSubjectKeyIdentifier);
    
    /* Request dictionary of dictionaries (one for each attribute). */
    certificate_values = SecCertificateCopyValues(cert, keys, NULL);
    status = CFDictionaryContainsKey(certificate_values, kSecOIDSubjectKeyIdentifier);
    
    skid_value = (CFDictionaryRef) CFDictionaryGetValue(certificate_values, kSecOIDSubjectKeyIdentifier); /* has two values */
    status = CFDictionaryContainsKey(skid_value, kSecPropertyKeyValue);
    
    /* The value of kSecOIDSubjectKeyIdentifier is an array type. */
    CFArrayRef skid_list = (CFArrayRef) CFDictionaryGetValue(skid_value, kSecPropertyKeyValue); /* size =2 */
    
    CFDictionaryRef skid;
    CFDataRef skid_data;
    const void *sub_value;
    
    for (int i = 0; i < CFArrayGetCount(skid_list); i++) {
        /* A dictionary will be the critical selector and the OID values. */
        skid = (CFDictionaryRef) CFArrayGetValueAtIndex(skid_list, i);
        sub_value = CFDictionaryGetValue(skid, kSecPropertyKeyValue);
        /* The OID value is a data type. */
        if (CFGetTypeID(sub_value) == CFDataGetTypeID()) {
            skid_data = (CFDataRef) sub_value;
            break;
        }
    }
    
    subject_data.AppendData(CFDataGetBytePtr(skid_data), CFDataGetLength(skid_data));
    
    CFRelease(keys);
    CFRelease(certificate_values);
    
    wxLogMessage(_("GetSubjectKeyIDFromCertificate: %s."), wxBase64Encode(subject_data));
    return wxBase64Encode(subject_data);
}

/* Returns SHA1 from an identity reference (certificate). */
wxString GetFingerprintFromIdentity(const SecIdentityRef &identity)
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

/* Returns a "fingerprint" meaning a SHA1 of a keychain persistent data reference. */
wxString GetFingerprintFromReference(const CFDataRef &item)
{
    wxMemoryBuffer item_buffer;
    
    item_buffer.AppendData(CFDataGetBytePtr(item), CFDataGetLength(item));
    return wxBase64Encode(item_buffer);
}

/* Sets item to the persistent data reference held in fingerprint. */
CFDataRef CreateReferenceFromFingerprint(const wxString &fingerprint)
{
    CFDataRef item;
    wxMemoryBuffer item_buffer;
    
    item_buffer = wxBase64Decode(fingerprint);
    item = CFDataCreate(kCFAllocatorDefault, (UInt8 *) item_buffer.GetData(), item_buffer.GetDataLen());
    
    return item;
}

/* Sets trust of x509 extensions for the certificate. */
bool SetTrustPolicy(SecCertificateRef &cert)
{
    OSStatus result;
    CFMutableArrayRef trust_settings_list;
    CFMutableDictionaryRef trust_setting;
    CFNumberRef trust_decision;
    SecPolicyRef x509_policy;
    
    /* Create a basic x509 policy. */
    x509_policy = SecPolicyCreateBasicX509();
    
    /* Create a list of trust settings, and a single trust setting (dictionary). */
    trust_settings_list = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    trust_setting = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    /* Set the setting to confirm trust. */
    SecTrustSettingsResult trust_action = kSecTrustResultConfirm;
    trust_decision = CFNumberCreate(NULL, kCFNumberSInt32Type, &trust_action);
    
    /* Set the policy for the trust setting. */
    CFDictionaryAddValue(trust_setting, kSecTrustSettingsPolicy, x509_policy);
    CFDictionaryReplaceValue(trust_setting, kSecTrustSettingsResult, trust_decision);
    CFArrayAppendValue(trust_settings_list, trust_setting);
    
    /* Apply trust settings to certificate (OSX). */
    result = SecTrustSettingsSetTrustSettings(cert, kSecTrustSettingsDomainUser, trust_settings_list);
    
    CFRelease(x509_policy);
    CFRelease(trust_settings_list);
    CFRelease(trust_setting);
    CFRelease(trust_decision);
    
    if (! result == errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (SetTrustPolicy) failed (err= %d)."), result);
    }
    
    /* Allow invalid parameters, for leaf certificates which have trusted issuers. */
    if (result == errSecParam || result == errSecSuccess) {
        return true;
    }
    
    return false;
}

bool InstallIdentityToKeychain(SecIdentityRef &identity, wxString keychain_name)
{
    OSStatus success;
    /* Todo: implement keychain access. */

    /* Find the identity item, add it to a dictionary, add it to the keychain. */
    CFMutableDictionaryRef identity_item;
    identity_item = CFDictionaryCreateMutable(kCFAllocatorDefault, 3,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(identity_item, kSecValueRef, identity);
    /* Tell the keychain that we could like a presistent reference to this item. */
    //CFDictionarySetValue(identity_item, kSecReturnPersistentRef, kCFBooleanTrue);
    
    /* Add the identity to the key chain, without error handling. */
    success = SecItemAdd(identity_item, NULL);
    
    /* Release. */
    CFRelease(identity_item);
    
    if (success == errSecParam) {
        wxLogMessage(_("SeruroCrypto> (InstallIdentityToKeychain) invalid parameters, manually accepting."));
        return true;
    }
    
    if (success != errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (InstallIdentityToKeychain) could not add identity to keychain (err= %d)."), success);
        return false;
    }
    return true;
}

//from MSW: InstallCertToStore (wxMemoryBuffer &cert, wxString store_name)
bool InstallCertificateToKeychain(const wxMemoryBuffer &cert_binary, wxString keychain_name, wxString &fingerprint)
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
    
    if (certificate == NULL) {
        wxLogMessage(_("SeruroCrypto> (InstallToKeychain) certificate is null."));
        
        CFRelease(cert_data);
        return false;
    }
    
    OSStatus success = 0;
    
    /* Apply a basic x509 trust policy to certificate. */
    if (! SetTrustPolicy(certificate)) {
        /* This is a priviledged action, which can fail if the user cannot/does not authenticate. */
        CFRelease(cert_data);
        CFRelease(certificate);
        return false;
    }
    
    /* Find the identity item, add it to a dictionary, add it to the keychain. */
    CFMutableDictionaryRef cert_item;
    cert_item = CFDictionaryCreateMutable(kCFAllocatorDefault, 4,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(cert_item, kSecClass, kSecClassCertificate);
    CFDictionarySetValue(cert_item, kSecValueRef, (const void *) certificate);
    /* Tell the keychain that we could like a presistent reference to this item. */
    //CFDictionarySetValue(cert_item, kSecReturnPersistentRef, kCFBooleanTrue);
    
    if (keychain != NULL) {
        CFDictionarySetValue(cert_item, kSecUseKeychain, (const void *) keychain);
        CFRelease(keychain);
    }
    
    /* Add to keychain. */
    success = SecItemAdd(cert_item, NULL);
    fingerprint.Append(GetSubjectKeyIDFromCertificate(certificate));
    
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
        fingerprints.Add(GetSubjectKeyIDFromCertificate(identity_cert));
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

bool SeruroCryptoMAC::RemoveIdentity(wxArrayString fingerprints) { return true; }
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

/*
wxString SeruroCryptoMAC::GetFingerprint(wxMemoryBuffer &cert)
{
    return GetFingerprintFromBuffer(cert);
}
*/


#endif