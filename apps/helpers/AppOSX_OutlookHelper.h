
#ifndef H_AppOSX_OutlookHelper
#define H_AppOSX_OutlookHelper



#include <Foundation/Foundation.h>


@interface AppOSX_OutlookHelper : NSObject

/* Helper Objective-C class methods for scripting resources (AppleScript). */
+(BOOL) addContactWithEmail:(NSString*)email firstName:(NSString*)firstName lastName:(NSString*)lastName;
+(BOOL) addContactCertificatesWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert;
+(BOOL) haveContactWithEmail:(NSString*)email;
+(BOOL) removeContactWithEmail:(NSString*)email;

/* Identities are parsed manually, no AppleScript used. */
//+(BOOL) addIdentityWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert;
//+(BOOL) haveIdentityWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert;
//+(BOOL) removeIdentityWithEmail:(NSString*)email;

//+(NSString*) getContactAuthCertWithEmail:(NSString*)email;
//+(NSString*) getContactEncCertWithEmail:(NSString*)email;
//+(NSString*) getIdentityAuthCertWithEmail:(NSString*)email;
//+(NSString*) getIdentityEncCertWithEmail:(NSString*)email;

/* Testing bridge method. */
//+(void) addContact;

@end

#endif
