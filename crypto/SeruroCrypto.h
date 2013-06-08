
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

	virtual void OnInit();

	/* wxJSONValue, For easy overloading access. */
	virtual wxString TLSRequest(wxJSONValue params);

	virtual bool InstallP12(wxMemoryBuffer &p12, wxString &password);

	virtual bool InstallCA(wxMemoryBuffer &ca);
	virtual bool InstallCert(wxMemoryBuffer &cert);

	virtual bool RemoveIdentity(wxString thumbprint);
	virtual bool RemoveCA(wxString thumbprint);
	virtual bool RemoveCerts(wxArrayString thumbprints);
};

#endif
