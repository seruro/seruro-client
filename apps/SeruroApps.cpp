
#include "SeruroApps.h"
#include "../SeruroClient.h"
#include "../api/SeruroStateEvents.h"

#include "ProcessManager.h"

DECLARE_APP(SeruroClient);

//#define RESTART_FORCE wxID_YES
//#define RESTART_DELAY wxID_CANCEL
//#define RESTART_OK    wxID_NO

wxString UUIDFromFingerprint(const wxString &fingerprint)
{
    wxArrayString server_list;
    wxArrayString account_list;
    wxArrayString identity_list;
    
    /* Find the first server which contains an account that has the given fingerprint. */
    server_list = theSeruroConfig::Get().GetServerList();
    for (size_t i = 0; i < server_list.size(); i++) {
        account_list = theSeruroConfig::Get().GetAddressList(server_list[i]);
        for (size_t j = 0; j < account_list.size(); j++) {
            identity_list = theSeruroConfig::Get().GetIdentity(server_list[i], account_list[j]);
            for (size_t k = 0; k < identity_list.size(); k++) {
                if (identity_list[k] == fingerprint) {
                    return server_list[i];
                }
            }
        }
    }
    return wxEmptyString;
}

#if defined(__WXOSX__) || defined(__WXMAC__)
/* OSX includes. */
#include "AppOSX_Mail.h"
#include <CoreFoundation/CFStream.h>

/* Converts a CF string into a wx string. */
wxString AsString(CFStringRef string)
{
    char *path;
    /* CFString lengths are in UTF 16 pairs. */
    size_t length = CFStringGetLength(string) * 2 * sizeof(char);
    
    /* Allocate and convert. */
    path = (char *) malloc(length);
    CFStringGetCString(string, path, length, kCFStringEncodingASCII);
    wxString path_string = _(path);
    delete path;
    
    return path_string;
}

/* Converts an expected CF string from CF collection into a wx string. */
wxString AsString(const void *value)
{
    if (CFGetTypeID(value) != CFStringGetTypeID()) {
        /* If the actual type is not a string, return an empty representation. */
        return wxEmptyString;
    }
    
    return AsString((CFStringRef) value);
}

void SeruroApps::InitOSX()
{
    AppHelper *helper;
    
    /* Example, create the helper, add the string association. */
    helper = (AppHelper *) new AppOSX_Mail();
    AddAppHelper(_("OSX Mail"), helper);
}

#endif

#if defined(__WXMSW__)
/* MS windows includes. */

#include "AppMSW_LiveMail.h"
#include "AppMSW_Outlook.h"

wxRegKey *GetInstallKey(wxString key_install, wxRegKey::StdKey hive, wxString base)
{
	wxRegKey *check_key = new wxRegKey(hive, base);
    
	/* If this is a 64-bit system then the registry will need to be reopened. */
	wxString installer_sub;
	long key_index = 0;

	if (check_key->Exists() && check_key->GetFirstKey(installer_sub, key_index)) {
		if (installer_sub.compare("ResolveIOD") == 0) {
			delete check_key;
			check_key = new wxRegKey(hive, base, wxRegKey::WOW64ViewMode_64);
		}
	}

	wxRegKey *install_key = new wxRegKey(*check_key, key_install);
	delete check_key;
	return install_key;
}

void SeruroApps::InitMSW()
{
    AppHelper *helper;

	helper = (AppHelper *) new AppMSW_LiveMail();
	AddAppHelper(_("Windows Live Mail"), helper);
	helper = (AppHelper *) new AppMSW_Outlook();
	AddAppHelper(_("Microsoft Outlook"), helper);
}

#endif

#if defined(__WXGTK__)
/* GTK includes. */

void SeruroApps::InitGTK()
{
    
}

#endif

SeruroApps::SeruroApps()
{
    this->app_count = 0;
    
    /* Start each app helper. */
#if defined(__WXMSW__)
    this->InitMSW();
#elif defined(__WXOSX__) || defined(__WXMAC__)
    this->InitOSX();
#elif defined(__WXGTK__)
    this->InitGTK();
#else
    wxLogMessage(_("SeruroApps> (ctor) cannot determine OS."));
    return;
#endif
}

AppHelper* SeruroApps::GetHelper(wxString app_name)
{
    AppHelper *helper = 0;
    
    for (size_t i = 0; i < app_names.size(); i++) {
        if (app_name.compare(app_names[i]) == 0) {
            helper = app_helpers[i];
        }
    }
    return helper;
}

wxArrayString SeruroApps::GetAppList()
{
    wxArrayString empty_whitelist;
    
    return this->GetAppList(empty_whitelist);
}

wxArrayString SeruroApps::GetAppList(wxArrayString whitelist)
{
    wxArrayString filtered_names;
    bool match_whitelist = false;
    
    /* If there is no whitelist, then everything matches. */
    if (whitelist.size() == 0) {
        return this->app_names;
    }
    
    for (size_t i = 0; i < this->app_names.size(); i++) {
        match_whitelist = false;
        /* Check the name against every acceptable name in the whitelist. */
        for (size_t j = 0; j < whitelist.size(); j++) {
            if (this->app_names[i] == whitelist[j]) {
                match_whitelist = true;
                break;
            }
        }
        /* Only add, if the name was found in the whitelist. */
        if (match_whitelist) {
            filtered_names.Add(this->app_names[i]);
        }
    }
    
    return filtered_names;
}

wxArrayString SeruroApps::GetAccountList(wxString app_name)
{
    wxArrayString empty_whitelist;
    
    return this->GetAccountList(app_name, empty_whitelist);
}

wxArrayString SeruroApps::GetAccountList(wxString app_name, wxArrayString whitelist)
{
    wxArrayString filtered_accounts;
    bool match_whitelist = false;
    
    wxArrayString accounts;
    AppHelper *helper;
    
    /* Get helper for the given app_name. */
    helper = this->GetHelper(app_name);
    if (helper == 0) {
        return accounts;
    }
    
    /* If there is no whitelist, then everything matches. */
    accounts = helper->GetAccountList();
    if (whitelist.size() == 0) {
        return accounts;
    }
    
    for (size_t i = 0; i < accounts.size(); i++) {
        match_whitelist = false;
        /* Check the name against every acceptable name in the whitelist. */
        for (size_t j = 0; j < whitelist.size(); j++) {
            if (accounts[i] == whitelist[j]) {
                match_whitelist = true;
                break;
            }
        }
        /* Only add, if the name was found in the whitelist. */
        if (match_whitelist) {
            filtered_accounts.Add(accounts[i]);
        }
    }
    
    /* The whitelist-applied account list. */
    return filtered_accounts;
}

wxJSONValue SeruroApps::GetApp(wxString app_name)
{
    wxJSONValue app_info;
    AppHelper *helper;
    
    /* Search the list of supported apps for the app given by name. */
    helper = this->GetHelper(app_name);
    
    /* Did not find the app, this should not happen. */
    if (helper == 0) return app_info;
    
    app_info["version"] = helper->GetVersion();
    //bool is_installed =
    /* Allow status as a tri-state. */
    if (helper->IsInstalled()) {
        app_info["status"] = _("Installed");
    } else if (! helper->is_detected) {
        app_info["status"] = _("N/A");
    } else {
        app_info["status"] = _("Not Installed");
    }
    
    return app_info;
}

wxString SeruroApps::GetAccountName(wxString app_name, wxString address)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return address;
    
    return helper->GetAccountName(address);
}

account_status_t SeruroApps::IdentityStatus(wxString app_name, wxString account_name, wxString &server_uuid, bool initial)
{
    AppHelper *helper;
    account_status_t status;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return APP_UNASSIGNED;

    /* Get the status, then check if this is an inital check, if so APP_ASSIGNED becomes APP_PENDING_RESTART. */
    status = helper->IdentityStatus(account_name, server_uuid);
    if (initial && status == APP_ASSIGNED) {
        if (helper->IsRunning()) {
            //return APP_PENDING_RESTART;
            status = APP_PENDING_RESTART;
        }
    }
    
    return status;
}

bool SeruroApps::AssignIdentity(wxString app_name, wxString server_uuid, wxString address)
{
    AppHelper *helper;
    bool assign_status;
    
    if (this->assign_pending == true) {
        /* Cannot assign while waiting for another application. */
        DEBUG_LOG(_("SeruroApps> (AssignIdentity) cannot to %s, there is another application pending."), app_name);
        return false;
    }
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;

    wxCriticalSectionLocker enter(wxGetApp().seruro_critsection_assign);
    this->assign_pending = true;
    
    /* Ask the application to assign. */
    assign_status = helper->AssignIdentity(server_uuid, address);
    if (! assign_status) {
        return false;
    }
    
    /* Create an identity update event. */
    SeruroStateEvent identity_event(STATE_TYPE_IDENTITY, STATE_ACTION_UPDATE);
    identity_event.SetServerUUID(server_uuid); /* This is normal discarded */
    identity_event.SetValue("app", app_name);
    identity_event.SetAccount(address);
    
    /* Check if the application needs a restart. */
    if (helper->needs_restart && helper->IsRunning()) {
        assign_status = this->RequireRestart(helper, app_name);
    }
    
    /* No longer require application or user interaction. */
    this->assign_pending = false;
    if (! assign_status) {
        /* Append to debug log. */
        identity_event.SetValue("status", "Restart Pending");
    }
    
    /* Process the event. */
    wxGetApp().AddEvent(identity_event);
    
    return true;
}

bool SeruroApps::RequireRestart(AppHelper *app, wxString app_name)
{
    int restart_state; 
    
    /* Create and show a RestartApp dialog. */
    this->restart_dialog = new RestartAppDialog(wxGetApp().GetFrame(), app_name);
    
    /* A monitor thread should check for this, and return a state event if the restart occurs. */
    /* Note: this application could already be pending a restart, we'll pop open the dialog anyway. */
    app->restart_pending = true;
    
    restart_state = restart_dialog->ShowModal();
    delete restart_dialog;
        
    if (restart_state == wxID_OK) {
        /* Try to restart the application. */
        this->RestartApp(app_name);
        return true;
    }
    
    if (restart_state == wxID_NO) {
        /* The application was stopped by the user, and the modal was closed by the monitor thread. */
        return true;
    }
    
    /* The user canceled the dialog, the application is still running. */
    return false;
}

bool SeruroApps::IsAppRunning(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->IsRunning();
}

bool SeruroApps::StopApp(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->StopApp();
}

bool SeruroApps::StartApp(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->StartApp();
}

bool SeruroApps::RestartApp(wxString app_name)
{
    AppHelper *helper;
    size_t delay_count;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    /* Take control from the monitor thread. */
    helper->restart_pending = false;
    
    /* Stop the application, then wait for it to end (this will block the main thread?). */
    /* Todo: show modal for restarting? */
    helper->StopApp();
    
    /* Timeout = APPS_RESTART_DELAY * APPS_RESTART_MAX_COUNT */
    delay_count = 0;
    while (helper->IsRunning() && delay_count < APPS_RESTART_MAX_COUNT) {
        wxMilliSleep(APPS_RESTART_DELAY);
        delay_count += 1;
    }
    
    if (helper->IsRunning()) {
        /* Could not stop application. Allow the the monitor thread to take over. */
        helper->restart_pending = true;

        return false;
    }
    
    /* Don't worry about waiting for the application to start. */
    /* Note: could check to make sure the identity is installed. */
    helper->StartApp();
    return true;
}

bool SeruroApps::CanAssign(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->can_assign;
}

bool SeruroApps::CanUnassign(wxString app_name)
{
    AppHelper *helper;
    
    helper = this->GetHelper(app_name);
    if (helper == 0) return false;
    
    return helper->can_unassign;
}

void SeruroApps::AddAppHelper(wxString app_name, AppHelper *app_helper)
{
    this->app_names.Add(app_name);
    this->app_helpers[app_count++] = app_helper;
}
