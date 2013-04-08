
#ifndef H_SeruroPanelDecrypt
#define H_SeruroPanelDecrypt

#include "SeruroFrame.h"

// Define a new frame type: this is going to be our main frame
class SeruroPanelDecrypt : public SeruroPanel
{
public:
    // ctor(s)
	SeruroPanelDecrypt(wxBookCtrlBase *book) : 
	  SeruroPanel(book, wxT("Decrypt")) {}
};

#endif