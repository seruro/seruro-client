
/* First thing, detect OS */
#if defined(__WXMAC__)

#include <wx/log.h>

/* Note: requires C++ flags: "-framework Security" */
//#include <Security/Security.h>
//#include <netinet/ip.h>
//#include <netdb.h>
//#include <sys/socket.h>

#include <CFNetwork/CFNetwork.h>

/* Code goes here */
#include "../SeruroConfig.h"
#include "SeruroCryptoMAC.h"

#if 0
OSStatus _SSLWrite (SSLConnectionRef connection, const void *data, size_t *length)
{
    wxLogMessage(wxT("SeruroCrypto::_SSLWrite> size (%d)"), *length);
    return (OSStatus) 0;
}

OSStatus _SSLRead (SSLConnectionRef connection, void *data, size_t *length)
{
    wxLogMessage(wxT("SeruroCryto::_SSLRead> size (%d) data: (%s)"), *length, (char *)data);
    return (OSStatus) 0;
}
#endif

void SeruroCryptoMAC::OnInit()
{
	wxLogStatus(wxT("SeruroCrypt::MAC> Initialized"));
    
	//TLSRequest(none, 0, verb, object, data); /* SERURO_SECURITY_OPTIONS_DATA */
}

/* Errors should be events. */

bool SeruroCryptoMAC::InstallP12(wxMemoryBuffer &p12, wxString &password)
{
    return false;
}

wxString SeruroCryptoMAC::TLSRequest(wxString p_serverAddress,
    int p_options, wxString p_verb, wxString p_object)
{
	wxString wx_data;
	return TLSRequest(p_serverAddress, p_options, p_verb, p_object, wx_data);
}

wxString SeruroCryptoMAC::TLSRequest(wxString p_serverName,
    int p_options, wxString p_verb, wxString p_object, wxString p_data)
{
	wxString response("");
    //int results;
    
#if 0
    SSLContextRef ssl_context;
    
    wxLogMessage(wxT("SeruroCrypto::TLS> Received, options: %d."), p_options);
    
    /* Get server address from p_serverAddress (create socket) */
    struct sockaddr_in address;
    struct hostent *server_entry;
    struct in_addr server_address;
    int client_socket;
    
    bool is_hostname = false;
    const char *server_name = p_serverName.mbc_str();
    for (size_t i = 0; i < p_serverName.size(); i++) {
        /* Loop through server address, searching for a non '.' and non (0-9) character. */
        if (server_name[i] != '.' && (server_name[i] < '0' || server_name[i] > '9')) {
            is_hostname = true;
            break;
        }
    }
    
    if (is_hostname) {
        /* Find IP from hostname */
        server_entry = gethostbyname(server_name);
        if (! server_entry) {
            wxLogMessage(wxT("SeruroCrypto::TLS> gethostbyname (%s) failed."), p_serverName);
            goto bailout;
        }
        memcpy((void*) &server_address, server_entry->h_addr, sizeof(struct in_addr));
    }
    
    client_socket = socket(PF_INET, SOCK_STREAM, 0);
    address.sin_addr = server_address;
    address.sin_port = htons((u_short) SERURO_DEFAULT_PORT);
    address.sin_family = PF_INET;
    
    results = connect(client_socket, (struct sockaddr *) &address, sizeof(struct sockaddr_in));
    if (results) {
        wxLogMessage(wxT("SeruroCrypto::TLS> socket connect error (%d)"), results);
        goto bailout;
    }
    
    /* Create session & connection */
    results = SSLNewContext(false, &ssl_context);
    results = SSLSetConnection(ssl_context, (SSLConnectionRef) client_socket);
    results = SSLSetIOFuncs(ssl_context, _SSLRead, _SSLWrite);
    results = SSLSetEnableCertVerify(ss_context, true);
#endif
    
    bool status;
    CFIndex bytes_read;
    /* Used to read data from stream sychronously. */
    UInt8 *read_buffer;
    /* HTTP response data structure. */
    CFHTTPMessageRef http_response;
    
    /* Set TLS version (hopfully TLS1.2 (fallback to TLS1), fail if not that (???). */
    wxString url_string = wxString(wxT("https://") + p_serverName + wxT(":") + wxT("443") + p_object);
    CFStringRef url_cfstring = CFStringCreateWithCString(kCFAllocatorDefault, url_string.mbc_str(), kCFStringEncodingMacRoman);
    CFURLRef url = CFURLCreateWithString(kCFAllocatorDefault, url_cfstring, NULL);
    
    /* Get user_agent from config "SERURO_DEFAULT_USER_AGENT" (???) */

    /* Set VERB and OBJECT (if post data exists, add.*/
    CFStringRef cfverb = CFStringCreateWithCString(kCFAllocatorDefault, p_verb.mbc_str(), kCFStringEncodingMacRoman);
    CFHTTPMessageRef request = CFHTTPMessageCreateRequest(kCFAllocatorDefault, cfverb, url, kCFHTTPVersion1_1);
    
    /* Todo: consider adding Content-Type: x-form-encoded (application/json)?... */
    if (p_options & SERURO_SECURITY_OPTIONS_DATA) {
        wxLogMessage(wxT("SeruroCrypto::TLS> POST, must add headers."));
        CFHTTPMessageSetHeaderFieldValue(request, CFSTR("Content-Type"), CFSTR("application/json"));
        /* Todo: cleanup possible auth data in p_data parameter. */
        //CFHTTPMessageSetBody(<#CFHTTPMessageRef message#>, <#CFDataRef bodyData#>);
    }
    
    /* Add request headers "Accept: application/json", calculate length. */
    CFHTTPMessageSetHeaderFieldValue(request, CFSTR("Accept"), CFSTR("application/json"));
    
    /* This will make a copy of the request, which can be released afterward (this will open the request!). */
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
    read_buffer = (UInt8 *) malloc(response.size() * sizeof(UInt8));
    memcpy(read_buffer, response.mbc_str(), response.size() * sizeof(UInt8));
    
    http_response = CFHTTPMessageCreateEmpty(kCFAllocatorDefault, false);
    status = CFHTTPMessageAppendBytes(http_response, read_buffer, response.size());
    delete read_buffer;
    
    /* Check the response code, expecting 200, but 401, 301 are acceptable. */
    response_code = CFHTTPMessageGetResponseStatusCode(http_response);
    wxLogMessage(wxT("SeruroCrypto::TLS> response code (%d)"), response_code);
    
    /* Now copy only the body data. */
    http_response_body = CFHTTPMessageCopyBody(http_response);
    read_buffer = (UInt8 *) malloc(CFDataGetLength(http_response_body) * sizeof(UInt8));
    CFDataGetBytes(http_response_body, CFRangeMake(0, CFDataGetLength(http_response_body)), read_buffer);
    
    /* Finally convert to wxString */
    response = wxString::FromAscii((char *) read_buffer, CFDataGetLength(http_response_body));
    
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

#endif