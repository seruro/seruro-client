
#ifndef H_SeruroPanelHome
#define H_SeruroPanelHome

#include "SeruroFrame.h"
#include "UIDefs.h"

#include "../wxJSON/wx/jsonval.h"
#include "../api/SeruroStateEvents.h"

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
    
    /* This is a general welcome message. */
    Text *text_welcome;
    
    /* This is a comment on the state of servers. */
    Text *servers_welcome;
    /* This is a comment on the state of identities. */
    Text *accounts_welcome;
    /* This is a comment on how to add/remove contacts. */
    Text *contacts_welcome;
    /* This is a comment on the state of applications. */
    Text *applications_welcome;
};

#endif