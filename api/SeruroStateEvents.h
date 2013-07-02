
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
	STATE_TYPE_CERTIFICATE,
};

//IMPLEMENT_DYNAMIC_CLASS( SeruroStateEvent, wxCommandEvent )
//wxDECLARE_EVENT_TYPE(SERURO_STATE_CHANGE, -1);
//class SeruroStateEvent;
//wxDECLARE_EVENT(SERURO_STATE_CHANGE, wxCommandEvent);

//wxDEFINE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);
//extern const wxEventType SERURO_STATE_CHANGE;

class SeruroStateEvent : public wxCommandEvent
{
public:
	SeruroStateEvent(int type= 0, int action=0);

	SeruroStateEvent(const SeruroStateEvent &event)
		: wxCommandEvent(event) { this->SetStateChange(event.GetStateChange()); }
	virtual wxEvent *Clone() const { return new SeruroStateEvent(*this); }
	
	wxJSONValue GetStateChange() const { return state_data; }
	void SetStateChange(wxJSONValue state) { state_data = state; }

	/* A state change will always have a server_name. */
	wxString GetServerName() { return state_data["server_name"].AsString(); }
	void SetServerName(wxString server_name) { state_data["server_name"] = server_name; }
	wxString GetValue(wxString key) { return state_data[key].AsString(); }
	void SetValue(wxString key, wxString value) { state_data[key] = value; }
	/* Every state has an associated action. */
	int GetAction() { return state_data["action"].AsInt(); }
	void SetAction(int action) { state_data["action"] = action; }

	//DECLARE_DYNAMIC_CLASS( SeruroStateEvent )

private:
	/* Should be formatted:
	 * {"server_name"
	 *  ["address"]
	 *  "action" : {"update", "remove", "add"}
	 * }
	 */
	wxJSONValue state_data;
};

//wxDEFINE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);
wxDECLARE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);

//DEFINE_EVENT_TYPE(SERURO_STATE_CHANGE);

/* Define a event type for State changes. */
typedef void (wxEvtHandler::*SeruroStateEventFunction) (SeruroStateEvent &);
#define SeruroStateEventHandler(func) wxEVENT_HANDLER_CAST(SeruroStateEventFunction, func)
//#define 

/*
#define SeruroStateEventHandler(func) \
    (wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
    wxStaticCastEvent(SeruroRequestEventFunction, &func)

#define EVT_SERURO_STATE(type, fn) \
	DECLARE_EVENT_TABLE_ENTRY(SERURO_API_RESULT, type, -1, \
	(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) \
	wxStaticCastEvent(SeruroStateEventFunction, &fn), (wxObject*) NULL),
*/

#endif 