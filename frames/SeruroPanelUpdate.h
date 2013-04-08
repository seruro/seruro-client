
#ifndef H_SeruroPanelUpdate
#define H_SeruroPanelUpdate

#include "SeruroFrame.h"

// Define a new frame type: this is going to be our main frame
class SeruroPanelUpdate : public SeruroPanel
{
public:
    // ctor(s)
	SeruroPanelUpdate(wxBookCtrlBase *book) : 
	  SeruroPanel(book, wxT("Update")) {}
};

#endif