#ifndef H_SeruroDefs
#define H_SeruroDefs

/* Remove when finished developing. */
#ifndef __WXDEBUG__
#define __WXDEBUG__ 1
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#define SERURO_APP_NAME     "Seruro Client"
#if defined(__WXMAC__)
#define SERURO_APP_DEFAULT_WIDTH  650
#else
#define SERURO_APP_DEFAULT_WIDTH  550
#endif
#define SERURO_APP_DEFAULT_HEIGHT 500

/* When a server name is displayed, display the host and port as well. */
#define SERURO_DISPLAY_SERVER_INFO true

#define SERURO_DEFAULT_PORT "443" /* Todo: maybe a string representation.*/
/* File name to store and fetch user configuration data from. */
#define SERURO_CONFIG_NAME  "SeruroClient.config"
/* File name to store and fetch user authentication tokens from. */
#define SERURO_TOKENS_FILE	"tokens"
/* User agent to use for Server API requests. */
#define SERURO_DEFAULT_USER_AGENT "Client/1.0"

/* Settings view related definitions. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_MIN_WIDTH 175
#else
#define SERURO_SETTINGS_TREE_MIN_WIDTH 150
#endif
/* The settings tree event control id. */
#define SERURO_SETTINGS_TREE_ID 1009

/* OSX has larger indents. */
#if defined(__WXMAC__)
#define SERURO_SETTINGS_TREE_INDENT 8
#else
#define SERURO_SETTINGS_TREE_INDENT 12
#endif

/* Width to wrap message text in setting views. (WIDTH-TREE_MIN) */
//#define SERURO_SETTINGS_VIEW_SOFT_WRAP

/* Bit settings for TLS requests. */
#define SERURO_SECURITY_OPTIONS_NONE    0x00
#define SERURO_SECURITY_OPTIONS_TLS12	0x01
#define SERURO_SECURITY_OPTIONS_STRONG	0x02
#define SERURO_SECURITY_OPTIONS_CLIENT  0x04
#define SERURO_SECURITY_OPTIONS_DATA	0x08

/* Used in the Server API to determine an expired or invalid token. */
#define SERURO_API_ERROR_INVALID_AUTH	"Invalid authentication token."

/* Server API routes, or in HTTP diction "objects". */
#define SERURO_API_OBJECT_SESSION_CREATE "/api/sessions/create"
#define SERURO_API_OBJECT_GETP12 "/api/seruro/getP12"
#define SERURO_API_OBJECT_GETCA "/api/seruro/getCA"
#define SERURO_API_OBJECT_GETCERT "/api/seruro/getCert"
#define SERURO_API_OBJECT_SEARCH "/api/seruro/search"
#define SERURO_API_OBJECT_GETCRL "/api/seruro/getCRL"

/* Seruro API authentication POST fields. */
#define SERURO_API_AUTH_FIELD_EMAIL "user_login[email]"
#define SERURO_API_AUTH_FIELD_PASSWORD "user_login[password]"
#define SERURO_API_AUTH_TOKEN_PARAMETER "token"

/* Textual data, stored here to allow easy 'future' translation. */
#define TEXT_ACCOUNT_LOGIN "Please enter your Account Information \
to log into the Seruro Server: "
#define TEXT_DECRYPT_METHOD_EMAIL "Please enter the 'password' you \
received via email."
#define TEXT_DECRYPT_METHOD_SMS "Please enter the 'password' you \
received as a text message."
#define TEXT_DECRYPT_EXPLAINATION "This is NOT your Seruro Account 'password'. \
This password will only be used to setup your personal certificates and does \
not need to be remembered or saved. You may delete the password immediately \
after you enter it: "

#endif 