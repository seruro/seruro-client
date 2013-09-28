
#ifndef H_SeruroPanelHome
#define H_SeruroPanelHome

#include "SeruroFrame.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../api/SeruroStateEvents.h"

#include <wx/button.h>

/* The type of required action. */
enum home_actions_t
{
    HOME_ACTION_CONTACTS,
    HOME_ACTION_SERVER,
    HOME_ACTION_ACCOUNT,
    HOME_ACTION_IDENTITY
};

class SeruroPanelHome : public SeruroPanel
{
public:
	SeruroPanelHome(wxBookCtrlBase *book);
    
    /* I want it all! */
    void OnAccountStateChange(SeruroStateEvent &event);
    void OnServerStateChange(SeruroStateEvent &event);
    void OnContactStateChange(SeruroStateEvent &event);
    void OnApplicationStateChange(SeruroStateEvent &event);
    void OnIdentityStateChange(SeruroStateEvent &event);
    void OnOptionStateChange(SeruroStateEvent &event);
    
    /* Determine if the application is 'set up'. */
    bool IsReady();
    
private:
    void GenerateServerBox();
    void GenerateApplicationBox();
    void GenerateWelcomeBox();
    
    /* If there is an action to be taken. */
    void OnAction(wxCommandEvent &event);
    
    /* This is a general welcome message. */
    Text *text_welcome;
    
    /* This is a comment on the state of servers. */
    Text *servers_welcome;
    /* This is a comment on the state of identities. */
    Text *accounts_welcome;
    /* This is a comment on how to add/remove contacts. */
    Text *contacts_welcome;
    /* This is a comment on the state of applications. */
    Text *applications_list;
    Text *applications_welcome;
    
    /* Action button */
     wxButton *action_button;
    home_actions_t action_type;
    /* Show the button, set label, and store action. */
    void SetAction(home_actions_t action);
    
    DECLARE_EVENT_TABLE()
};

#endif