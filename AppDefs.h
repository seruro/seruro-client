
#define SERURO_APP_NAME "Seruro Client"
#ifndef SERURO_VERSION
/* SERURO_VERSION should be set by compiler options. */
#define SERURO_VERSION  "0.0.1"
#endif

#if defined(__WXDEBUG__) || defined(DEBUG)
#define DEBUG_REPORT_SERVER "beta.seruro.com"
#define DEBUG_REPORT_PORT   "9001"
#define DEBUG_REPORT_URL    "/report"
#endif
