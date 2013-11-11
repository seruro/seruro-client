
#define SERURO_APP_NAME "Seruro Client"
#ifndef SERURO_VERSION
/* SERURO_VERSION should be set by compiler options. */
#define SERURO_VERSION  "0.2.2.1"
#endif

/* For alpha or beta-testing. */
#define RELEASE_DEBUG 1
/* For masking the .seruro.com in server name. */
#define SERURO_CLOUD_CLIENT 1

#undef __VLD__

#if defined(__WXDEBUG__) || defined(RELEASE_DEBUG)
#define DEBUG_REPORT_SERVER "alpha.seruro.com"
#define DEBUG_REPORT_PORT   "9001"
#define DEBUG_REPORT_URL    "/report"
#endif
