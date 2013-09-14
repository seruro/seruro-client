
#include "AppMonitor.h"
#include "SeruroApps.h"
#include "../api/SeruroStateEvents.h"
//#include "SeruroServerAPI.h"
//#include "../crypto/SeruroCrypto.h"

#include "../SeruroClient.h"
#include "../SeruroConfig.h"
#include "../logging/SeruroLogger.h"

/* Note: SeruroClient monitors for STATE_TYPE_APPLICATION/STATE_ACTION_CLOSE.
 *   If this occurs it assumes the application was running while pending a restart.
 *   The callback should close any restart dialogs, assuming there is only one 
 *   restart dialog which can be opened at any given time.
 */

DECLARE_APP(SeruroClient);

AppMonitor::AppMonitor()
{
    this->pending_closed = wxJSONValue(wxJSONTYPE_OBJECT);
}

bool AppMonitor::FastMonitor()
{
    wxArrayString app_list;
    
    /* Iterate through apps, check for "restart_pending". */
    app_list = theSeruroApps::Get().GetAppList();
    for (size_t i = 0; i < app_list.size(); i++) {
        if (theSeruroApps::Get().IsRestartPending(app_list[i])) {
            if (! theSeruroApps::Get().IsAppRunning(app_list[i])) {
                /* Do not send multiple events. */
                if (pending_closed.HasMember(app_list[i])) continue;
            
                /* Pending closed will never be removed, when the closed event bubbles the previous condition will fall out. */
                pending_closed[app_list[i]] = true;
                this->CreateCloseEvent(app_list[i]);
            }
        } else if (pending_closed.HasMember(app_list[i])) {
            if (theSeruroApps::Get().IsAppRunning(app_list[i])) {
                /* The aforementioned condition will fall in if the app is opened and a restart becomes pending again. */
                pending_closed.Remove(app_list[i]);
            }
        }
    }
    
    return true;
}

void AppMonitor::CreateCloseEvent(wxString app_name)
{
    /* If "restart_pending" and ! app->IsRunning(), create event. */
    SeruroStateEvent event(STATE_TYPE_APPLICATION, STATE_ACTION_CLOSE);
    event.SetValue("app_name", app_name);
    
    /* It might be beneficial to assign an ID to the close event. */
    wxGetApp().AddEvent(event);
}
