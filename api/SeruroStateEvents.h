
#ifndef H_SeruroStateEvents
#define H_SeruroStateEvents

#include "../wxJSON/wx/jsonval.h"

#include <wx/event.h>

/* State events are currently defined as server/account change events.
 * Variable parts of the application can handle these events.
 */

enum state_action_t
{
	STATE_ACTION_REMOVE = 0x01,
	STATE_ACTION_ADD    = 0x02,
	STATE_ACTION_UPDATE = 0x03
};

/* May later include identity/CA. */
enum state_types_t
{
	STATE_TYPE_SERVER,
	STATE_TYPE_ACCOUNT,
	STATE_TYPE_CERTIFICATE
};

DECLARE_EVENT_TYPE(SERURO_STATE_CHANGE, -1);

class SeruroStateEvent : public wxCommandEvent
{
public:
	SeruroStateEvent(int id= 0, wxEventType command_type = SERURO_STATE_CHANGE)
		: wxCommandEvent(command_type, id) {}

	SeruroStateEvent(const SeruroStateEvent &event)
		: wxCommandEvent(event) { this->SetStateChange(event.GetStateChange()); }
	
	wxJSONValue GetStateChange() const { return state_data; }
	void SetStateChange(wxJSONValue state) { state_data = state; }

	/* A state change will always have a server_name. */
	wxString GetServerName() { return state_data["server_name"].AsString(); }
	//wxString GetAction() const
	int GetAction() { return state_data["action"].AsInt(); }

private:
	/* Should be formatted:
	 * {"server_name"
	 *  ["address"]
	 *  "action" : {"update", "remove", "add"}
	 * }
	 */
	wxJSONValue state_data;
};

/* Define a event type for State changes. */
typedef void (wxEvtHandler::*SeruroStateEventFunction) (SeruroStateEvent &);

#define SeruroStateEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
    wxStaticCastEvent(SeruroRequestEventFunction, &func)

#define EVT_SERURO_STATE(type, fn) \
	DECLARE_EVENT_TABLE_ENTRY(SERURO_API_RESULT, type, -1, \
	(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
	wxStaticCastEvent(SeruroStateEventFunction, &fn), (wxObject*) NULL),

#endif 