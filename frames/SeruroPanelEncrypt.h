
#ifndef H_SeruroPanelEncrypt
#define H_SeruroPanelEncrypt

#include "SeruroFrame.h"

// Define a new frame type: this is going to be our main frame
class SeruroPanelEncrypt : public SeruroPanel
{
public:
    // ctor(s)
	SeruroPanelEncrypt(wxBookCtrlBase *book) : 
	  SeruroPanel(book, wxT("Encrypt")) {}
};

#endif