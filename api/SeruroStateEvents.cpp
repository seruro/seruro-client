
#include "SeruroStateEvents.h"

wxDEFINE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);

SeruroStateEvent::SeruroStateEvent(int type, int action) 
	: wxCommandEvent(SERURO_STATE_CHANGE, type)
{
		/* Initialize the data structure. */
		state_data = wxJSONValue(wxJSONTYPE_OBJECT);
		state_data["action"] = action;
}

/* Not much else... */
