
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_OutlookIdentity.h"
#include "AppOSX_Utils.h"

#include "../../api/Utils.h"
#include "../../SeruroClient.h"
#include "../../logging/SeruroLogger.h"

#include <wx/base64.h>

uint32_t __bint32(uint32_t x)
{
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) { return x; }
    return (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24));
}

uint16_t __bint16(uint16_t x)
{
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) { return x; }
    return (((x) >> 8) | ((x) << 8));
}

void AppOSX_OutlookIdentity::SetPath(wxString path)
{
    wxArrayString test_string;
    
    /* A simple wrapper to set the path. */
    this->full_path = path;
}

void AppOSX_OutlookIdentity::SetData(void *data, size_t length)
{
    /* Copy memory to internal storage while the identity is created. */
    this->raw_data.AppendData(data, length);
}

bool AppOSX_OutlookIdentity::HasAuthCertificate()
{
    return (this->marc_certs.auth_cert_size > 0);
}

bool AppOSX_OutlookIdentity::HasEncCertificate()
{
    return (this->marc_certs.enc_cert_size > 0);
}

wxJSONValue AppOSX_OutlookIdentity::GetAuthCertificate()
{
    wxJSONValue auth_certificate;
    wxMemoryBuffer serial, subject;
    
    if (this->marc_certs.auth_cert_size > 0) {
        serial.AppendData(this->marc_certs.auth_cert.serial, this->marc_certs.auth_cert.der_offset);
        subject.AppendData(this->marc_certs.auth_cert.der, this->marc_certs.auth_cert.der_size);
        
        auth_certificate["serial"] = wxBase64Encode(serial);
        auth_certificate["subject"] = wxBase64Encode(subject);
        
        serial.Clear();
        subject.Clear();
    }
    
    return auth_certificate;
}

wxJSONValue AppOSX_OutlookIdentity::GetEncCertificate()
{
    wxJSONValue enc_certificate;
    wxMemoryBuffer serial, subject;
    
    if (this->marc_certs.enc_cert_size > 0) {
        serial.AppendData(this->marc_certs.enc_cert.serial, this->marc_certs.enc_cert.der_offset);
        subject.AppendData(this->marc_certs.enc_cert.der, this->marc_certs.enc_cert.der_size);
        
        enc_certificate["serial"] = wxBase64Encode(serial);
        enc_certificate["subject"] = wxBase64Encode(subject);
        
        serial.Clear();
        subject.Clear();
    }
    
    return enc_certificate;
}

wxString AppOSX_OutlookIdentity::GetAddress()
{
    /* Marc data must be present. */
    if (! this->marc_parsed) {
        return wxEmptyString;
    }
    
    wxString account_address;
    marc_option_string_t *option_string;
    
    /* Find the address option string, and copy the bytes into a wxString, then add to the account object. */
    for (size_t i = 0; i < marc_option_strings.size(); ++i) {
        option_string = marc_option_strings[i];
        
        if (__bint32(option_string->tag) == OPTION_EMAIL_ADDRESS) {
            account_address.Append((char*) option_string->value, __bint32(option_string->length));
            break;
        }
    }
    
    return account_address;
}

bool AppOSX_OutlookIdentity::ParseMarc()
{
    uint32_t marc_offset = 0;
    if (raw_data.GetBufSize() < sizeof(marc_header_t)) {
        /* The header is missing. */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) buffer size (%d) is less than header (%d)."),
            (int) raw_data.GetDataLen(), (int) sizeof(marc_header_t));
        return false;
    }
    
    /* CHECKS:
     *   1) The size of the buffer contains at least enough for a MARC header.
     *   2) The size1 and size2 *should* be equal.
     *   3) The size1 *should* be equal to the size of the buffer - the size of the header.
     *   4) The size1 *must* be at least the size of an extended header.
     */
    
    /* The magic string is missing (it was read earlier). */
    marc_header = (marc_header_t*) raw_data.GetData();
    marc_offset += sizeof(marc_header_t);
    
    /* The size reported in size1 does NOT count the header or extended header. */
    if (__bint32(marc_header->size1) != raw_data.GetDataLen() - sizeof(marc_header_t)
        || __bint32(marc_header->size1) < sizeof(marc_extended_header_t)) {
        /* The header is reporting an odd-length size. */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) incorrect marc_header size1 (%d)."),
                  (int) __bint32(marc_header->size1));
        return false;
    }
    
    /* CHECKS:
     *   1) The options count * 8 *should* be equal to the options size.
     *   2) The options size + data size *must* equal size1 from the header.
     *   3) The combined checks assure that reading options and data are valid in the buffer.
     */
    
    /* The above condition checked if there was adequate allocation for an extended header. */
    marc_ext_header = (marc_extended_header_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
    marc_offset += sizeof(marc_extended_header_t);
    
    if (__bint32(marc_ext_header->options_size) != (__bint16(marc_ext_header->options_count) * MARC_OPTION_BYTE_SIZE) + sizeof(marc_extended_header_t)) {
        /* This is an un-explored condition. */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) incorrect marc_header options count (%d) and options size (%d)."),
                  (int) __bint32(marc_ext_header->options_count), (int) __bint32(marc_ext_header->options_size));
        return false;
    }
    
    if (__bint32(marc_ext_header->options_size) + __bint32(marc_ext_header->data_size) != __bint32(marc_header->size1)) {
        /* The combined header size is incorrect. */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) options (%d) and data (%d) sizes are too large, expected (%d)."),
                  (int) __bint32(marc_ext_header->options_size), (int) __bint32(marc_ext_header->data_size),
                  (int) __bint32(marc_header->size1));
        return false;
    }
    
    //MarcOptionArray marc_options;
    marc_option_t *marc_option;
    /* The option flags are [0xb00 - 0xc00). The count is maintained for the data data structure. */
    size_t marc_option_flags_size = 0;
    
    /* Iterate over the number of options, adding them to the option array. */
    for (uint32_t i = 0; i < __bint16(marc_ext_header->options_count); ++i) {
        marc_option = (marc_option_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
        if (__bint32(marc_option->tag) >= MARC_OPTION_FLAG_MIN &&
            __bint32(marc_option->tag) < MARC_OPTION_FLAG_MAX) {
            /* This check is fuzzy. */
            marc_option_flags_size += 1;
        }
        
        /* Check for certificate length options. */
        if (__bint32(marc_option->tag) == OPTION_AUTH_CERT_LENGTH &&
            __bint32(marc_option->value) > 0) {
            marc_certs.auth_cert_size = __bint32(marc_option->value);
        } else if (__bint32(marc_option->tag) == OPTION_ENC_CERT_LENGTH &&
                   __bint32(marc_option->value) > 0) {
            marc_certs.enc_cert_size = __bint32(marc_option->value);
        }
        
        marc_options.Add(marc_option);
        marc_offset += MARC_OPTION_BYTE_SIZE;
    }
    
    marc_data_header = (marc_data_header_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
    marc_offset += sizeof(marc_data_header_t);
    
    marc_option_flags = (uint8_t*) ((char*) raw_data.GetData() + marc_offset);
    marc_offset += marc_option_flags_size;
    
    /* Store the potiner to each cert (if present) and advance the offset count. */
    if (marc_certs.auth_cert_size > 0) {
        if (marc_certs.auth_cert_size + marc_offset > raw_data.GetDataLen()) {
            /* The auth cert size will cause a buffer overrun. */
            DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) authentication cert size (%d) too large."),
                      (int) marc_certs.auth_cert_size);
            return false;
        }
        marc_certs.auth_cert_data = ((uint8_t*) raw_data.GetData() + marc_offset);
        this->ParseCert(marc_certs.auth_cert_data, marc_certs.auth_cert_size, true);
        marc_offset += marc_certs.auth_cert_size;
    }
    
    if (marc_certs.enc_cert_size > 0) {
        if (marc_certs.enc_cert_size + marc_offset > raw_data.GetDataLen()) {
            /* The enc cert size will cause a buffer overrun. */
            DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) encipherment cert size (%d) too large."),
                      (int) marc_certs.enc_cert_size);
            return false;
        }
        marc_certs.enc_cert_data = ((uint8_t*) raw_data.GetData() + marc_offset);
        this->ParseCert(marc_certs.enc_cert_data, marc_certs.enc_cert_size, false);
        marc_offset += marc_certs.enc_cert_size;
    }
    
    marc_option_string_t *marc_option_string;
    /* Potentially save the offset of the options data. */
    size_t marc_option_data_offset = 0;
    size_t marc_option_data_size = 0;

    /* Iterate through the option/length identifiers. */
    for (uint32_t i = 0; i < __bint16(marc_ext_header->options_count); ++i) {
        marc_option = marc_options[i];
        if (__bint32(marc_option->tag) < MARC_OPTION_STRING_MIN ||
            __bint32(marc_option->tag) >= MARC_OPTION_STRING_MAX) {
            continue;
        }
        
        if (__bint32(marc_option->value) + marc_offset > raw_data.GetDataLen()) {
            /* The option string size will cause a buffer overrun. */
            DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) option (%d) string size (%d) too large."),
                      (int) __bint32(marc_option->tag), (int) __bint32(marc_option->value));
            return false;
        }
        
        /* Notice the heap and no bint32 conversions. */
        marc_option_string = new marc_option_string_t;
        marc_option_string->tag = marc_option->tag;
        marc_option_string->length = marc_option->value;
        marc_option_string->value = (uint8_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
        marc_option_strings.Add(marc_option_string);
        
        if (__bint32(marc_option->tag) == OPTION_DATA) {
            /* Save the offset to the options data for later-parsing. */
            marc_option_data_offset = marc_offset;
            marc_option_data_size = __bint32(marc_option_string->length);
        }
        
        marc_offset += __bint32(marc_option_string->length);
    }
    
    if (marc_offset != raw_data.GetDataLen()) {
        /* There is trailing data in the identity? */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) problem, there are (%d) trailing bytes."),
                  (int) (raw_data.GetDataLen() - marc_offset));
        return false;
    }
    
    if (marc_option_data_offset == 0 || marc_option_data_size == 0) {
        /* No options data was found, thus parsing is finished! */
        return true;
    }
    
    if (marc_option_data_size % MARC_OPTION_DATA_SIZE != 0) {
        /* There is an unexpected number of bytes within the options data? */
        DEBUG_LOG(_("AppOSX_OutlookIdentity> (ParseMarc) the option data size (%s) is incorrect."),
                  (int) marc_option_data_size);
        return false;
    }
    
    marc_option_data_t *marc_option_data;
    
    for (size_t i = 0; i < marc_option_data_size / MARC_OPTION_DATA_SIZE; ++i) {
        marc_option_data = (marc_option_data_t*) ((uint8_t*) raw_data.GetData() + marc_option_data_offset);
        marc_option_datas.Add(marc_option_data);
        
        marc_option_data_offset += MARC_OPTION_DATA_SIZE;
    }
    
    this->marc_parsed = true;
    return true;
}

bool AppOSX_OutlookIdentity::ParseCert(uint8_t *data, uint32_t size, bool is_auth)
{
    marc_cert_t *cert;
    
    /* Select the auth structure. */
    if (is_auth) { cert = &this->marc_certs.auth_cert; }
    else { cert = &this->marc_certs.enc_cert; }
    
    if (size < sizeof(uint32_t) * 3) {
        return false;
    }
    
    /* This, like certs, is a pseudo structure "filled-in" with the raw_data structure counter parts. */
    cert->_unknown =   __bint32(*(uint32_t*) (data));
    cert->der_offset = __bint32(*(uint32_t*) (data + sizeof(uint32_t)));
    cert->der_size =   __bint32(*(uint32_t*) (data + (sizeof(uint32_t) * 2)));
    
    if (cert->der_offset == 0 || cert->der_size == 0 ||
        cert->der_offset + cert->der_size + (sizeof(uint32_t) * 3) != size) {
        /* Sizing problems. */
        return false;
    }
    
    cert->serial = data + (sizeof(uint32_t) * 3);
    cert->der = data + (sizeof(uint32_t) * 3) + cert->der_offset;
    
    return true;
}

#endif /* OS Check */
