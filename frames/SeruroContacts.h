
#ifndef H_SeruroPanelContacts
#define H_SeruroPanelContacts

#include "SeruroFrame.h"

//#include "components/CheckedListCtrl.h"
//#include "../api/SeruroServerAPI.h"

#include "components/ContactList.h"

#include "../wxJSON/wx/jsonval.h"
#include "../api/SeruroStateEvents.h"

#include <wx/button.h>

class SeruroPanelContacts : public SeruroPanel, public ContactList
{
public:
	SeruroPanelContacts(wxBookCtrlBase *book);

private:
    wxButton *recheck_button;
    
	void OnContactStateChange(SeruroStateEvent &event);
	void OnServerStateChange(SeruroStateEvent &event);
    void OnIdentityStateChange(SeruroStateEvent &event);
    
    /* Add/remove recheck button. */
    void OnOptionChange(SeruroStateEvent &event);
    void OnRecheckContacts(wxCommandEvent &event);
    
    void AlignList();
    
    DECLARE_EVENT_TABLE()
};

#endif