
/* Used by all code */

#ifndef H_SeruroCrypto
#define H_SeruroCrypto

/* Class inherits from OS-specific? */

#include "SeruroCryptoMSW.h"

#if defined(__WXMSW__)
class SeruroCrypto : public SeruroCryptoMSW
#endif
{
public:
	SeruroCrypto() {}

};

#endif
