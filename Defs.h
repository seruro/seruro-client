#ifndef H_SeruroDefs
#define H_SeruroDefs

/* Remove wxT warnings. */
#ifdef __WXMAC__
#pragma GCC diagnostic ignored "-Wwrite-strings"
//#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif

/* Remove when finished developing. */
//#ifndef __WXDEBUG__
//#define __WXDEBUG__ 1
//#endif
//#ifndef NDEBUG
//#define NDEBUG 1
//#endif
#include "AppDefs.h"

/* When a server name is displayed, display the host and port as well. */
#define SERURO_DISPLAY_SERVER_INFO 1
/* Enable/Disable the encrypt/decrypt tabs. */
#define SERURO_ENABLE_CRYPT_PANELS 0
#define SERURO_ENABLE_DEBUG_PANELS 0
/* Allow servers to exist without an account? */
#define SERURO_MUST_HAVE_ACCOUNT 1
/* Allow server configurations/additions with optional non-standard TLS ports. */
#define SERURO_ALLOW_CUSTOM_PORT 0
/* Allow the client to automatically download P12 containers if this is the first account. */
#define SERURO_ALLOW_AUTO_DOWNLOAD 0

#define SERURO_DEFAULT_PORT "443"
/* File name to store and fetch user configuration data from. */
#define SERURO_CONFIG_NAME  "SeruroClient.config"
/* File name to store and fetch user authentication tokens from. */
#define SERURO_TOKENS_FILE	"tokens"
/* User agent to use for Server API requests. */
#define SERURO_DEFAULT_USER_AGENT "SeruroClient/"SERURO_VERSION

/* The maximum amount of 'application panels' which are 'tabs'. 
 * This is used in the SeruroTray to identity the panel IDs. 
 */
#define SERURO_MAX_PANELS 10
#define SERURO_INPUT_MAX_LENGTH 256
#define SERURO_BUFFER_SIZE 256
#define SERURO_INPUT_WHITELIST \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.-_() "
#define SERURO_INPUT_HOSTNAME_WHITELIST \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890.-"
/* Todo: consider non-ascii characters. */
#define SERURO_INPUT_ADDRESS_WHITELIST \
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890._-@"

/* Width to wrap message text in setting views. (WIDTH-TREE_MIN) */
//#define SERURO_SETTINGS_VIEW_SOFT_WRAP

/* Bit settings for TLS requests. */
#define SERURO_SECURITY_OPTIONS_NONE    0x00
#define SERURO_SECURITY_OPTIONS_TLS12	0x01
#define SERURO_SECURITY_OPTIONS_STRONG	0x02
#define SERURO_SECURITY_OPTIONS_CLIENT  0x04
#define SERURO_SECURITY_OPTIONS_DATA	0x08

/* Used in the Server API to determine an expired or invalid token. */
#define SERURO_API_ERROR_SUCCESS		"Success."
#define SERURO_API_ERROR_INVALID_AUTH	"Invalid authentication token."
#define SERURO_API_ERROR_CONNECTION		"Connection failed."

/* Server API routes, or in HTTP diction "objects". */
#define SERURO_API_OBJECT_LOGIN	 "/api/sessions/create"
#define SERURO_API_OBJECT_P12S   "/api/seruro/p12s"
#define SERURO_API_OBJECT_CA	 "/api/seruro/ca"
#define SERURO_API_OBJECT_CERTS  "/api/seruro/certs"
#define SERURO_API_OBJECT_SEARCH "/api/seruro/search"
#define SERURO_API_OBJECT_CRL    "/api/seruro/crl"
/* A special API which does nothing, but allows auth checking. */
#define SERURO_API_OBJECT_PING	 "/api/seruro/ping"

/* Seruro API authentication POST fields. */
#define SERURO_API_AUTH_FIELD_EMAIL		"user_login[email]"
#define SERURO_API_AUTH_FIELD_PASSWORD	"user_login[password]"
#define SERURO_API_AUTH_TOKEN_PARAMETER "token"

/* Textual data, stored here to allow easy 'future' translation. */
#include "lang/en_US.h"

#endif 