
#if defined(__WXOSX__) || defined(__WXMAC__)

#include "AppOSX_OutlookIdentity.h"
#include "AppOSX_Utils.h"

#include "../../api/Utils.h"
#include "../../SeruroClient.h"
#include "../../logging/SeruroLogger.h"
#include <zlib.h>

#include <wx/base64.h>

uint32_t buffer_crc32(const wxMemoryBuffer &buffer)
{
    uLong crc = 0;
    
    crc = crc32(crc, (Bytef *) buffer.GetData(), (uInt) buffer.GetDataLen());
    return (uint32_t) crc;
}

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

uint32_t __to_bint32(uint32_t x)
{
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) { return x; }
    return (((x) << 24) | (((x) & 0x0000FF00) << 8) | (((x) & 0x00FF0000) >> 8) | ((x) >> 24));
}

uint16_t __to_bint16(uint16_t x)
{
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) { return x; }
    return (((x) << 8) | ((x) >> 8));
}

void WriteBE32(wxMemoryBuffer &buffer, uint32_t i)
{
    /* Input is BE. */
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) {
        /* System is BE. */
        buffer.AppendByte((i & 0xFF000000) >> 24);
        buffer.AppendByte((i & 0x00FF0000) >> 16);
        buffer.AppendByte((i & 0x0000FF00) >> 8);
        buffer.AppendByte((i & 0x000000FF));
    } else {
        /* System is LE. */
        buffer.AppendByte((i & 0x000000FF));
        buffer.AppendByte((i & 0x0000FF00) >> 8);
        buffer.AppendByte((i & 0x00FF0000) >> 16);
        buffer.AppendByte((i & 0xFF000000) >> 24);
    }
}

void WriteBE64(wxMemoryBuffer &buffer, uint64_t i)
{
    /* Input is BE. */
    union { uint16_t i; char c[2]; } __myint = {0x0102};
    if (__myint.c[0] == 1) {
        /* System is BE. */
        WriteBE32(buffer, (i & 0xFFFFFFFF00000000) >> 32);
        WriteBE32(buffer, (i & 0x00000000FFFFFFFF));
    } else {
        /* System is LE. */
        WriteBE32(buffer, (i & 0x00000000FFFFFFFF));
        WriteBE32(buffer, (i & 0xFFFFFFFF00000000) >> 32);
    }
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

bool AddOption(MarcOptionArray &options, const marc_option_t *option, bool trigger, marc_option_tag_t tag, uint32_t value)
{
    marc_option_t *new_option;
    
    /* Check if option is greater than tag, if yes and trigger has not been met add the tag/value. */
    if ((__bint32(option->tag) > tag && ! trigger) || __bint32(option->tag) == tag) {
        /* Add value. */
        new_option = (marc_option_t*) malloc(sizeof(marc_option_t));
        new_option->tag = __to_bint32(tag);
        new_option->value = __to_bint32(value);
        options.Add(new_option);

        return true;
    }
    
    return false;
}

bool AddOptionData(MarcOptionDataArray &options, const marc_option_data_t *option, bool trigger,
    marc_option_tag_t tag, uint32_t rev)
{
    marc_option_data_t *new_option;
    
    /* Check if option is greater than tag, if yes and trigger has not been met add the tag/value. */
    if ((__bint32(option->tag) > tag && ! trigger) || __bint32(option->tag) == tag) {
        /* Add value. */
        new_option = (marc_option_data_t*) malloc(sizeof(marc_option_data_t));
        new_option->tag = __to_bint32(tag);
        new_option->value1 = 0;
        new_option->value2 = rev;
        options.Add(new_option);
        
        return true;
    }
    
    return false;
}

bool AppOSX_OutlookIdentity::AssignCerts(const wxString &auth_issuer, const wxString &auth_serial,
    const wxString &enc_issuer, const wxString &enc_serial)
{
    /* Copy the parsed data into local structures, and prepare them for writing. */
    marc_header_t new_marc_header = *this->marc_header;
    marc_extended_header_t new_marc_ext_header = *this->marc_ext_header;
    uint32_t new_revision;
    
    /* Construct options, in order, assuring the certificate-specific options are included. */
    MarcOptionArray new_marc_options;
    wxMemoryBuffer new_marc_option_flags;
    
    /* Required option flags. */
    bool has_option_encrypt_outgoing = false;
    bool has_option_sign_outgoing = false;
    bool has_option_include_certs = false;
    bool has_option_send_signed = false;
    
    /* Request certificate option-length flags. */
    bool has_option_auth_length = false;
    bool has_option_enc_length = false;
    
    wxMemoryBuffer auth_issuer_buffer, auth_serial_buffer;
    wxMemoryBuffer enc_issuer_buffer, enc_serial_buffer;
    
    /* Decode the parameters, to determine lengths. */
    auth_issuer_buffer = wxBase64Decode(auth_issuer);
    auth_serial_buffer = wxBase64Decode(auth_serial);
    enc_issuer_buffer = wxBase64Decode(enc_issuer);
    enc_serial_buffer = wxBase64Decode(enc_serial);
    
    /* Add each option from the parsed MaRC, insert all certificate-related options. */
    marc_option_t *new_option;
    for (size_t i = 0; i < this->marc_options.size(); ++i) {
        /* The AppOption will modify the option set if it found the given option or it has passed the option. */
        if (AddOption(new_marc_options, marc_options[i], has_option_sign_outgoing, OPTION_SIGN_OUTGOING, 0x01)) {
            has_option_sign_outgoing = true;
            new_marc_option_flags.AppendByte(0x01);
            /* If this is the given option, do not add it twice. */
            if (__bint32(marc_options[i]->tag) == OPTION_SIGN_OUTGOING) continue;
        }
        
        if (AddOption(new_marc_options, marc_options[i], has_option_include_certs, OPTION_INCLUDE_CERTS, 0x01)) {
            has_option_include_certs = true;
            new_marc_option_flags.AppendByte(0x01);
            if (__bint32(marc_options[i]->tag) == OPTION_INCLUDE_CERTS) continue;
        }
        
        if (AddOption(new_marc_options, marc_options[i], has_option_send_signed, OPTION_SEND_SIGNED_CLEAR, 0x01)) {
            has_option_send_signed = true;
            new_marc_option_flags.AppendByte(0x01);
            if (__bint32(marc_options[i]->tag) == OPTION_SEND_SIGNED_CLEAR) continue;
        }
        
        if (AddOption(new_marc_options, marc_options[i], has_option_encrypt_outgoing, OPTION_ENCRYPT_OUTGOING, 0x01)) {
            has_option_encrypt_outgoing = true;
            new_marc_option_flags.AppendByte(0x01);
            if (__bint32(marc_options[i]->tag) == OPTION_ENCRYPT_OUTGOING) continue;
        }
        
        /* Option-lengths for certificate data. */
        if (AddOption(new_marc_options, marc_options[i], has_option_auth_length, OPTION_AUTH_CERT_LENGTH,
                      auth_issuer_buffer.GetDataLen() + auth_serial_buffer.GetDataLen() + 12)) {
            has_option_auth_length = true;
            if (__bint32(marc_options[i]->tag) == OPTION_AUTH_CERT_LENGTH) continue;
        }
        
        if (AddOption(new_marc_options, marc_options[i], has_option_enc_length, OPTION_ENC_CERT_LENGTH,
                      enc_issuer_buffer.GetDataLen() + enc_serial_buffer.GetDataLen() + 12)) {
            has_option_enc_length = true;
            if (__bint32(marc_options[i]->tag) == OPTION_ENC_CERT_LENGTH) continue;
        }
        
        /* The option has not yet been added. */
        new_option = (marc_option_t*) malloc(sizeof(marc_option_t));
        new_option->tag = marc_options[i]->tag;
        new_option->value = marc_options[i]->value;
        new_marc_options.Add(new_option);
        
        /* The option booleans (flags) are added following the order of the option set. */
        if (__bint32(new_option->tag) >= MARC_OPTION_FLAG_MIN && __bint32(new_option->tag) < MARC_OPTION_FLAG_MAX) {
            new_marc_option_flags.AppendByte(*(marc_option_flags + i));
        }
    }
    
    new_marc_ext_header.options_count = __to_bint16(new_marc_options.size());
    new_marc_ext_header.options_size = __to_bint32(new_marc_options.size()*8 +12);
    new_revision = __to_bint32(__bint32(this->revision) + 1);
    
    /* Copy and change ints (BE) for data header. */
    marc_data_header_t new_marc_data_header = *this->marc_data_header;
    memcpy(new_marc_data_header.encryption_algorithm, "A256", 4);
    memcpy(new_marc_data_header.hash_algorithm, "S256", 4);
    
    /* Increment last updated by 1. */
    new_marc_data_header.last_updated = __to_bint32(__bint32(new_marc_data_header.last_updated)+1);
    //new_marc_data_header.revision = __to_bint32(__bint32(new_marc_data_header.revision)+1);

    /* Required option-data flags (all must include a revision increment). */
    has_option_encrypt_outgoing = false;
    has_option_sign_outgoing = false;
    has_option_include_certs = false;
    has_option_send_signed = false;
    has_option_auth_length = false;
    has_option_enc_length = false;
    
    /* Similar to the option set, add each option data (a revision count). */
    MarcOptionDataArray new_marc_option_datas;
    marc_option_data_t *new_option_data;
    for (size_t i = 0; i < marc_option_datas.size(); ++i) {
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_sign_outgoing, OPTION_SIGN_OUTGOING, new_revision)) {
            has_option_sign_outgoing = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_SIGN_OUTGOING) continue;
        }
        
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_include_certs, OPTION_INCLUDE_CERTS, new_revision)) {
            has_option_include_certs = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_INCLUDE_CERTS) continue;
        }
        
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_send_signed, OPTION_SEND_SIGNED_CLEAR, new_revision)) {
            has_option_send_signed = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_SEND_SIGNED_CLEAR) continue;
        }
        
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_encrypt_outgoing, OPTION_ENCRYPT_OUTGOING, new_revision)) {
            has_option_encrypt_outgoing = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_ENCRYPT_OUTGOING) continue;
        }
        
        /* Option-lengths for certificate data. */
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_auth_length, OPTION_AUTH_CERT_LENGTH, new_revision)) {
            has_option_auth_length = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_AUTH_CERT_LENGTH) continue;
        }
        
        if (AddOptionData(new_marc_option_datas, marc_option_datas[i], has_option_enc_length, OPTION_ENC_CERT_LENGTH, new_revision)) {
            has_option_enc_length = true;
            if (__bint32(marc_option_datas[i]->tag) == OPTION_ENC_CERT_LENGTH) continue;
        }
        
        /* The option revision has not yet been added. */
        new_option_data = (marc_option_data_t*) malloc(sizeof(marc_option_data_t));
        new_option_data->tag = marc_option_datas[i]->tag;
        new_option_data->value1 = 0;
        new_option_data->value2 = marc_option_datas[i]->value2;
        new_marc_option_datas.Add(new_option_data);
    }
    
    /* Assemble the new identity data, to calcuate the data size. */
    wxMemoryBuffer new_data_buffer;
    
    /* Write the data header. */
    new_data_buffer.AppendData(&new_marc_data_header, sizeof(marc_data_header_t));
    new_data_buffer.AppendData(new_marc_option_flags.GetData(), new_marc_option_flags.GetDataLen());
    
    /* Write authentication certificate blob (marc_cert_t) structure. */
    WriteBE32(new_data_buffer, __to_bint32(1));
    WriteBE32(new_data_buffer, __to_bint32(auth_serial_buffer.GetDataLen()));
    WriteBE32(new_data_buffer, __to_bint32(auth_issuer_buffer.GetDataLen()));
    new_data_buffer.AppendData(auth_serial_buffer.GetData(), auth_serial_buffer.GetDataLen());
    new_data_buffer.AppendData(auth_issuer_buffer.GetData(), auth_issuer_buffer.GetDataLen());

    /* Write encipherment certificate blob (marc_cert_t) structure. */
    WriteBE32(new_data_buffer, __to_bint32(1));
    WriteBE32(new_data_buffer, __to_bint32(enc_serial_buffer.GetDataLen()));
    WriteBE32(new_data_buffer, __to_bint32(enc_issuer_buffer.GetDataLen()));
    new_data_buffer.AppendData(enc_serial_buffer.GetData(), enc_serial_buffer.GetDataLen());
    new_data_buffer.AppendData(enc_issuer_buffer.GetData(), enc_issuer_buffer.GetDataLen());
    
    /* Add each of the option data values (including the revision set). */
    for (size_t i = 0; i < marc_option_strings.size(); ++i) {
        //WriteBE32(new_data_buffer, marc_option_strings[i]->tag);
        if (__bint32(marc_option_strings[i]->tag) == OPTION_DATA) {
            /* Fill in with option revision (data) structures for the option data tag. */
            //WriteBE32(new_data_buffer, new_marc_option_datas.size() * sizeof(marc_option_data_t));
            for (size_t j = 0; j < new_marc_option_datas.size(); ++j) {
                WriteBE32(new_data_buffer, new_marc_option_datas[j]->tag);
                WriteBE32(new_data_buffer, new_marc_option_datas[j]->value1);
                WriteBE32(new_data_buffer, new_marc_option_datas[j]->value2);
            }
            continue;
        }
        
        if (__bint32(marc_option_strings[i]->tag) == OPTION_REVISION) {
            WriteBE32(new_data_buffer, 0);
            WriteBE32(new_data_buffer, new_revision);
            continue;
        }
        
        new_data_buffer.AppendData(marc_option_strings[i]->value, __bint32(marc_option_strings[i]->length));
    }
    
    new_marc_ext_header.data_size = __to_bint32(new_data_buffer.GetDataLen());
    
    /* Now the the data section is sized, fill in all of the (content), which is not the header. */
    wxMemoryBuffer new_content_buffer;
    
    new_content_buffer.AppendData(&new_marc_ext_header, sizeof(marc_extended_header_t));
    for (size_t i = 0; i < new_marc_options.size(); ++i) {
        WriteBE32(new_content_buffer, new_marc_options[i]->tag);
        
        if (__bint32(new_marc_options[i]->tag) == OPTION_DATA) {
            /* Option data contains the length of option revisions (32bit tag + 64bit revision). */
            WriteBE32(new_content_buffer, __to_bint32(new_marc_option_datas.size() * (sizeof(uint32_t) * 3)));
        } else {
            WriteBE32(new_content_buffer, new_marc_options[i]->value);
        }
    }
    new_content_buffer.AppendData(new_data_buffer.GetData(), new_data_buffer.GetDataLen());
    new_data_buffer.Clear();
    
    /* Calculate CRC on content, write to header, along with updated size. */
    new_marc_header.size1 = __to_bint32(new_content_buffer.GetDataLen());
    new_marc_header.size2 = new_marc_header.size1;
    new_marc_header.crc = __to_bint32(buffer_crc32(new_content_buffer));
    
    wxMemoryBuffer new_marc_buffer;
    
    new_marc_buffer.AppendData(&new_marc_header, sizeof(marc_header_t));
    new_marc_buffer.AppendData(new_content_buffer.GetData(), new_content_buffer.GetDataLen());
    new_content_buffer.Clear();
    
    
    DEBUG_LOG(_("%s"), wxBase64Encode(new_marc_buffer));
    return true;
}

bool AppOSX_OutlookIdentity::ClearCerts()
{
    return true;
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
    
    //marc_data_header = (marc_data_header_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
    //marc_offset += sizeof(marc_data_header_t);
    
    marc_option_string_t *marc_option_string;
    
    /* Read in option labels or field-type data (account_type, hash/enc types, last updated). */
    for (uint32_t i = 0; i < __bint16(marc_ext_header->options_count); ++i) {
        marc_option = marc_options[i];
        if (__bint32(marc_option->tag) < MARC_OPTION_FIELD_MIN || __bint32(marc_option->tag) >= MARC_OPTION_FIELD_MAX) {
            continue;
        }
        
        marc_option_string = new marc_option_string_t;
        marc_option_string->tag = marc_option->tag;
        marc_option_string->length = marc_option->value;
        marc_option_string->value = (uint8_t*) ((uint8_t*) raw_data.GetData() + marc_offset);
        marc_option_strings.Add(marc_option_string);
        
        marc_offset += __bint32(marc_option_string->length);
    }
    
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
        
        if (__bint32(marc_option->tag) == OPTION_REVISION) {
            this->revision = *((uint32_t*) marc_option_string->value + 1);
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
