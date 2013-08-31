
/* First thing, detect OS */
#if defined(__WXMAC__)

#include <wx/log.h>
#include <wx/base64.h>
#include <wx/osx/core/cfstring.h>

/* For making HTTP/TLS reqeusts. */
#include <CoreServices/CoreServices.h>
//#include <CFNetwork/CFNetwork.h>
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
#include "../api/Utils.h"

#define IDENTITY_KEYCHAIN       "login"
#define CERTIFICATE_KEYCHAIN    "login"
#define CA_KEYCHAIN             "login"

enum search_types_t
{
    CRYPTO_SEARCH_CERT,
    CRYPTO_SEARCH_IDENTITY
};

DECLARE_APP(SeruroClient);

bool GetReferenceFromSubjectKeyID(wxString subject_key, search_types_t type, wxString keychain_name, CFTypeRef *reference)
{
    /* Find the base64 encoded subject key ID. */
    OSStatus result;
    wxMemoryBuffer subject_key_buffer;
    CFDataRef subject_key_data;
    CFMutableDictionaryRef query;
    
    /* Create subject data reference. */
    subject_key_buffer = wxBase64Decode(subject_key);
    subject_key_data = CFDataCreate(kCFAllocatorDefault,
        (UInt8 *) subject_key_buffer.GetData(), subject_key_buffer.GetDataLen());
    
    /* Create search query. */
    query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (type == CRYPTO_SEARCH_IDENTITY) {
        CFDictionaryAddValue(query, kSecClass, kSecClassIdentity);
    } else if (type == CRYPTO_SEARCH_CERT) {
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    }
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);
    CFDictionaryAddValue(query, kSecAttrSubjectKeyID, subject_key_data);
    //CFDictionaryAddValue(query, kSecMatchTrustedOnly, kCFBooleanTrue);
    CFRelease(subject_key_data);
    
    /* If a reference pointer is provided then return the search result. */
    if (reference != NULL) {
        CFDictionaryAddValue(query, kSecReturnRef, kCFBooleanTrue);
        result = SecItemCopyMatching(query, reference);
    } else {
        result = SecItemCopyMatching(query, NULL);
    }
    CFRelease(query);
    
    if (! result == errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (GetReferenceFromSubjectKeyID) failed (err= %d)."), result);
        return false;
    }
    return true;
}

bool FindSubjectKeyIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name)
{
    bool result_matches;
    CFTypeRef result_data;
    
    if (! GetReferenceFromSubjectKeyID(subject_key, type, keychain_name, &result_data)) {
        /* Could not match search query. */
        return false;
    }
    
    /* The return type may be any keychain value, make sure it matches the expected. */
    result_matches = false;
    if (CFGetTypeID(result_data) == SecCertificateGetTypeID() && type == CRYPTO_SEARCH_CERT) {
        result_matches = true;
    } else if (CFGetTypeID(result_data) == SecIdentityGetTypeID() && type == CRYPTO_SEARCH_IDENTITY) {
        result_matches = true;
    }
    
    CFRelease(result_data);
    return result_matches;
}

bool DeleteSubjectKeyIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name)
{
    OSStatus result;
    CFMutableArrayRef result_items;
    CFTypeRef result_data;
    CFMutableDictionaryRef query;
    
    /* Use a 'delete-by-transient-reference' method, using get reference. */
    if (! GetReferenceFromSubjectKeyID(subject_key, type, keychain_name, &result_data)) {
        return false;
    }
    
    /* Although there is only one item returned by GetReference..., a match list must be an array. */
    result_items = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    CFArrayAppendValue(result_items, result_data);
    
    /* The search dictionary is simply a match list of the single result_data. */
    query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    /* Sec API says the class must be specified. */
    if (type == CRYPTO_SEARCH_IDENTITY) {
        CFDictionaryAddValue(query, kSecClass, kSecClassIdentity);
    } else if (type == CRYPTO_SEARCH_CERT) {
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    }
    CFDictionaryAddValue(query, kSecMatchItemList, result_items);
    CFRelease(result_data);
    CFRelease(result_items);
    
    result = SecItemDelete(query);
    CFRelease(query);
    
    if (! result == errSecSuccess) {
        wxLogMessage(_("SeruroCrypto> (DeleteSubjectKeyIDInKeychain) failed (err= %d)."), result);
        return false;
    }
    return true;
}

wxString GetSubjectKeyIDFromCertificate(SecCertificateRef &cert)
{
    wxMemoryBuffer subject_data;
    CFDictionaryRef certificate_values;
    CFDictionaryRef skid_value;
    CFMutableArrayRef keys;
    
    bool status;
    
    /* Set the list of attributes. */
    keys = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    CFArrayAppendValue(keys, kSecOIDSubjectKeyIdentifier); /* SecCertificateOIDs.h */
    
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

bool SeruroCryptoMAC::RemoveIdentity(wxArrayString fingerprints)
{
    bool status;
    
    for (size_t i = 0; i < fingerprints.size(); i++) {
        status = DeleteSubjectKeyIDInKeychain(fingerprints[i], CRYPTO_SEARCH_IDENTITY, _(IDENTITY_KEYCHAIN));
        if (! status) return false;
    }
    return true;
}

bool SeruroCryptoMAC::RemoveCA(wxString fingerprint)
{
    bool status;
    
    status = DeleteSubjectKeyIDInKeychain(fingerprint, CRYPTO_SEARCH_CERT, _(CA_KEYCHAIN));
    return status;
}

bool SeruroCryptoMAC::RemoveCertificates(wxArrayString fingerprints)
{
    bool status;
    
    for (size_t i = 0; i < fingerprints.size(); i++) {
        status = DeleteSubjectKeyIDInKeychain(fingerprints[i], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
        if (! status) return false;
    }
    return true;
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
    
    status = FindSubjectKeyIDInKeychain(ca_fingerprint, CRYPTO_SEARCH_CERT, _(CA_KEYCHAIN));
    
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
	bool cert_1 = FindSubjectKeyIDInKeychain(certificates[0], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
	bool cert_2 = FindSubjectKeyIDInKeychain(certificates[1], CRYPTO_SEARCH_CERT, _(CERTIFICATE_KEYCHAIN));
	wxLogMessage(_("SeruroCrypto> (HaveCertificates) address (%s) (%s) in store: (1: %s, 2: %s)."),
                 server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	return (cert_1 && cert_2);
}

bool SeruroCryptoMAC::HaveIdentity(wxString server_name, wxString address, wxString fingerprint)
{
    wxArrayString identity;
    //bool cert_exists = true;
    
	/* First get the fingerprint string from the config. */
    if (fingerprint.compare(wxEmptyString) == 0) {
        //if (! theSeruroConfig::Get().HaveIdentity(server_name, address)) return false;
        identity = theSeruroConfig::Get().GetIdentity(server_name, address);
        if (identity.size() == 0) return false;
    } else {
        identity.Add(fingerprint);
    }
    
	//if (identity.size() != 2) {
	//	wxLogMessage(_("SeruroCrypto> (HaveIdentity) the identity (%s) (%s) does not have 2 certificates?"),
    //                 server_name, address);
	//	return false;
	//}
    
    
	/* Looking at the trusted Root store. */
    for (size_t i = 0; i < identity.size(); i++) {
        DEBUG_LOG(_("SeruroCrypto> (HaveIdentity) checking for fingerprint (%s)."), identity[i]);
        if (! FindSubjectKeyIDInKeychain(identity[i], CRYPTO_SEARCH_CERT, _(IDENTITY_KEYCHAIN))) {
            /* Todo: not use CRYPTO_SEARCH_IDENTITY */
            return false;
        }
    }
    
	//bool cert_1 = FindSubjectKeyIDInKeychain(identity[0], CRYPTO_SEARCH_IDENTITY, _(IDENTITY_KEYCHAIN));
	//bool cert_2 = FindSubjectKeyIDInKeychain(identity[1], CRYPTO_SEARCH_IDENTITY, _(IDENTITY_KEYCHAIN));
	//wxLogMessage(_("SeruroCrypto> (HaveIdentity) identity (%s) (%s) in store: (1: %s, 2: %s)."),
    //             server_name, address, (cert_1) ? "true" : "false", (cert_2) ? "true" : "false");
	//return (cert_1 && cert_2);
    return true;
}

#endif