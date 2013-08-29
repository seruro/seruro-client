
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
	STATE_ACTION_UPDATE = 0x03,
    
    /* Special, for Certificates/Identities. */
    STATE_ACTION_REVOKE = 0x11,
    
    /* Special, for Email applications. */
    STATE_ACTION_CLOSE  = 0x21,
};

/* May later include identity/CA. */
enum state_types_t
{
	STATE_TYPE_SERVER,      /* A Seruro server changed. */
	STATE_TYPE_ACCOUNT,     /* A Seruro account changed. */
	STATE_TYPE_CERTIFICATE, /* A Seruro certificate changed. */
    STATE_TYPE_IDENTITY,    /* A Seruro account identity changed. */
    
    STATE_TYPE_APPLICATION, /* An Email application changed. */
};

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
	wxString GetServerUUID() { return state_data["server_uuid"].AsString(); }
	void SetServerUUID(wxString server_uuid) { state_data["server_uuid"] = server_uuid; }
    wxString GetAccount() { return state_data["account"].AsString(); }
    void SetAccount(wxString account) { state_data["account"] = account; }
    
	wxString GetValue(wxString key) { return state_data[key].AsString(); }
	void SetValue(wxString key, wxString value) { state_data[key] = value; }
    bool HasValue(wxString key) { return state_data.HasMember(key); }
	/* Every state has an associated action. */
	int GetAction() { return state_data["action"].AsInt(); }
	void SetAction(int action) { state_data["action"] = action; }

private:
	/* Should be formatted:
	 * {"server_uuid"
	 *  ["address"]
	 *  "action" : {"update", "remove", "add"}
	 * }
	 */
	wxJSONValue state_data;
};

wxDECLARE_EVENT(SERURO_STATE_CHANGE, SeruroStateEvent);

/* Define a event type for State changes. */
typedef void (wxEvtHandler::*SeruroStateEventFunction) (SeruroStateEvent &);
#define SeruroStateEventHandler(func) wxEVENT_HANDLER_CAST(SeruroStateEventFunction, func)


#endif 