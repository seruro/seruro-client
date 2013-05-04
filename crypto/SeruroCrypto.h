
/* Used by all code */

#ifndef H_SeruroCrypto
#define H_SeruroCrypto

#include "../Defs.h"

/* Class inherits from OS-specific? */

#include "SeruroCryptoMSW.h"
#include "SeruroCryptoMAC.h"

#if defined(__WXMSW__)
class SeruroCrypto : public SeruroCryptoMSW
#endif
#if defined(__WXMAC__)
class SeruroCrypto : public SeruroCryptoMAC
#endif
{
public:
	SeruroCrypto() {}
};

#endif
