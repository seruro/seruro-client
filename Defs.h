#ifndef H_SeruroDefs
#define H_SeruroDefs

/* Remove when finished developing. */
#ifndef __WXDEBUG__
#define __WXDEBUG__ 1
#endif
#ifndef NDEBUG
#define NDEBUG 1
#endif

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



#endif 