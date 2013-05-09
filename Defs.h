#ifndef H_SeruroDefs
#define H_SeruroDefs

/* Remove when finished developing. */
#ifndef __WXDEBUG__
#define __WXDEBUG__ 1
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

#define SERURO_DEFAULT_PORT 443 /* Todo: maybe a string representation.*/
#define SERURO_CONFIG_NAME  "SeruroClient.config"
#define SERURO_TOKENS_FILE	"tokens"
#define SERURO_APP_NAME     "Seruro Client"

//#define SERURO_DEFAULT_PORT 443
#define SERURO_DEFAULT_USER_AGENT "Client/1.0"
//#define SERURO_AUTH_API_SYNC " sync"

#define SERURO_SECURITY_OPTIONS_NONE    0x00
#define SERURO_SECURITY_OPTIONS_TLS12	0x01
#define SERURO_SECURITY_OPTIONS_STRONG	0x02
#define SERURO_SECURITY_OPTIONS_CLIENT  0x04
#define SERURO_SECURITY_OPTIONS_DATA	0x08

/* Used in SeruroServerAPI. */
#define SERURO_API_ERROR_INVALID_AUTH	"Invalid authentication token."

#define SERURO_API_OBJECT_SESSION_CREATE "/api/sessions/create"
#define SERURO_API_OBJECT_GETP12 "/api/seruro/getP12"
#define SERURO_API_OBJECT_GETCA "/api/seruro/getCA"
#define SERURO_API_OBJECT_GETCERT "/api/seruro/getCert"
#define SERURO_API_OBJECT_SEARCH "/api/seruro/search"
#define SERURO_API_OBJECT_GETCRL "/api/seruro/getCRL"

#define SERURO_API_AUTH_FIELD_EMAIL "user_login[email]"
#define SERURO_API_AUTH_FIELD_PASSWORD "user_login[password]"

#define TEXT_ACCOUNT_LOGIN "Please enter your Account Information \
to log into the Seruro Server: "
//#define TEXT_ACCOUNT_LOGIN_LABEL "Account Information"
#define TEXT_DECRYPT_METHOD_EMAIL "Please enter the 'password' you \
received via email."
#define TEXT_DECRYPT_METHOD_SMS "Please enter the 'password' you \
received as a text message."
#define TEXT_DECRYPT_EXPLAINATION "This is NOT your Seruro Account 'password'. \
This password will only be used to setup your personal certificates and does \
not need to be remembered or saved. You may delete the password immediately \
after you enter it: "

#endif 