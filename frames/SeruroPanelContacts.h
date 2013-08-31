
#ifndef H_SeruroPanelContacts
#define H_SeruroPanelContacts

#include "SeruroFrame.h"

//#include "components/CheckedListCtrl.h"
//#include "../api/SeruroServerAPI.h"

#include "components/ContactList.h"

#include "../wxJSON/wx/jsonval.h"
#include "../api/SeruroStateEvents.h"

class SeruroPanelContacts : public SeruroPanel, public ContactList
{
public:
	SeruroPanelContacts(wxBookCtrlBase *book);

private:
	void OnContactStateChange(SeruroStateEvent &event);
    
    void AlignList();
};

#endif