
#include "SeruroStateEvents.h"

//DEFINE_EVENT_TYPE(SERURO_STATE_CHANGE);
//wxDEFINE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);
//const wxEventType SERURO_STATE_CHANGE = wxNewEventType();
wxDEFINE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);

SeruroStateEvent::SeruroStateEvent(int type, int action) 
	: wxCommandEvent(SERURO_STATE_CHANGE, type)
{
		/* Initialize the data structure. */
		state_data = wxJSONValue(wxJSONTYPE_OBJECT);
		state_data["action"] = action;
}

/* Not much else... */
