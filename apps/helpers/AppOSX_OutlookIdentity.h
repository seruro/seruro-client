
#ifndef H_AppOSX_OutlookIdentity
#define H_AppOSX_OutlookIdentity

#if defined(__WXOSX__) || defined(__WXMAC__)

/* This class provides a utility into Microsoft's MaRC identity file (Mail Account).
 * The MaRC parsing is an example of horrible interfaces to controlling S/MIME characteristics.
 */

#include <wx/string.h>
#include <wx/memory.h>

#include "../../wxJSON/wx/jsonval.h"

#include <wx/dynarray.h>

enum {
    /* Boolean options (a lot missing. */
    OPTION_ENCRYPT_OUTGOING  = 0xb1a,
    OPTION_SIGN_OUTGOING     = 0xb17,
    OPTION_INCLUDE_CERTS     = 0xb18,
    OPTION_SEND_SIGNED_CLEAR = 0xb19,
    
    /* Certificate data options. */
    OPTION_AUTH_CERT_LENGTH  = 0xd10,
    OPTION_ENC_CERT_LENGTH   = 0xd11,
    
    /* Data length options. */
    OPTION_EMAIL_ADDRESS     = 0x1e10,
    OPTION_IMAP_ADDRESS      = 0x1e11,
    OPTION_SMTP_ADDRESS      = 0x1e13,
    OPTION_ACCOUNT_NAME_U    = 0x1f10,
    OPTION_FULL_NAME_U       = 0x1f12,
    OPTION_EMAIL_ADDRESS_U   = 0x1f13,
    
    /* Option value data section. */
    OPTION_DATA              = 0x2015
};

/* A list of option tags which must be incremented when saving an identity. */
#define INCREMENTOR_OPTIONS {0xb12, 0xb16, 0x1d12, 0x1d14, 0x1e13}

/* Size of each tag/value pair in options set. */
#define MARC_OPTION_BYTE_SIZE 8
#define MARC_OPTION_DATA_SIZE 12
#define MARC_OPTION_FLAG_MIN 0xb00
#define MARC_OPTION_FLAG_MAX 0xc00
#define MARC_OPTION_STRING_MIN 0x1e00
#define MARC_OPTION_STRING_MAX 0x3000

typedef struct
{
    unsigned char magic[4];
    uint32_t size1;
    uint32_t size2;
    uint32_t crc;
    uint32_t _unknown; /* might be the version. */
} marc_header_t;

typedef struct
{
    uint16_t options_count;
    uint16_t _unknown;
    uint32_t options_size;
    uint32_t data_size;
} marc_extended_header_t;

typedef struct
{
    uint32_t tag;
    uint32_t value;
} marc_option_t;

/* Option strings are tags [0x1e00-0x3000] */
typedef struct
{
    uint32_t tag;
    uint32_t length;
    uint8_t *value;
} marc_option_string_t;

typedef struct
{
    size_t auth_cert_size;
    size_t enc_cert_size;
    
    uint8_t *auth_cert_data;
    uint8_t *enc_cert_data;
} marc_certs_t;

/* Option data (quad ints) proceed strings and their length is specified by 0x2015.
 * These values include their relevant tag, but are in the same order as the options.
 */
typedef struct
{
    uint32_t tag;
    uint64_t value;
} marc_option_data_t;

typedef struct
{
    unsigned char account_type[4];
    uint32_t account_int;
    uint32_t _unknown1[3];
    unsigned char encryption_algorithm[4];
    unsigned char hash_algorithm[4];
    uint32_t _unknown2;
    
    uint32_t last_updated;
} marc_data_header_t;

/* Create a dynamitc set of MARC options. */
WX_DEFINE_ARRAY(marc_option_t*, MarcOptionArray);
WX_DEFINE_ARRAY(marc_option_string_t*, MarcOptionStringArray);
WX_DEFINE_ARRAY(marc_option_data_t*, MarcOptionDataArray);



class AppOSX_OutlookIdentity
{
public:
    AppOSX_OutlookIdentity() {
        marc_parsed = false;
        account_parsed = false;
        
        marc_certs.auth_cert_size = 0;
        marc_certs.enc_cert_size = 0;
    }
    ~AppOSX_OutlookIdentity() {}
    
    void SetPath(wxString path);
    void SetData(void *data, size_t length);
    bool ParseMarc();
    
    /* Proccess marc data into account. */
    void ParseAccount();
    
    /* After the identity is parsed, accessors open. */
    wxJSONValue GetAccount();
    
private:
    wxMemoryBuffer raw_data;
    wxJSONValue account;
    
    /* Data structures for identity. */
    marc_header_t *marc_header;
    marc_extended_header_t *marc_ext_header;
    marc_data_header_t *marc_data_header;
    
    MarcOptionArray marc_options;
    uint8_t *marc_option_flags;
    
    marc_certs_t marc_certs;

    MarcOptionStringArray marc_option_strings;
    MarcOptionDataArray marc_option_datas;
    
    /* Determine if accessors are valid. */
    bool marc_parsed;
    bool account_parsed;
};

#endif /* OS Check */

#endif