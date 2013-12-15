#pragma once

#if defined(__WXMSW__)

#ifndef H_AppMSW_Outlook
#define H_AppMSW_Outlook

#include "SeruroApps.h"

#define INITGUID
#define USES_IID_IMessage
#define USES_IID_IMAPITable
//#define USES_IID_IMAPIPropData
//#define USES_IID_IMAPITable

#include <Windows.h>
#include <MAPI.h>

#include <initguid.h> //this is needed,
#include "../include/mapiguid.h" //then this
#include "../include/mapiform.h"
#include <objbase.h>
#include "../include/mapix.h"
#include "../include/mapitags.h"
#include "../include/mapidefs.h"
#include "../include/mapiutil.h"
#include <imessage.h>

/* Not available in the Windows 8 SDK, copied from 7A and linked explicitly. */
//#include <MAPIX.h>
//#include <MAPIUtil.h>

/* Defined in header for testing. */
wxJSONValue GetContactProperties(wxString address);
wxString CreateOutlookContact(wxString address);

class AppMSW_Outlook : public AppHelper
{
public:
    /* Most likely, will do nothing. */
    AppMSW_Outlook() : AppHelper() {
		this->info["accounts"] = wxJSONValue(wxJSONTYPE_OBJECT);

		can_assign = true;
		can_unassign = true;
		needs_restart = false;
		needs_contacts = true;
	}
    
    bool IsInstalled();
    wxString GetVersion();
    
    wxArrayString GetAccountList();
    account_status_t IdentityStatus(wxString address, wxString &server_uuid);

	bool AssignIdentity(wxString server_uuid, wxString address);
	bool UnassignIdentity(wxString address);

	bool AddContact(wxString server_uuid, wxString address);
	//bool RemoveContact(wxString server_uuid, wxString address);
    
private:
    bool GetInfo();

	//void FindAccountProperties(LPSERVICEADMIN &service_admin, 
	//	SPropValue &uid, wxString &account_name);

	/* Get info from an account .oeaccount file. 
	 *  - Set the account info in this->info,
	 *  - return the address of the account.
	 */
	//wxString GetAccountInfo(const wxString &account_folder,
	//	const wxString &account_filename);
	//wxString GetAccountFile(wxString address);

	/* Populate account_file if true. */
	//bool GetAccountFile(const wxString &sub_folder, 
	//	wxString &account_filename);

};

#endif

/* OS detection. */
#endif