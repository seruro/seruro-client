
#if defined(__WXMAC__) || defined(__WXOSX__)

#include "OSXCryptoUtils.h"
#include "../logging/SeruroLogger.h"

#include <wx/memory.h>
#include <wx/osx/core/cfstring.h>
#include <wx/base64.h>

/* For certificate/keychain import. */
#include <Security/SecCertificate.h>
/* For hashing/fingerprinting (SHA1). */
#include <CommonCrypto/CommonDigest.h>
/* For reading p12 data. */
#include <Security/SecImportExport.h>
/* For adding identity to keychain. */
#include <Security/SecItem.h>

bool GetReferenceFromSubjectKeyID(wxString subject_key, search_types_t type, wxString keychain_name, CFTypeRef *reference)
{
    /* Find the base64 encoded subject key ID. */
    OSStatus result;
    wxMemoryBuffer subject_key_buffer;
    CFDataRef subject_key_data;
    CFMutableDictionaryRef query;
    
    /* Create subject data reference. */
    subject_key_buffer = wxBase64Decode(subject_key);
    subject_key_data = CFDataCreate(kCFAllocatorDefault, (UInt8 *) subject_key_buffer.GetData(), subject_key_buffer.GetDataLen());
    
    /* Create search query. */
    query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    if (type == CRYPTO_SEARCH_IDENTITY) {
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate); /* kSecClassCertificate */
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
        DEBUG_LOG(_("SeruroCrypto> (GetReferenceFromSubjectKeyID) failed (err= %d)."), result);
        return false;
    }
    return true;
}

bool GetReferenceFromIssuerSerial(wxString issuer, wxString serial, wxString keychain_name, CFTypeRef *reference)
{
    /* Find the base64 encoded subject key ID. */
    OSStatus result;
    
    wxMemoryBuffer issuer_buffer, serial_buffer;
    CFDataRef issuer_data, serial_data;
    //CFNumberRef serial_data;
    
    CFMutableDictionaryRef query;
    
    /* Create subject data reference. */
    issuer_buffer = wxBase64Decode(issuer);
    issuer_data = CFDataCreate(kCFAllocatorDefault, (UInt8 *) issuer_buffer.GetData(), issuer_buffer.GetDataLen());
    
    serial_buffer = wxBase64Decode(serial);
    serial_data = CFDataCreate(kCFAllocatorDefault, (UInt8 *) serial_buffer.GetData(), serial_buffer.GetDataLen());
    
    /* Create search query. */
    query = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);
    CFDictionaryAddValue(query, kSecAttrIssuer, issuer_data);
    CFDictionaryAddValue(query, kSecAttrSerialNumber, serial_data);
    
    CFRelease(issuer_data);
    CFRelease(serial_data);
    
    if (reference != NULL) {
        result = SecItemCopyMatching(query, reference);
    } else {
        result = SecItemCopyMatching(query, NULL);
    }
    
    CFRelease(query);
    
    if (! result == errSecSuccess) {
        DEBUG_LOG(_("SeruroCrypto> (FindSubjectSerialInKeychain) failed (err= %d)."), result);
        return false;
    }
    return true;
}

bool FindIssuerSerialInKeychain(wxString issuer, wxString serial, wxString keychain_name)
{
    return GetReferenceFromIssuerSerial(issuer, serial, keychain_name, NULL);
}

bool FindSKIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name)
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

bool DeleteSKIDInKeychain(wxString subject_key, search_types_t type, wxString keychain_name)
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
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate); /* kSecClassIdentity */
    } else if (type == CRYPTO_SEARCH_CERT) {
        CFDictionaryAddValue(query, kSecClass, kSecClassCertificate);
    }
    CFDictionaryAddValue(query, kSecMatchItemList, result_items);
    CFDictionaryAddValue(query, kSecMatchLimit, kSecMatchLimitOne);
    CFRelease(result_data);
    CFRelease(result_items);
    
    result = SecItemDelete(query);
    CFRelease(query);
    
    if (! result == errSecSuccess) {
        DEBUG_LOG(_("SeruroCrypto> (DeleteSubjectKeyIDInKeychain) failed (err= %d)."), result);
        return false;
    }
    return true;
}

bool GetPropertyFromCertificate(const SecCertificateRef &cert, const CFTypeRef &oid, CFDataRef *property)
{
    CFDictionaryRef certificate_values;
    CFDictionaryRef property_values;
    CFMutableArrayRef keys;
    
    /* Set the list of attributes. */
    keys = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    CFArrayAppendValue(keys, oid); /* SecCertificateOIDs.h */
    
    /* Request dictionary of dictionaries (one for each attribute). */
    certificate_values = SecCertificateCopyValues(cert, keys, NULL);
    CFRelease(keys);
    
    if (! CFDictionaryContainsKey(certificate_values, oid)) {
        /* Certificate does not have the requested property. */
        return false;
    }
    
    property_values = (CFDictionaryRef) CFDictionaryGetValue(certificate_values, oid);
    
    if (! CFDictionaryContainsKey(property_values, kSecPropertyKeyValue)) {
        /* Odd, there was not value in the property result. */
        return false;
    }
    
    /* Todo: this value should be copied, certificate_values, owned by this function, will go out of scope. */
    *property = (CFDataRef) CFDictionaryGetValue(property_values, kSecPropertyKeyValue);
    return true;
}

wxString GetSerialFromCertificate(const SecCertificateRef &cert)
{
    wxMemoryBuffer encoded_serial;
    
    CFStringRef serial_data;
    char *serial_cstring;
    
    if (! GetPropertyFromCertificate(cert, kSecOIDX509V1SerialNumber, (CFDataRef*) &serial_data)) {
        return wxEmptyString;
    }
    
    /* A string-type value contains a C-string, that cannot be accessed directly. */
    serial_cstring = (char*) malloc(sizeof(char) * CFStringGetLength(serial_data)+1);
    if (! CFStringGetCString(serial_data, serial_cstring, CFStringGetLength(serial_data)+1, kCFStringEncodingASCII)) {
        free(serial_cstring);
        return wxEmptyString;
    }
    
    /* Add cstring to a memory buffer to return encoded. */
    encoded_serial.AppendData(serial_cstring, CFStringGetLength(serial_data));
    free(serial_cstring);
    
    return wxBase64Encode(encoded_serial);
}

uint8_t OIDFromString(const CFStringRef &oid)
{
    const char* oid_raw;
    
    /* Convert a cstring into a single-byte integer. */
    oid_raw = CFStringGetCStringPtr(oid, CFStringGetFastestEncoding(oid));
    if (oid_raw == NULL) {
        DEBUG_LOG(_("OSXCryptoUtils> (OIDFromString) Cannot encode OID."));
        return 0;
    }
    
    return (uint8_t) atoi(oid_raw);
}

bool ParseIssuerAttribute(issuer_attr_t &attribute, const CFStringRef &oid, const CFStringRef &value)
{
    CFArrayRef oid_sections;
    uint8_t oid_encoded;
    
    /* Split the OID by ".". */
    oid_sections = CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, oid, CFSTR("."));
    
    if (oid_sections == NULL) {
        DEBUG_LOG(_("OSXCryptoUtils> (ParseIssuerAttribute) OID section parsing failued."));
        /* The string could not be separated. */
        return false;
    }
    
    if (CFArrayGetCount(oid_sections) != 4) {
        /* This is very unexpected, there is a non-standard OID in the issuer subject. */
        DEBUG_LOG(_("OSXCryptoUtils> (ParseIssuerAttribute) Unexpected OID."));
        CFRelease(oid_sections);
        return false;
    }
    
    /* Encode the 4-sized OID. */
    oid_encoded = OIDFromString((CFStringRef) CFArrayGetValueAtIndex(oid_sections, 0));
    if (oid_encoded == 0) { CFRelease(oid_sections); return false; }
    attribute.oid[0] = oid_encoded * 40;
    
    oid_encoded = OIDFromString((CFStringRef) CFArrayGetValueAtIndex(oid_sections, 1));
    if (oid_encoded == 0) { CFRelease(oid_sections); return false; }
    attribute.oid[0] += oid_encoded;
    
    oid_encoded = OIDFromString((CFStringRef) CFArrayGetValueAtIndex(oid_sections, 2));
    if (oid_encoded == 0) { CFRelease(oid_sections); return false; }
    attribute.oid[1] = oid_encoded;
    
    oid_encoded = OIDFromString((CFStringRef) CFArrayGetValueAtIndex(oid_sections, 3));
    if (oid_encoded == 0) { CFRelease(oid_sections); return false; }
    attribute.oid[2] = oid_encoded;
    
    CFRelease(oid_sections);

    /* Set the value pointer. */
    attribute.value = (char*) CFStringGetCStringPtr(value, CFStringGetFastestEncoding(value));
    if (attribute.value == NULL) {
        DEBUG_LOG(_("OSXCryptoUtils> (ParseIssuerAttribute) Cannot get attribute value data."));
        return false;
    }
    
    /* Finally set the size. */
    attribute.size = CFStringGetLength(value);
    return true;
}

wxString GetIssuerFromCertificate(const SecCertificateRef &cert)
{
    wxMemoryBuffer encoded_issuer;
    //wxJSONValue issuer;
    
    CFArrayRef issuer_attributes;
    CFDictionaryRef attribute;
    
    if (! GetPropertyFromCertificate(cert, kSecOIDX509V1IssuerName, (CFDataRef*) &issuer_attributes)) {
        return wxEmptyString;
    }

    issuer_t issuer;
    CFStringRef attribute_value, attribute_oid;
    
    issuer.attribute_count = (uint8_t) CFArrayGetCount(issuer_attributes);
    issuer.attributes = (issuer_attr_t*) malloc(sizeof(issuer_attr_t) * issuer.attribute_count);
    
    uint16_t total_size = 0;
    bool parsing_failed = false;
    for (uint8_t i = 0; i < issuer.attribute_count; i++) {
        /* A dictionary will be the critical selector for the OID values. */
        attribute = (CFDictionaryRef) CFArrayGetValueAtIndex(issuer_attributes, i);
        attribute_value = (CFStringRef) CFDictionaryGetValue(attribute, kSecPropertyKeyValue);
        attribute_oid = (CFStringRef) CFDictionaryGetValue(attribute, kSecPropertyKeyLabel);
        
        if (! ParseIssuerAttribute(issuer.attributes[i], attribute_oid, attribute_value)) {
            parsing_failed = true;
            break;
        }
        
        total_size += issuer.attributes[i].size + 11 /* size of added DER header. */;
    }
    
    if (parsing_failed) {
        /* Could not parse an OID/value in the issuer name. */
        DEBUG_LOG(_("OSXCryptoUtils> (GetIssuerFromCertificate) OID Attribute parsing failed."));
        return wxEmptyString;
    }
    
    /* Add DER header (sequence + size) */
    encoded_issuer.AppendByte(0x30);
    if (total_size <= 127) {
        encoded_issuer.AppendByte((uint8_t) total_size);
    } else {
        /* Total size requires a byte = number of additional bytes with size. */
        encoded_issuer.AppendByte(0x80 + (total_size / 256)+1);
        //encoded_issuer.AppendByte(0x81);
        if (total_size > 256) {
            /* Only support 2 bytes, in the first place the big-endian. */
            encoded_issuer.AppendByte(total_size & 0xFF00);
        }
        encoded_issuer.AppendByte(total_size & 0x00FF);
    }
    
    for (uint8_t i = 0; i < issuer.attribute_count; i++) {
        encoded_issuer.AppendByte(0x31);
        encoded_issuer.AppendByte(issuer.attributes[i].size + 9);
        encoded_issuer.AppendByte(0x30);
        encoded_issuer.AppendByte(issuer.attributes[i].size + 7);
        encoded_issuer.AppendByte(0x06);
        encoded_issuer.AppendByte(0x03);
        encoded_issuer.AppendByte(issuer.attributes[i].oid[0]);
        encoded_issuer.AppendByte(issuer.attributes[i].oid[1]);
        encoded_issuer.AppendByte(issuer.attributes[i].oid[2]);
        encoded_issuer.AppendByte(0x0C);
        encoded_issuer.AppendByte(issuer.attributes[i].size);
        encoded_issuer.AppendData(issuer.attributes[i].value, issuer.attributes[i].size);
    }
    
    /* Do some stuff. */
    return wxBase64Encode(encoded_issuer);
}

wxString GetSKIDFromCertificate(const SecCertificateRef &cert)
{
    wxMemoryBuffer encoded_skid;
    
    CFArrayRef skid_attributes;
    CFDictionaryRef skid;
    
    if (! GetPropertyFromCertificate(cert, kSecOIDSubjectKeyIdentifier, (CFDataRef*) &skid_attributes)) {
        return wxEmptyString;
    }
    
    CFDataRef skid_data_pointer = nil;
    const void *skid_value;
    
    for (int i = 0; i < CFArrayGetCount(skid_attributes); i++) {
        /* A dictionary will be the critical selector for the OID values. */
        skid = (CFDictionaryRef) CFArrayGetValueAtIndex(skid_attributes, i);
        skid_value = CFDictionaryGetValue(skid, kSecPropertyKeyValue);
        
        /* The OID value is a data type. */
        if (CFGetTypeID(skid_value) == CFDataGetTypeID()) {
            skid_data_pointer = (CFDataRef) skid_value;
            break;
        }
    }
    
    if (skid_data_pointer != nil) {
        encoded_skid.AppendData(CFDataGetBytePtr(skid_data_pointer), CFDataGetLength(skid_data_pointer));
    }
    
    return wxBase64Encode(encoded_skid);
}

wxString GetSKIDFromIssuerSerial(wxString issuer, wxString serial, wxString keychain_name)
{
    CFTypeRef cert_data;
    SecCertificateRef cert;
    wxString subject_key;
    
    /* Populate the cert structure. */
    if (!GetReferenceFromIssuerSerial(issuer, serial, keychain_name, &cert_data)) {
        return wxEmptyString;
    }
    
    if (CFGetTypeID(cert_data) != SecCertificateGetTypeID()) {
        CFRelease(cert_data);
        return wxEmptyString;
    }
    
    cert = (SecCertificateRef) cert_data;
    subject_key = GetSKIDFromCertificate(cert);
    CFRelease(cert_data);
    
    return subject_key;
}

bool GetIssuerSerialFromSKID(const wxString &subject_key, wxString &issuer, wxString &serial, wxString keychain_name)
{
    CFTypeRef cert_data;
    SecCertificateRef cert;
    
    /* Populate the cert structure. */
    if (!GetReferenceFromSubjectKeyID(subject_key, CRYPTO_SEARCH_CERT, keychain_name, &cert_data)) {
        return false;
    }
    
    if (CFGetTypeID(cert_data) != SecCertificateGetTypeID()) {
        CFRelease(cert_data);
        return false;
    }
    
    cert = (SecCertificateRef) cert_data;
    serial.Append(GetSerialFromCertificate(cert));
    issuer.Append(GetIssuerFromCertificate(cert));
    
    CFRelease(cert_data);
    return true;
}

/* Sets trust of x509 extensions and S/MIME for the certificate. */
bool SetTrustPolicy(SecCertificateRef &cert)
{
    OSStatus result;
    CFMutableArrayRef trust_settings_list;
    CFMutableDictionaryRef trust_setting;
    SecPolicyRef trust_policy;
    
    CFNumberRef trust_decision;
    SecTrustSettingsResult trust_action;
    
    /* Create a list of trust settings (x509 and S/MIME). */
    trust_settings_list = CFArrayCreateMutable (NULL, 0, &kCFTypeArrayCallBacks);
    trust_action = kSecTrustResultConfirm;
    
    /* Create a basic x509 policy. */
    trust_policy = SecPolicyCreateBasicX509();
    /* Create a single trust setting (dictionary). */
    trust_setting = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    /* Set the setting to confirm trust. */
    trust_decision = CFNumberCreate(NULL, kCFNumberSInt32Type, &trust_action);
    /* Set the policy for the trust setting. */
    CFDictionaryAddValue(trust_setting, kSecTrustSettingsPolicy, trust_policy);
    CFDictionaryReplaceValue(trust_setting, kSecTrustSettingsResult, trust_decision);
    CFArrayAppendValue(trust_settings_list, trust_setting);
    
    /* Cleanup x509 trust data. */
    CFRelease(trust_policy);
    CFRelease(trust_setting);
    CFRelease(trust_decision);
    
    /* Create an S/MIME policy. */
    trust_policy = SecPolicyCreateWithOID(kSecPolicyAppleSMIME);
    /* Create a single trust setting (dictionary). */
    trust_setting = CFDictionaryCreateMutable(NULL, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    /* Set the setting to confirm trust. */
    trust_decision = CFNumberCreate(NULL, kCFNumberSInt32Type, &trust_action);
    /* Set the policy for the trust setting. */
    CFDictionaryAddValue(trust_setting, kSecTrustSettingsPolicy, trust_policy);
    CFDictionaryReplaceValue(trust_setting, kSecTrustSettingsResult, trust_decision);
    CFArrayAppendValue(trust_settings_list, trust_setting);
    
    /* Cleanup S/MIME trust data. */
    CFRelease(trust_policy);
    CFRelease(trust_setting);
    CFRelease(trust_decision);
    
    /* Apply trust settings to certificate (OSX). */
    result = SecTrustSettingsSetTrustSettings(cert, kSecTrustSettingsDomainUser, trust_settings_list);
    CFRelease(trust_settings_list);
    
    if (! result == errSecSuccess) {
        DEBUG_LOG(_("SeruroCrypto> (SetTrustPolicy) failed (err= %d)."), result);
    }
    
    /* Allow invalid parameters, for leaf certificates which have trusted issuers. */
    if (result == errSecParam || result == errSecSuccess) {
        return true;
    }
    
    return false;
}

bool InstallIdentityToKeychain(const SecIdentityRef &identity, wxString keychain_name)
{
    OSStatus success;
    
    /* Find the identity item, add it to a dictionary, add it to the keychain. */
    CFMutableDictionaryRef identity_item;
    identity_item = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
                                              &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    
    CFDictionarySetValue(identity_item, kSecValueRef, identity);
    
    /* Add the identity to the key chain, without error handling. */
    success = SecItemAdd(identity_item, NULL);
    
    /* Release. */
    CFRelease(identity_item);
    
    if (success == errSecParam) {
        DEBUG_LOG(_("SeruroCrypto> (InstallIdentityToKeychain) invalid parameters, manually accepting."));
        return true;
    }
    
    if (success != errSecSuccess && success != errSecDuplicateItem) {
        DEBUG_LOG(_("SeruroCrypto> (InstallIdentityToKeychain) could not add identity to keychain (err= %d)."), success);
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
        DEBUG_LOG(_("SeruroCrypto> (InstallToKeychain) certificate is null."));
        
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
    
    if (keychain != NULL) {
        CFDictionarySetValue(cert_item, kSecUseKeychain, (const void *) keychain);
        CFRelease(keychain);
    }
    
    /* Add to keychain. */
    success = SecItemAdd(cert_item, NULL);
    fingerprint.Append(GetSKIDFromCertificate(certificate));
    
    /* Release objects. */
    CFRelease(certificate);
    CFRelease(cert_data);
    CFRelease(cert_item);
    
    if (success == errSecDuplicateItem) {
        DEBUG_LOG(_("SeruroCrypto> (InstallToKeychain) duplicate certificate detected."));
        return true;
    }
    
    if (success != errSecSuccess) {
        DEBUG_LOG(_("SeruroCrypto> (InstallToKeychain) error (%d)."), success);
        return false;
    }
    return true;
}

#endif
