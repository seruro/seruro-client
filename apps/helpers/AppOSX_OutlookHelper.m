
#include "AppOSX_OutlookHelper.h"

/* MDLS attributes:
 * kMDItemEmailAddresses - email address list
 * kMDItemTitle = full name
 * kMDItemDisplayName = full name
 * com_microsoft_outlook_lastName = last name
 * com_microsoft_outlook_firstName = first name
 */

@implementation AppOSX_OutlookHelper

+(BOOL) addContactWithEmail:(NSString*)address firstName:(NSString*)first_name lastName:(NSString*)last_name
{
    BOOL result = false;
    
    return result;
}

+(BOOL) addContactCertificatesWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert
{
    BOOL result = false;
    
    return result;
}

+(BOOL) haveContactWithEmail:(NSString*)email
{
    BOOL result = false;
    
    return result;
}

+(BOOL) removeContactWithEmail:(NSString*)email
{
    BOOL result = false;
    
    return result;
}

/*
+(BOOL) addIdentityWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert
{
    BOOL result = false;
    
    return result;
}

+(BOOL) haveIdentityWithEmail:(NSString*)email authCert:(id)auth_cert encCert:(id)enc_cert
{
    BOOL result = false;
    
    return result;
}

+(BOOL) removeIdentityWithEmail:(NSString*)email
{
    BOOL result = false;
    
    return result;
}
*/

@end